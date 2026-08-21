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

#ifndef FUNZEL_H
#define FUNZEL_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QSettings>
#include <QVariantList>
#include <QVariantMap>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "device.h"
#include "tohcapability.h"

#include <signal.h>

struct LedPattern
{
    int pause = -1;       // ms to pause at the end of the pattern, -1 for ininifty (one-off pattern)
    QVector<int> pattern; // sequence of ints, each specifying ms on, ms off, -1 for infinity

};

Q_DECLARE_METATYPE(LedPattern);

class Funzel : public QObject
{
    Q_OBJECT
public:
    explicit Funzel(QObject *parent = 0);
    ~Funzel();

    enum ToHDeviceState {
            UnknownState,
            LedUnknown,
            LedOn = SIGUSR1,   // 10, SIGUSR1 turns LED on  in Inari Blue
            LedOff = SIGUSR2,  // 12, SIGUSR2 turns LED off in Inari Blue
    };
    Q_ENUM(ToHDeviceState);
    Q_INVOKABLE void powerLed(const int &ledNumber, const int &intensityRed, const int &intensityGreen, const int &intensityBlue);
    Q_INVOKABLE void tohLed(bool on, const int &id);
    Q_INVOKABLE void setUseAnimation(const bool &useAnimation);
    Q_INVOKABLE bool getUseAnimation();
    Q_INVOKABLE void setAnimationColor(const int &animationColor);
    Q_INVOKABLE int getAnimationColor();
    Q_INVOKABLE bool isToHFound();
    Q_INVOKABLE bool isContactsDbAvailable();
    Q_INVOKABLE void loadContacts();
    Q_INVOKABLE void assignAnimationColor(const QString &animationColor, const QString &contactId);
    Q_INVOKABLE void deleteContactAssignment(const QString &contactId);
    Q_INVOKABLE QVariantMap getColorAssignments();
    Q_INVOKABLE QString getContactDisplayName(const QString &contactId);
    Q_INVOKABLE QString getColorId(const int &colorIndex);
    Q_INVOKABLE int getColorIndex(const QString &colorId);
    Q_INVOKABLE void addLedPattern(const QString &name, const QVector<int>& pattern, const int &pause);

    Q_PROPERTY(bool supportedDeviceFound READ supportedDeviceFound NOTIFY deviceChanged);
    Q_PROPERTY(QVariant deviceInfo READ getDeviceInfo NOTIFY deviceChanged);
    Q_PROPERTY(QStringList supportedDevices READ listSupportedDevices CONSTANT);

    ToHDeviceState tohState(int deviceId) const ;

    QVariant getDeviceInfo() {
            if (foundDevice->isValid())
                return QVariant::fromValue(foundDevice);
            else
                return QVariant::fromValue(FunzelDeviceInfo());
    };
//    FunzelDeviceInfo* deviceInfo() { return foundDevice; };
    bool supportedDeviceFound() { return foundDevice->device != Device::Unknown; };
    QStringList listSupportedDevices() const;

signals:
    void powerOn();
    void powerColor(const int &colorIndex);
    void powerOff();
    void useAnimationChanged();
    void animationColorChanged();
    void contactAssignmentsInvalidated();
    void contactsLoaded(const QVariantList &contacts);
    void errorLoadingContacts();
    void deviceChanged();
    void tohConnected(const QVariant device);

public slots:
    void onIncomingCall(const QDBusMessage &dBusMessage);
    void onCallStatusChanged(const QDBusMessage &dBusMessage);
    void onVoiceCallsChanged(const QDBusMessage &dBusMessage);
    void onPrivacySwitchChanged(const QDBusMessage &dBusMessage);

private:
    QNetworkAccessManager *networkAccessManager;
    QSettings settings;
    FunzelDeviceInfo* foundDevice;
    bool tohFound;
    bool canUseContactsDb;
    QSqlDatabase database;
    QVariantMap contacts;
    QVariantMap colorAssignments;
    QVariantMap contactAssignments;
    int currentColorIndex = -1;
    QList<ToHCapability> tohCapabilities();
    ToHDeviceState tohLedState = ToHDeviceState::LedUnknown;
    QMap<QString, LedPattern> ledPatterns;
    void toggleToHDevice(const int &id, ToHDeviceState newState);

    bool identifyDevice();
    void analyzeDevices();
    void initializeDatabase();
    void initializeContactAssignments();
    void synchronizeData();

};

#endif // FUNZEL_H
