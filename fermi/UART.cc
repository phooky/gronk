/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#include "UART.hh"
#include "Pin.hh"
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/io.h>

bool ovfl_in = false;

// We support three platforms: Atmega168 (1 UART), Atmega644, and Atmega1280/2560
#if defined (__AVR_ATmega168__)     \
    || defined (__AVR_ATmega328__)  \
    || defined (__AVR_ATmega644P__) \
    || defined (__AVR_ATmega1280__) \
    || defined (__AVR_ATmega2560__)
#else
    #error UART not implemented on this processor type!
#endif

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__)

    #define UBRR_VALUE 25
    #define UCSR0A_VALUE 0

    #define INIT_SERIAL(uart_) \
    { \
        UBRR0H = UBRR_VALUE >> 8; \
        UBRR0L = UBRR_VALUE & 0xff; \
        \
        /* set config for uart, explicitly clear TX interrupt flag */ \
        UCSR0A = UCSR0A_VALUE | _BV(TXC0); \
        UCSR0B = _BV(RXEN0) | _BV(TXEN0); \
        UCSR0C = _BV(UCSZ01)|_BV(UCSZ00); \
    }

#elif defined (__AVR_ATmega644P__)

    #define UBRR_VALUE 25
    #define UBRRA_VALUE 0

    // Adapted from ancient arduino/wiring rabbit hole
    #define INIT_SERIAL(uart_) \
    { \
        UBRR##uart_##H = UBRR_VALUE >> 8; \
        UBRR##uart_##L = UBRR_VALUE & 0xff; \
        \
        /* set config for uart_ */ \
        UCSR##uart_##A = UBRRA_VALUE; \
        UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_); \
        UCSR##uart_##C = _BV(UCSZ##uart_##1)|_BV(UCSZ##uart_##0); \
    }

#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)

    // Use double-speed mode for more accurate baud rate?
    #define UBRR0_VALUE 16 // 115200 baud
    #define UBRR1_VALUE 51 // 38400 baud
    #define UCSRA_VALUE(uart_) _BV(U2X##uart_)

    // Adapted from ancient arduino/wiring rabbit hole
    #define INIT_SERIAL(uart_) \
    { \
        UBRR##uart_##H = UBRR##uart_##_VALUE >> 8; \
        UBRR##uart_##L = UBRR##uart_##_VALUE & 0xff; \
        \
        /* set config for uart_ */ \
        UCSR##uart_##A = UCSRA_VALUE(uart_); \
        UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_); \
        UCSR##uart_##C = _BV(UCSZ##uart_##1)|_BV(UCSZ##uart_##0); \
    }
#endif

#define ENABLE_SERIAL_INTERRUPTS(uart_) \
{ \
UCSR##uart_##B |= _BV(RXCIE##uart_) | _BV(TXCIE##uart_); \
}

#define DISABLE_SERIAL_INTERRUPTS(uart_) \
{ \
UCSR##uart_##B &= ~(_BV(RXCIE##uart_) | _BV(TXCIE##uart_)); \
}


// Circular buffer implementation
template <int N>
class CBuf {
public:
  const uint8_t blen = N;
  uint8_t buf[N];
  uint8_t data; // index to first data byte
  uint8_t sz; // size of valid data
  CBuf() : data(0), sz(0) {}
  
  bool full() {
    return sz == blen;
  }

  bool empty() {
    return sz == 0;
  }
  
  bool queue(uint8_t in) {
    if (full()) return false;
    cli();
    buf[(data+sz)%blen] = in;
    sz++;
    sei();
    return true;
  }

  uint8_t dequeue() {
    if (empty()) return 0xff; // Should check for empty first
    cli();
    uint8_t rv = buf[data];
    sz--;
    data = (data + 1)%blen;
    sei();
    return rv;
  }
};

CBuf<128> out_buf;
CBuf<128> in_buf;
  
void initialize() {
  INIT_SERIAL(0);
}

bool write(uint8_t byte) {
  if (out_buf.empty()) {
    UDR0 = byte; return true;
  }
  else return out_buf.queue(byte);
}

// Queue the given number of bytes of the buffer; returns the
// number of bytes successfully queued
int write_buffer(uint8_t* buf, uint8_t length) {
  int idx = 0;
  while (idx < length) {
    if (!write(buf[idx])) return idx;
    idx++;
  }
  return idx;
}

/// Return the amount of data available on the uart
int available() {
  return in_buf.sz;
}

/// Read N bytes from the uart into the buffer
/// Returns the number of bytes read
int read(uint8_t* buf, uint8_t n) {
  cli();
  if (in_buf.sz < n) { n = in_buf.sz; }
  sei();
  uint8_t nc = n;
  while (n) {
    *(buf++) = in_buf.dequeue();
    n--;
  }
  return nc;
}

void enable(bool enabled) {
  if (enabled) { ENABLE_SERIAL_INTERRUPTS(0); }
  else { DISABLE_SERIAL_INTERRUPTS(0); }
}

// Send and receive interrupts
ISR(USART0_RX_vect)
{
  ovfl_in = ovfl_in || !out_buf.queue(UDR0);
}

ISR(USART0_TX_vect)
{
  if (!out_buf.empty()) UDR0 = out_buf.dequeue();
}
