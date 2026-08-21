/*
    Copyright (C) 2018 Sebastian J. Wolf

    This file is part of Funzel.

    Funzel is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Funzel is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Funzel. If not, see <http://www.gnu.org/licenses/>.
*/

#include "funzel.h"
#include <QFile>
#include <QStringListIterator>
#include <QListIterator>
#include <QMutableListIterator>
#include <QMapIterator>
#include <QSqlError>
#include <QStandardPaths>

const char SETTINGS_USE_ANIMATION[] = "settings/useAnimation";
const char SETTINGS_ANIMATION_COLOR[] = "settings/animationColor";
const char SETTINGS_COLOR_ASSIGNMENT_PREFIX[] = "settings/colorAssignment_";

const char POGO_PIN_5V[]  = "/sys/class/yft_pogo_pin/yft_pogo_pin_5v_out_state";
const char POGO_PIN_ADC[] = "/sys/class/yft_pogo_pin/yft_pogo_pin_adc_value";
const char POGO_PIN_INT[] = "/sys/class/yft_pogo_pin/yft_pogo_pin_int_state";

const char DBUS_SD_MANAGER_SERVICE[]  = "org.freedesktop.systemd1";
const char DBUS_SD_MANAGER_PATH[]     = "/org/freedesktop/systemd1";
const char DBUS_SD_MANAGER_IFACE[]    = "org.freedesktop.systemd1.Manager";
const char DBUS_SD_TOH_SERVICE_NAME[]  = "jolla-csd-tohd.service";

const int JP2601_LED_COUNT = 1;

Funzel::Funzel(QObject *parent) : QObject(parent), settings("harbour-funzel", "settings")
{
    this->networkAccessManager = new QNetworkAccessManager(this);

    QDBusConnection::sessionBus().connect("", "/calls/active", "org.nemomobile.voicecall.VoiceCall", "lineIdChanged",
                                          this, SLOT(onIncomingCall(const QDBusMessage&)));
    QDBusConnection::sessionBus().connect("", "/calls/active", "org.nemomobile.voicecall.VoiceCall", "statusChanged",
                                          this, SLOT(onCallStatusChanged(const QDBusMessage&)));
    QDBusConnection::sessionBus().connect("org.nemomobile.voicecall", "/", "org.nemomobile.voicecall.VoiceCallManager", "voiceCallsChanged",
                                          this, SLOT(onVoiceCallsChanged(const QDBusMessage&)));

    if (QFile::exists("/proc/aw9120_operation")) {
        this->geminiFound = true;
    } else {
        this->geminiFound = false;
    }
    this->jp2601Found = QFile::exists(POGO_PIN_INT);

    initializeDatabase();
    initializeContactAssignments();
    if (this->geminiFound)
        qInfo() << "Identified device: Gemini PDA.";
    if (this->jp2601Found)
        qInfo() << "Identified device: Jolla Phone 2026.";
    leds();
}

Funzel::~Funzel()
{
    powerLed(1, 0, 0, 0);
    powerLed(2, 0, 0, 0);
    powerLed(3, 0, 0, 0);
    powerLed(4, 0, 0, 0);
    powerLed(5, 0, 0, 0);
}

