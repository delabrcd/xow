/*
 * Copyright (C) 2020 Medusalix
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

#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
struct Frame;
class Bytes;

/*
 * Base class for GIP (Game Input Protocol) devices
 * Performs basic handshake process:
 *   <- Announce            (from controller)
 *   -> Identify            (from dongle, unused)
 *   <- Identify            (from controller, unused)
 *   -> Power mode: on      (from dongle)
 *   -> LED mode: dim       (from dongle)
 *   -> Authenticate        (from dongle, unused)
 *   <- Authenticate        (from controller, unused)
 *   -> Serial number: 0x00 (from dongle, unused)
 *   <- Serial number       (from controller, unused)
 *   -> Serial number: 0x04 (from dongle)
 *   <- Serial number       (from controller)
 */
struct Header {
    uint8_t type : 8;
    uint8_t length : 8;
} __attribute__((packed));

struct MidiProDrumInputData : public Header {
    uint8_t dpad_up_cym_yellow : 1;
    uint8_t dpad_down_cym_blue : 1;
    uint8_t dpad_left : 1;
    uint8_t dpad_right : 1;

    uint8_t start : 1;
    uint8_t select : 1;
    uint8_t hihat : 1;
    uint8_t pad_select : 1;

    uint8_t lb_kick : 1;
    uint8_t cym_select : 1;
    uint8_t guide : 1;
    uint8_t : 1;

    uint8_t a_green : 1;
    uint8_t b_red : 1;
    uint8_t x_blue : 1;
    uint8_t y_yellow : 1;

    uint8_t unknown[16];

    inline void print() const {
        std::cout << "I THINK THIS PACKET SAYS:" << std::endl;
        if (cym_select) {
            if (y_yellow && dpad_up_cym_yellow)
                std::cout << "\t YELLOW CYMBAL" << std::endl;
            if (x_blue && dpad_down_cym_blue)
                std::cout << "\t BLUE CYMBAL" << std::endl;
            if (a_green)
                std::cout << "\t GREEN CYMBAL" << std::endl;
        }
        if (pad_select) {
            if (a_green)
                std::cout << "\t GREEN PAD" << std::endl;
            if (b_red)
                std::cout << "\t RED PAD" << std::endl;
            if (x_blue)
                std::cout << "\t BLUE PAD" << std::endl;
            if (y_yellow)
                std::cout << "\t YELLOW PAD" << std::endl;
        }
        if (!cym_select && !pad_select) {
            if (dpad_down_cym_blue)
                std::cout << "\t DPAD DOWN" << std::endl;
            if (dpad_left)
                std::cout << "\t DPAD LEFT" << std::endl;
            if (dpad_up_cym_yellow)
                std::cout << "\t DPAD UP" << std::endl;
            if (dpad_right)
                std::cout << "\t DPAD RIGHT" << std::endl;
            if (a_green)
                std::cout << "\t A" << std::endl;
            if (b_red)
                std::cout << "\t B" << std::endl;
            if (x_blue)
                std::cout << "\t X" << std::endl;
            if (y_yellow)
                std::cout << "\t Y" << std::endl;
        }

        if (start)
            std::cout << "\t START" << std::endl;

        if (select)
            std::cout << "\t SELECT" << std::endl;

        if (guide)
            std::cout << "\t GUIDE" << std::endl;

        if (lb_kick)
            std::cout << "\t KICK" << std::endl;

        if (hihat)
            std::cout << "\t HH/Double Bass" << std::endl;
    }
} __attribute__((packed));

class GipDevice {
public:
    using SendPacket = std::function<bool(Bytes &data)>;

    bool handlePacket(const Bytes &packet);

protected:
    enum BatteryType {
        BATT_TYPE_CHARGING = 0x00,
        BATT_TYPE_ALKALINE = 0x01,
        BATT_TYPE_NIMH     = 0x02,
    };

    enum BatteryLevel {
        BATT_LEVEL_EMPTY = 0x00,
        BATT_LEVEL_LOW   = 0x01,
        BATT_LEVEL_MED   = 0x02,
        BATT_LEVEL_FULL  = 0x03,
    };

    // Controller input can be paused temporarily
    enum PowerMode {
        POWER_ON    = 0x00,
        POWER_SLEEP = 0x01,
        POWER_OFF   = 0x04,
    };

