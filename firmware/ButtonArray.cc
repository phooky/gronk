#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static const uint8_t ARROW_BUTTON_MAP = 0x78;
static const uint8_t CENTER_BUTTON_MAP = 0x04;

namespace ButtonArray {

uint8_t previous;
uint8_t have_presses;

void init() {
  previous = 0;

  // Set all of the known buttons to inputs
  DDRJ &= ~(ARROW_BUTTON_MAP);
  PORTJ &= ~(ARROW_BUTTON_MAP);

  have_presses = 0;

  INTERFACE_CENTER.setDirection(false);
}

void scan() {
  uint8_t cur = (PINJ & ARROW_BUTTON_MAP) | (PING & CENTER_BUTTON_MAP);
  have_presses |= previous & (cur ^ previous);
  previous = cur;
}

uint8_t pressed() {
  uint32_t rv = have_presses;
  have_presses = 0;
  return rv;
}
} // namespace ButtonArray