void Funzel::powerLed(const int &ledNumber, const int &intensityRed, const int &intensityGreen, const int &intensityBlue)
{
    // qDebug() << "Funzel::powerLed" << ledNumber << intensityRed << intensityGreen << intensityBlue;
    if (geminiFound) {
        QFile ledFile("/proc/aw9120_operation");
        if (ledNumber < 1 || ledNumber > 5) {
            qDebug() << "Invalid LED number" << ledNumber;
            return;
        }
        if (intensityRed < 0 || intensityRed > 3) {
            qDebug() << "Invalid red LED intensity" << intensityRed;
            return;
        }
        if (intensityGreen < 0 || intensityGreen > 3) {
            qDebug() << "Invalid green LED intensity" << intensityGreen;
            return;
        }
        if (intensityBlue < 0 || intensityBlue > 3) {
            qDebug() << "Invalid blue LED intensity" << intensityBlue;
            return;
        }
        if (ledFile.open(QIODevice::WriteOnly)) {
            QString ledString;
            ledString.append(QString::number(ledNumber));
            ledString.append(" ");
            ledString.append(QString::number(intensityRed));
            ledString.append(" ");
            ledString.append(QString::number(intensityGreen));
            ledString.append(" ");
            ledString.append(QString::number(intensityBlue));
            ledFile.write(ledString.toUtf8());
            ledFile.flush();
            ledFile.close();
        } else {
            qDebug() << "[Funzel] Unable to acquire write access to LED";
        }
    }
    if (jp2601Found) {
        QFile rLedFile("/sys/class/leds/red/brightness");
        QFile gLedFile("/sys/class/leds/green/brightness");
        QFile bLedFile("/sys/class/leds/blue/brightness");
        QFile rMaxLedFile("/sys/class/leds/red/max_brightness");
        QFile gMaxLedFile("/sys/class/leds/green/max_brightness");
        QFile bMaxLedFile("/sys/class/leds/blue/max_brightness");
        if (ledNumber > JP2601_LED_COUNT) {
            qDebug() << "Invalid LED number" << ledNumber;
            return;
        }
        if (rLedFile.open(QIODevice::WriteOnly)) {
            if (rMaxLedFile.open(QIODevice::ReadOnly)) {
                int max = rMaxLedFile.readAll()[0];
                if (intensityRed < max) {
                    rLedFile.write(QByteArray::number(intensityRed));
                }
            } else {
                qDebug() << "[Funzel] Unable to read max brightness file";
                return;
            }
        } else {
            qDebug() << "[Funzel] Unable to acquire write access to LED";
            return;
        }
        if (gLedFile.open(QIODevice::WriteOnly)) {
            if (gMaxLedFile.open(QIODevice::ReadOnly)) {
                int max = gMaxLedFile.readAll()[0];
                if (intensityGreen < max) {
                    gLedFile.write(QByteArray::number(intensityGreen));
                }
            } else {
                qDebug() << "[Funzel] Unable to read max brightness file";
                return;
            }
        } else {
            qDebug() << "[Funzel] Unable to acquire write access to LED";
            return;
        }
        if (bLedFile.open(QIODevice::WriteOnly)) {
            if (bMaxLedFile.open(QIODevice::ReadOnly)) {
                int max = bMaxLedFile.readAll()[0];
                if (intensityBlue < max) {
                    gLedFile.write(QByteArray::number(intensityBlue));
                }
            } else {
                qDebug() << "[Funzel] Unable to read max brightness file";
                return;
            }
        } else {
            qDebug() << "[Funzel] Unable to acquire write access to LED";
            return;
        }
    }
}

void Funzel::setUseAnimation(const bool &useAnimation)
{
    settings.setValue(SETTINGS_USE_ANIMATION, useAnimation);
    emit useAnimationChanged();
}

bool Funzel::getUseAnimation()
{
    return settings.value(SETTINGS_USE_ANIMATION, false).toBool();
}

void Funzel::setAnimationColor(const int &animationColor)
{
    settings.setValue(SETTINGS_ANIMATION_COLOR, animationColor);
    emit animationColorChanged();
}

int Funzel::getAnimationColor()
{
    return settings.value(SETTINGS_ANIMATION_COLOR, 0).toInt();
}

bool Funzel::isGeminiFound()
{
    return this->geminiFound;
}

bool Funzel::isJP2601Found()
{
    return this->jp2601Found;
}

bool Funzel::isToHFound()
{
    return this->tohFound;
}

bool Funzel::isContactsDbAvailable()
{
    return this->canUseContactsDb;
}

void Funzel::loadContacts()
{
    qDebug() << "Funzel::loadContacts";
    QVariantList listContacts;
    contacts.clear();
    QSqlQuery databaseQuery(database);
    databaseQuery.prepare("select distinct contactId, displayLabel from Contacts where displayLabel is not null order by displayLabel collate nocase asc;");
    if (databaseQuery.exec()) {
        qDebug() << "Contacts successfully selected from database!";
        while (databaseQuery.next()) {
            QVariantMap singleContact;
            singleContact.insert("contactId", databaseQuery.value(0).toString());
            singleContact.insert("displayName", databaseQuery.value(1).toString());
            listContacts.append(singleContact);
            qDebug() << singleContact.value("contactId").toString() << singleContact.value("displayName");
            contacts.insert(singleContact.value("contactId").toString(), singleContact.value("displayName"));
        }
        qDebug() << "We probably found some contacts: " << listContacts.size();
        emit contactsLoaded(listContacts);
    } else {
        qDebug() << "Error selecting followers from database!";
        emit errorLoadingContacts();
    }
}

