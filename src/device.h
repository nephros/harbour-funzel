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

#ifndef FUNZELDEVICE_H
#define FUNZELDEVICE_H

#include <QObject>
#include <QString>

enum Device {
        Invalid = -1,
        Unknown,
        GeminiPDA,
        JollaPhone2026,
};

enum ToHDeviceType {
    UnknownDevice,
    LedDevice
};

struct FunzelDeviceInfo {
    QString id     ;
    Device  device ;
    QString name   ;
    QVariantList* leds;
    void*   deviceData   ;
    //Q_DISABLE_COPY(FunzelDeviceInfo)
    FunzelDeviceInfo () {
        this->id     = QStringLiteral("");
        this->name   = QStringLiteral("");
        this->device = Device::Invalid;
        this->leds   = new QVariantList();
        this->deviceData   = nullptr;
    }
    FunzelDeviceInfo (const FunzelDeviceInfo &other) {
        this->id = other.id;
        this->name = other.name;
        this->device = other.device;
        this->leds = other.leds;
        this->deviceData = other.deviceData;
    }
    FunzelDeviceInfo ( const QString& id, const QString& name, const Device& device = Device::Invalid, void* deviceData = nullptr) {
        this->id = id;
        this->name = name;
        this->device = device;
        this->leds = leds;
        this->deviceData = deviceData;
    }
    bool isValid() {
        return !id.isNull() && !name.isNull() && device != Device::Invalid;
    }
};

// TODO: verify existence:
//    QFile::exists("/proc/aw9120_operation"))
//    QFile::exists(POGO_PIN_INT);

Q_DECLARE_METATYPE(FunzelDeviceInfo);
Q_DECLARE_METATYPE(FunzelDeviceInfo*);


const FunzelDeviceInfo unknownDevice = FunzelDeviceInfo("unknown", "Unknown Device", Device::Unknown );
const QMap<QString, FunzelDeviceInfo> supportedDevices = {
            { "jp2601",    FunzelDeviceInfo("jp2601",    "Jolla Phone 2026", Device::JollaPhone2026 ) },
            { "geminipda", FunzelDeviceInfo("geminipda", "Gemini PDA",       Device::GeminiPDA ) }
            };


#endif // FUNZEL_H
