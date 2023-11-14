#ifndef __FIXED32_HH__
#define __FIXED32_HH__
#include <stdint.h>

// 16.16
class Fixed32 {
public:
  union {
    struct {
      uint16_t frac_p;
      int16_t int_p;
    } v16;
    int32_t v32;
  } v;

  void reset() { v.v32 = 0; }
  void negate() { v.v32 = -v.v32; }

  int16_t& int_part() { return v.v16.int_p; }
  uint16_t& frac_part() { return v.v16.frac_p; }
};
#endif
