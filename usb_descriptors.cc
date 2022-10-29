/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "usb_descriptors.h"
#include "tusb.h"
#include "uniV4/config.h"

/* A combination of interfaces must have a unique product id, since PC will save
 * device driver after the first plug. Same VID/PID with different interface e.g
 * MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID                                                                \
  (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) |           \
   _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
const tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = VENDOR_ID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
const uint8_t *tud_descriptor_device_cb(void) {
  return (const uint8_t *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// clang-format off
const uint8_t desc_hid_keyboard_report[] = {
    // Keyboard
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_KEYBOARD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    // Bitmap of modifiers.
    HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),
      HID_USAGE_MIN(224),
      HID_USAGE_MAX(231),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(1),
      HID_REPORT_COUNT(8),
      HID_REPORT_SIZE(1),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    // LED Indicator
    HID_USAGE_PAGE(HID_USAGE_PAGE_LED),
      HID_USAGE_MIN(1),
      HID_USAGE_MAX(5),
      HID_REPORT_COUNT(5),
      HID_REPORT_SIZE(1),
      HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    // Padding
      HID_REPORT_COUNT(1),
      HID_REPORT_SIZE(3),
      HID_OUTPUT(HID_CONSTANT),
    // Bitmap of keys.
      HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),
      HID_USAGE_MIN(0),
      HID_USAGE_MAX(223),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(1),
      HID_REPORT_COUNT(224),
      HID_REPORT_SIZE(1),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),
      HID_USAGE_MIN(232),
      HID_USAGE_MAX(255),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(1),
      HID_REPORT_COUNT(24),
      HID_REPORT_SIZE(1),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,
};

const uint8_t desc_hid_console_report[] = {
    // Console
    0x06, 0x31, 0xFF, // HID_USAGE_PAGE(0xff31)
    HID_USAGE(0x0074),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    // Send
      HID_USAGE(0x75),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(255),
      HID_REPORT_COUNT(64),
      HID_REPORT_SIZE(8),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    // Receive
      HID_USAGE(0x76),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(255),
      HID_REPORT_COUNT(64),
      HID_REPORT_SIZE(8),
      HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,
};
// clang-format on

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) {
  switch (instance) {
  case ITF_NUM_KEYBOARD:
    return desc_hid_keyboard_report;

  case ITF_NUM_CONSOLE:
    return desc_hid_console_report;

  default:
    __builtin_unreachable();
  }
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#define CONFIG_TOTAL_LEN                                                       \
  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN +           \
   TUD_CDC_DESC_LEN)

#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X ||                                      \
    CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || CFG_TUSB_MCU == OPT_MCU_LPC40XX
// LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
// 1 Interrupt, 2 Bulk, 3 Iso, 4 Interrupt, 5 Bulk etc ...
// #define EPNUM_KEYBOARD 0x81

// #define EPNUM_CONSOLE_OUT 0x04
// #define EPNUM_CONSOLE_IN 0x84

// #define EPNUM_CDC_NOTIF 0x87
// #define EPNUM_CDC_OUT 0x08
// #define EPNUM_CDC_IN 0x88
#else
#define EPNUM_KEYBOARD 0x81

#define EPNUM_CONSOLE_OUT 0x02
#define EPNUM_CONSOLE_IN 0x82

#define EPNUM_CDC_NOTIF 0x83
#define EPNUM_CDC_OUT 0x04
#define EPNUM_CDC_IN 0x84
#endif

const uint8_t desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_KEYBOARD, 0, HID_ITF_PROTOCOL_KEYBOARD,
                       sizeof(desc_hid_keyboard_report), EPNUM_KEYBOARD,
                       CFG_TUD_HID_EP_BUFSIZE, 1),

    // HID Input & Output descriptor
    // Interface number, string index, protocol, report descriptor len,
    // EP OUT & IN address, size & polling interval
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_CONSOLE, 0, HID_ITF_PROTOCOL_NONE,
                             sizeof(desc_hid_console_report), EPNUM_CONSOLE_OUT,
                             EPNUM_CONSOLE_IN, CFG_TUD_CONSOLE_BUFSIZE, 1),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT,
                       EPNUM_CDC_IN, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index; // for multiple configurations
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
const char *const string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    MANUFACTURER_NAME,          // 1: Manufacturer
    PRODUCT_NAME,               // 2: Product
    "",                         // 3: Serials, should use chip ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;

  uint8_t chr_count;

  if (index == 0) {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  } else {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
      return NULL;

    const char *str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if (chr_count > 31)
      chr_count = 31;

    // Convert ASCII string into UTF-16
    for (uint8_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

  return _desc_str;
}
