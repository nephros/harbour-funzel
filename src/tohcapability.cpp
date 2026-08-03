/*
    Copyright (C) 2026 Peter G. <sailfish@nephros.org>

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

#include "tohcapability.h"
#include <QFile>
//#include <QStringListIterator>
//#include <QListIterator>
//#include <QMutableListIterator>
//#include <QMapIterator>
//#include <QSqlError>
//#include <QStandardPaths>

const char POGO_PIN_5V[]  = "/sys/class/yft_pogo_pin/yft_pogo_pin_5v_out_state";
const char POGO_PIN_ADC[] = "/sys/class/yft_pogo_pin/yft_pogo_pin_adc_value";
const char POGO_PIN_INT[] = "/sys/class/yft_pogo_pin/yft_pogo_pin_int_state";

ToHCapability::ToHCapability(QObject *parent) : QObject(parent)
{
    detect();
}

/*
ToHCapability::~ToHCapability()
{
}
*/


bool ToHCapability::detect()
{
    QFile pin(POGO_PIN_INT);
    QByteArray value;

    if (!pin.open(QIODevice::ReadOnly | QIODevice::NotOpen))
        return false;
    value = pin.readAll();
    pin.close();

    bool found = ((value.size() == 1) && (value.data()[0] != 0));
    if (found != m_detected) {
            m_detected = found;
            emit changed();
    }
    return m_detected;
}

QVariantList ToHCapability::capability() const
{
    return QVariantList();
}

