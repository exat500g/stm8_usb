#include "stm8s.h"

const uint8_t usb_device_descriptor[] = {
    0x12, // Size of the Descriptor in Bytes
    0x01, // Device Descriptor (0x01)
    0x10,
    0x01, // USB 1.1 = 0x0110， USB 1.0 = 0x0100
    0x00, // Class Code
    0x00, // Subclass Code
    0x00, // Protocol Code
    0x08, // Maximum Packet Size for Zero Endpoint
    0x11,0x22, // VID
    0x33,0x44,// PID
    0x55,0x66, // Device Release Number
    0x00, // Index of Manufacturer String Descriptor
    0x00, // Index of Product String Descriptor
    0x00, // Index of Serial Number String Descriptor
    0x01, // Number of Possible Configurations
};

const uint8_t usb_configuration_descriptor[] = {
    0x09, // Size of Descriptor in Bytes
    0x02, // Configuration Descriptor (0x02)
    34,
    0x00, // Total length in bytes of data returned
    0x01, // Number of Interfaces
    0x01, // Value to use as an argument to select this configuration
    0x00, // Index of String Descriptor describing this configuration
    0x80, // D7 Reserved, set to 1. (USB 1.0 Bus Powered), D6 Self Powered, D5 Remote Wakeup, D4..0 Reserved, set to 0.
    50,   // Maximum Power Consumption in 2mA units

    // interface descriptor
    0x09, // Size of Descriptor in Bytes (9 Bytes)
    0x04, // Interface Descriptor (0x04)
    0x00, // Number of Interface
    0x00, // Value used to select alternative setting
    0x01, // Number of Endpoints used for this interface
    0x03, // Class Code
    0x01, // Subclass Code
    0x01, // Protocol Code,1=keyboard,2=mouse
    0x00, // Index of String Descriptor Describing this interface

    // HID descriptor
    9,    // Size of Descriptor in Bytes (9 Bytes)
    0x21, // HID descriptor (0x21)
    0x10,
    0x01, // BCD representation of HID version
    0x21, // Target country code
    0x01, // Number of HID Report (or other HID class) Descriptor infos to follow */
    0x22, // Descriptor type: report
    25,
    0,  /* total length of report descriptor */

    // Endpoint descriptor
    0x07, // Size of Descriptor in Bytes (7 Bytes)
    0x05, // Endpoint descriptor (0x05)
    0x81, // IN endpoint number 1 (0x81)
    0x03, // attrib: Interrupt endpoint
    0x04, //
    0x00, // maximum packet size
    100  // POLL INTERVAL (ms)
};

const uint8_t usb_report_descriptor[25] = {
    0x05, 0x01,                    //    USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    //    USAGE (Keyboard)
    0xa1, 0x01,                    //    COLLECTION (Application)
    0x05, 0x07,                    //    USAGE_PAGE (Keyboard)
    //（5）
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0xFF,                    //   LOGICAL_MAXIMUM (255)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)

    0xc0,                          //  END_COLLECTION
};
