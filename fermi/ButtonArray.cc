#include "ButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;
static uint8_t previousG;

static const uint8_t ARROW_BUTTON_MAP = 0x78;
static const uint8_t CENTER_BUTTON_MAP = 0x04;

void ButtonArray::init() {
        previousJ = 0;
        previousG = 0;
        
        // Set all of the known buttons to inputs 
        DDRJ  &= ~( ARROW_BUTTON_MAP );
        PORTJ &= ~( ARROW_BUTTON_MAP );
        
        INTERFACE_CENTER.setDirection(false);
}

uint8_t ButtonArray::pressed() {
  return (PINJ & ARROW_BUTTON_MAP) | (PING & CENTER_BUTTON_MAP);
}