void Funzel::assignAnimationColor(const QString &animationColor, const QString &contactId)
{
    qDebug() << "Funzel::assignAnimationColor" << animationColor << contactId;
    contactAssignments.insert(contactId, animationColor);
    synchronizeData();
}

void Funzel::deleteContactAssignment(const QString &contactId)
{
    qDebug() << "Funzel::deleteContactAssignment" << contactId;
    contactAssignments.remove(contactId);
    synchronizeData();
    emit contactAssignmentsInvalidated();
}

QVariantMap Funzel::getColorAssignments()
{
    qDebug() << "Funzel::getColorAssignments";
    return this->colorAssignments;
}

QString Funzel::getContactDisplayName(const QString &contactId)
{
    qDebug() << "Funzel::getContactDisplayName" << contactId;
    qDebug() << contacts.value(contactId).toString();
    return this->contacts.value(contactId).toString();
}

QString Funzel::getColorId(const int &colorIndex)
{
    switch (colorIndex) {
    case 0:
        return "red";
        break;
    case 1:
        return "green";
        break;
    case 2:
        return "blue";
        break;
    case 3:
        return "yellow";
        break;
    case 4:
        return "lightBlue";
        break;
    case 5:
        return "purple";
        break;
    case 6:
        return "white";
        break;
    case 7:
        return "rainbow";
        break;
    default:
        return "red";
        break;
    }
}

int Funzel::getColorIndex(const QString &colorId)
{
    if (colorId == "red") {
        return 0;
    }
    if (colorId == "green") {
        return 1;
    }
    if (colorId == "blue") {
        return 2;
    }
    if (colorId == "yellow") {
        return 3;
    }
    if (colorId == "lightBlue") {
        return 4;
    }
    if (colorId == "purple") {
        return 5;
    }
    if (colorId == "white") {
        return 6;
    }
    if (colorId == "rainbow") {
        return 7;
    }
    return 0;
}

void Funzel::onIncomingCall(const QDBusMessage &dBusMessage)
{
    qDebug() << "Funzel::onIncomingCall" << dBusMessage;

    QString callingNumber = dBusMessage.arguments().at(0).toString();
    QDBusInterface voiceCallInterface("org.nemomobile.voicecall",
                                      "/calls/active",
                                      "org.nemomobile.voicecall.VoiceCall",
                                      QDBusConnection::sessionBus() );

    if (voiceCallInterface.property("isIncoming").toBool()) {
       int colorIndex = -1;
       qDebug() << "[Funzel] Incoming call..." << callingNumber;
       QSqlQuery databaseQuery(database);
       databaseQuery.prepare("select distinct contactId from PhoneNumbers where phoneNumber=(:callingNumber);");
       databaseQuery.bindValue(":callingNumber", callingNumber);
       if (databaseQuery.exec()) {
           while (databaseQuery.next()) {
               QString contactId = databaseQuery.value(0).toString();
               if (contactAssignments.contains(contactId)) {
                   QString colorId = contactAssignments.value(contactId).toString();
                   qDebug() << "Contact found!" << colorId;
                   colorIndex = getColorIndex(colorId);
               }
           }
       }
       qDebug() << "[Funzel] Power ON!";
       if (colorIndex >= 0) {
           emit powerColor(colorIndex);
       } else {
           emit powerOn();
       }
    } else {
       qDebug() << "[Funzel] Other call..." << callingNumber;
    }
}

void Funzel::onCallStatusChanged(const QDBusMessage &dBusMessage)
{
    qDebug() << "Funzel::onCallStatusChanged" << dBusMessage;
    int callStatusCode = dBusMessage.arguments().at(0).toInt();
    if (callStatusCode != 5) {
        qDebug() << "[Funzel] Power OFF!";
        emit powerOff();
    }
}

void Funzel::onVoiceCallsChanged(const QDBusMessage &dBusMessage)
{
    qDebug() << "Funzel::onVoiceCallsChanged" << dBusMessage;
    QDBusInterface voiceCallInterface("org.nemomobile.voicecall",
                                      "/",
                                      "org.nemomobile.voicecall.VoiceCallManager",
                                      QDBusConnection::sessionBus() );
    QStringList voiceCalls = voiceCallInterface.property("voiceCalls").toStringList();
    if (voiceCalls.size() == 0) {
        qDebug() << "[Funzel] Power OFF!";
        powerOff();
    }
}

