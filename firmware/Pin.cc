#include "Pin.hh"

Pin::Pin(const port_base_t port_in, uint8_t pin_index_in)
    : port_base(port_in), pin_index(pin_index_in), pin_mask(1 << pin_index_in),
      pin_mask_inverted(~(1 << pin_index_in)) {}

void Pin::setDirection(bool out) const {
    if (out) {
        DDRx |= pin_mask;
    } else {
        DDRx &= pin_mask_inverted;
    }
}

bool Pin::getValue() const {
    return (uint8_t)((uint8_t)PINx & (uint8_t)pin_mask) != 0;
}

void Pin::setValue(bool on) const {
    if (on) {
        PORTx |= pin_mask;
    } else {
        PORTx &= pin_mask_inverted;
    }
}