    enum LedMode {
        LED_OFF        = 0x00,
        LED_ON         = 0x01,
        LED_BLINK_FAST = 0x02,
        LED_BLINK_MED  = 0x03,
        LED_BLINK_SLOW = 0x04,
        LED_FADE_SLOW  = 0x08,
        LED_FADE_FAST  = 0x09,
    };

    struct AnnounceData {
        uint8_t  macAddress[6];
        uint16_t unknown;
        uint16_t vendorId;
        uint16_t productId;

        struct {
            uint16_t major;
            uint16_t minor;
            uint16_t build;
            uint16_t revision;
        } __attribute__((packed)) firmwareVersion, hardwareVersion;
    } __attribute__((packed));

    struct StatusData {
        uint32_t batteryLevel : 2;
        uint32_t batteryType : 2;
        uint32_t connectionInfo : 4;
        uint8_t  unknown1;
        uint16_t unknown2;
    } __attribute__((packed));

    struct GuideButtonData {
        uint8_t pressed;
        uint8_t unknown;
    } __attribute__((packed));

    struct RumbleData {
        uint8_t unknown;
        uint8_t setRight : 1;
        uint8_t setLeft : 1;
        uint8_t setRightTrigger : 1;
        uint8_t setLeftTrigger : 1;
        uint8_t padding : 4;
        uint8_t leftTrigger;
        uint8_t rightTrigger;
        uint8_t left;
        uint8_t right;
        uint8_t duration;
        uint8_t delay;
        uint8_t repeat;
    } __attribute__((packed));

    struct LedModeData {
        uint8_t unknown;
        uint8_t mode;
        uint8_t brightness;
    } __attribute__((packed));

    struct SerialData {
        uint16_t unknown;
        char     serialNumber[14];
    } __attribute__((packed));

    struct InputData {
        struct Buttons {
            uint32_t unknown : 2;
            uint32_t start : 1;
            uint32_t select : 1;
            uint32_t a : 1;
            uint32_t b : 1;
            uint32_t x : 1;
            uint32_t y : 1;
            uint32_t dpadUp : 1;
            uint32_t dpadDown : 1;
            uint32_t dpadLeft : 1;
            uint32_t dpadRight : 1;
            uint32_t bumperLeft : 1;
            uint32_t bumperRight : 1;
            uint32_t stickLeft : 1;
            uint32_t stickRight : 1;
            Buttons() {
                unknown     = 0;
                start       = 0;
                select      = 0;
                a           = 0;
                b           = 0;
                x           = 0;
                y           = 0;
                dpadUp      = 0;
                dpadDown    = 0;
                dpadLeft    = 0;
                dpadRight   = 0;
                bumperLeft  = 0;
                bumperRight = 0;
                stickLeft   = 0;
                stickRight  = 0;
            }
        } __attribute__((packed)) buttons;

        uint16_t triggerLeft;
        uint16_t triggerRight;
        int16_t  stickLeftX;
        int16_t  stickLeftY;
        int16_t  stickRightX;
        int16_t  stickRightY;
        InputData() {
            triggerLeft  = 0;
            triggerRight = 0;
            stickLeftX   = 0;
            stickLeftY   = 0;
            stickRightX  = 0;
            stickRightY  = 0;
        }
    } __attribute__((packed));

    GipDevice(SendPacket sendPacket);
    virtual ~GipDevice() = default;

    virtual void deviceAnnounced(uint8_t id, const AnnounceData *announce) = 0;
    virtual void statusReceived(uint8_t id, const StatusData *status)      = 0;
    virtual void guideButtonPressed(const GuideButtonData *button)         = 0;
    virtual void serialNumberReceived(const SerialData *serial)            = 0;
    virtual void inputReceived(const InputData *input)                     = 0;

    bool setPowerMode(uint8_t id, PowerMode mode);
    bool performRumble(RumbleData rumble);
    bool setLedMode(LedModeData mode);
    bool requestSerialNumber();
    bool xboxOneSReset(uint8_t id);
    bool identify(uint8_t id);

    SendPacket sendPacket;

private:
    bool    acknowledgePacket(Frame frame);
    uint8_t getSequence(bool accessory = false);

    uint8_t sequence          = 0x01;
    uint8_t accessorySequence = 0x01;
};
