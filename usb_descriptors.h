//---------------------------------------------------------------------------

#pragma once
#include "tusb_config.h"

//---------------------------------------------------------------------------

enum {
  ITF_NUM_KEYBOARD,
  ITF_NUM_CONSOLE,
  ITF_NUM_PLOVER_HID,
  ITF_NUM_CDC,
  ITF_NUM_CDC_DATA,
  ITF_NUM_TOTAL,
};

//---------------------------------------------------------------------------

const int KEYBOARD_PAGE_REPORT_ID = 1;
const int CONSUMER_PAGE_REPORT_ID = 2;

//---------------------------------------------------------------------------
