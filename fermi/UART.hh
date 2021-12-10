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

#ifndef UART_HH_
#define UART_HH_

#include <stdint.h>

namespace UART {

  /// Initialize the UART module
  void initialize();

  /// Queue a byte for writing; returns true if successfully queued
  bool write(uint8_t byte);

  /// Queue the given number of bytes of the buffer; returns the
  /// number of bytes successfully queued
  int write_buffer(uint8_t* buf, uint8_t length);

  /// Return the amount of data available on the uart
  int available();

  /// Read N bytes from the uart into the buffer
  /// Returns the number of bytes read
  int read(uint8_t* buf, uint8_t n);
};

#endif // UART_HH_