void Funzel::initializeDatabase()
{
    qDebug() << "Funzel::initializeDatabase";
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/system/Contacts/qtcontacts-sqlite/contacts.db");
    if (database.open()) {
        qDebug() << "Contacts database successfully opened :)";
        canUseContactsDb = true;
        loadContacts();
    } else {
        qDebug() << "Error opening Contacts database :(";
        canUseContactsDb = false;
    }
}

void Funzel::initializeContactAssignments()
{
    qDebug() << "Funzel::initializeContactAssignments";
    colorAssignments.clear();
    contactAssignments.clear();
    for (int i = 0; i <= 6; i++) {
        QString colorId = getColorId(i);
        QVariantList assignedContacts = settings.value(SETTINGS_COLOR_ASSIGNMENT_PREFIX + QString::number(i)).toList();
        QMutableListIterator<QVariant> contactIterator(assignedContacts);
        while (contactIterator.hasNext()) {
            QString contactId = contactIterator.next().toString();
            if (contacts.contains(contactId)) {
                contactAssignments.insert(contactId, colorId);
            } else {
                contactIterator.remove();
            }
        }
        colorAssignments.insert(colorId, assignedContacts);
    }
}

void Funzel::synchronizeData()
{
    qDebug() << "Funzel::synchronizeData";
    colorAssignments.clear();
    QMapIterator<QString, QVariant> contactAssignmentIterator(contactAssignments);
    while (contactAssignmentIterator.hasNext()) {
        contactAssignmentIterator.next();
        QString currentContactId = contactAssignmentIterator.key();
        QString currentAnimationColor = contactAssignmentIterator.value().toString();

        QVariantList assignedContacts;
        if (colorAssignments.contains(currentAnimationColor)) {
            assignedContacts = colorAssignments.value(currentAnimationColor).toList();
        }
        assignedContacts.append(currentContactId);
        colorAssignments.insert(currentAnimationColor, assignedContacts);
    }

    for (int i = 0; i <= 6; i++) {
        QString colorId = getColorId(i);
        if (colorAssignments.contains(colorId)) {
            QVariantList currentColorAssignments = colorAssignments.value(colorId).toList();
            settings.setValue(SETTINGS_COLOR_ASSIGNMENT_PREFIX + QString::number(getColorIndex(colorId)), currentColorAssignments);
        } else {
            settings.remove(SETTINGS_COLOR_ASSIGNMENT_PREFIX + QString::number(getColorIndex(colorId)));
        }
    }
}

QVariantList Funzel::leds() {
    QVariantList leds;
    if (jp2601Found) {
        QVariantMap map({
                        { "name", "JP2601 Red LED" },
                        { "path", "/sys/class/leds/red/"}
        });
        leds.append(map);
        map = {
                        { "name", "JP2601 Green LED" },
                        { "path", "/sys/class/leds/green/"}
        };
        leds.append(map);
        map = {
                        { "name", "JP2601 Blue LED" },
                        { "path", "/sys/class/leds/blue/"}
        };
        leds.append(map);
    }
    return leds;
}

void Funzel::tohLed(bool on, const int &deviceId = 0) {
    if (deviceId == 0 ) {
        ToHDeviceState want = on ? ToHDeviceState::LedOn : ToHDeviceState::LedOff;
        toggleToHDevice(deviceId, want);
        return;
    }
    qWarning() << "Inknown device with ID" << deviceId;
}

Funzel::ToHDeviceState Funzel::tohState(int deviceId = 0) const
{
    if (deviceId == 0 ) {
        return tohLedState;
    }
   return ToHDeviceState::UnknownState;
}

void Funzel::toggleToHDevice(const int &deviceId, ToHDeviceState newState)
{
    if (deviceId == 0 ) {
        QDBusInterface tohService( DBUS_SD_MANAGER_SERVICE, DBUS_SD_MANAGER_PATH, DBUS_SD_MANAGER_IFACE, QDBusConnection::sessionBus() );
        tohService.callWithArgumentList(QDBus::NoBlock, QStringLiteral("KillUnit"),
                        QVariantList {
                            QVariant(DBUS_SD_TOH_SERVICE_NAME),
                            QVariant(QStringLiteral("main")),
                            QVariant((int) newState) // NB: we set the signal number as the enum value!
                        }
                        );

    }
}

