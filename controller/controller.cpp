/*
 * Copyright (C) 2019 Medusalix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "controller.h"
#include "../utils/log.h"
#include "bytes.h"

#include <cstdlib>
#include <cmath>
#include <linux/input.h>
#include <string.h>

Controller::Controller(SendPacket sendPacket) : GipDevice(sendPacket) {
    initInput(nullptr);
}

Controller::~Controller() {
    if (!setPowerMode(DEVICE_ID_CONTROLLER, POWER_OFF)) {
        Log::error("Failed to turn off controller");
    }
}

void Controller::deviceAnnounced(uint8_t id, const AnnounceData *announce) {
    Log::info("Device announced, product id: %04x", announce->productId);
    Log::debug("Firmware version: %d.%d.%d.%d", announce->firmwareVersion.major,
               announce->firmwareVersion.minor, announce->firmwareVersion.build,
               announce->firmwareVersion.revision);
    Log::debug("Hardware version: %d.%d.%d.%d", announce->hardwareVersion.major,
               announce->hardwareVersion.minor, announce->hardwareVersion.build,
               announce->hardwareVersion.revision);
}

void Controller::statusReceived(uint8_t id, const StatusData *status) {
    const std::string levels[] = {"empty", "low", "medium", "full"};

    uint8_t type  = status->batteryType;
    uint8_t level = status->batteryLevel;

    // Controller is charging or level hasn't changed
    if (type == BATT_TYPE_CHARGING || level == batteryLevel) {
        return;
    }

    Log::info("Battery level: %s", levels[level].c_str());

    batteryLevel = level;
}

void Controller::serialNumberReceived(const SerialData *serial) {
    const std::string number(serial->serialNumber, sizeof(serial->serialNumber));
    m_SerialData = *serial;
    Log::info("Serial number: %s", number.c_str());
}

void Controller::inputReceived(const InputData *input) {
    if (memcmp(input, &m_InputData, sizeof(InputData)) == 0) {
        return;
    }
    Log::info("Input Recieved");
    m_InputData = *input;
    Log::info("a: %d", m_InputData.buttons.a);
}

void Controller::initInput(const AnnounceData *announce) {
    LedModeData ledMode = {};
    // Dim the LED a little bit, like the original driver
    // Brightness ranges from 0x00 to 0x20
    ledMode.mode       = LED_ON;
    ledMode.brightness = 0x14;

    if (!xboxOneSReset(DEVICE_ID_CONTROLLER)) {
        Log::error("Failed to reset device");
        return;
    }

    if (!setPowerMode(DEVICE_ID_CONTROLLER, POWER_ON)) {
        Log::error("Failed to set initial power mode");

        return;
    }

    if (!setLedMode(ledMode)) {
        Log::error("Failed to set initial LED mode");

        return;
    }

    if (!requestSerialNumber()) {
        Log::error("Failed to request serial number");

        return;
    }
}