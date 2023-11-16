#ifndef __CBUF_HH__
#define __CBUF_HH__

#include <avr/interrupt.h>

///
/// Generalized simple circular buffer implementation
///

// Circular buffer implementation
template <int N, typename T> class CBuf {
  public:
    const uint8_t blen = N;
    T buf[N];
    uint8_t data; // index to first data byte
    uint8_t sz;   // size of valid data
    CBuf() : data(0), sz(0) {}

    bool full() { return sz == blen; }

    bool empty() { return sz == 0; }

    bool queue(T in) {
        if (full())
            return false;
        cli();
        buf[(data + sz) % blen] = in;
        sz++;
        sei();
        return true;
    }

    T& last() {
        if (empty()) return buf[0]; // Better than corruption
        uint8_t off = (data + sz - 1) % blen;
        return buf[data];
    }
    
    T dequeue() {
        if (empty())
            return T(); // Better than corruption; should check for empty first
        cli();
        T rv = buf[data];
        sz--;
        data = (data + 1) % blen;
        sei();
        return rv;
    }
};

#endif
