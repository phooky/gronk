#ifndef PIN_HH
#define PIN_HH

#include "AvrPort.hh"
#include "Pin.hh"

/// \ingroup HardwareLibraries
class Pin {
  public:
    const port_base_t port_base;
    const uint8_t pin_index;
    const uint8_t pin_mask;
    const uint8_t pin_mask_inverted;

  public:
    Pin(const port_base_t port_in, const uint8_t pin_index_in);
    // Pin(const Pin& other_pin);
    void setDirection(bool out) const;

    void setValue(bool value) const;
    bool getValue() const;
    bool isNull() const { return port_base == NULL_PORT; }
};

static const Pin NullPin(NULL_PORT, 0);

#endif // PIN_HH
