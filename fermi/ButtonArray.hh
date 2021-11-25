#ifndef BUTTONARRAY_HH
#define BUTTONARRAY_HH

#include <util/atomic.h>

enum Button {
  CENTER          = 1<<2,
  RIGHT           = 1<<3,
  LEFT            = 1<<6,
  DOWN            = 1<<4,
  UP              = 1<<5,
};

namespace ButtonArray {
        
  void init();

  // Returns true if any of the button states have changed.
  uint8_t pressed();
};


#endif // BUTTONARRAY_HH
