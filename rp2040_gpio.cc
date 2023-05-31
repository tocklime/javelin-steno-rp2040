//---------------------------------------------------------------------------

#include "javelin/hal/gpio.h"
#include <hardware/gpio.h>

//---------------------------------------------------------------------------

void Gpio::SetPin(int pin, bool value) {
  gpio_init(pin);
  gpio_set_dir(pin, true);
  gpio_put(pin, value);
}

//---------------------------------------------------------------------------