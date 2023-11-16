#ifndef __FIXED32_HH__
#define __FIXED32_HH__
#include <stdint.h>

// 16.16
class Fixed32 {
  public:
    union val {
        struct {
            uint16_t frac_p;
            int16_t int_p;
        } v16;
        int32_t v32;
        val() : v32(0) {}
        val(int32_t v) : v32(v) {}
        val(int16_t i, uint16_t f) : v16({f,i}) {}
    } v;

    Fixed32(int16_t ip, uint16_t fp) : v(ip, fp) {}
    Fixed32(int32_t iv) : v(iv) {}
    Fixed32() : v() {}
    void reset() { v.v32 = 0; }
    void negate() { v.v32 = -v.v32; }

    const Fixed32 operator * (const Fixed32& other) {
        return Fixed32(v.v32 * other.v.v32);
    }

    int16_t &int_part() { return v.v16.int_p; }
    uint16_t &frac_part() { return v.v16.frac_p; }
};
#endif
