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

#ifndef SHARED_AVR_PORT_HH_
#define SHARED_AVR_PORT_HH_

#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <util/atomic.h>

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) ||                \
    defined(__AVR_ATmega644P__)

typedef uint8_t port_base_t;
const port_base_t NULL_PORT = 0xff;

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

typedef uint16_t port_base_t;
const port_base_t NULL_PORT = 0xffff;

#endif

#include <avr/io.h>

// The AVR port and pin mapping is based on a convention that has held true for
// all ATMega chips released so far: that the ports begin in sequence from
// register 0x00 from A onwards, and are arranged: 0 PINx 1 DDRx 2 PORTx This is
// verified true for the 168/328/644p/1280/2560.

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) ||                \
    defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1280__) ||              \
    defined(__AVR_ATmega2560__)
#else
#error Unsupported processor
#endif

#define PINx _SFR_MEM8(port_base + 0)
#define DDRx _SFR_MEM8(port_base + 1)
#define PORTx _SFR_MEM8(port_base + 2)

#if defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1280__) ||              \
    defined(__AVR_ATmega2560__)
const port_base_t PortA(0x20);
#endif // __AVR_ATmega644P__
const port_base_t PortB(0x23);
const port_base_t PortC(0x26);
const port_base_t PortD(0x29);
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
const port_base_t PortE(0x2C);
const port_base_t PortF(0x2F);
const port_base_t PortG(0x32);
const port_base_t PortH(0x100);
const port_base_t PortJ(0x103);
const port_base_t PortK(0x106);
const port_base_t PortL(0x109);
#endif //__AVR_ATmega1280__

// Macro to expand steppers into Port, Pin, PinNumber and DDR
#define STEPPER_PORT(PLETTER, PNUMBER)                                         \
  {                                                                            \
    _SFR_MEM_ADDR(PORT##PLETTER), _SFR_MEM_ADDR(PIN##PLETTER),                 \
        PIN##PLETTER##PNUMBER, _SFR_MEM_ADDR(DDR##PLETTER)                     \
  }

#endif // SHARED_AVR_PORT_HH_
