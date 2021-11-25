/*
 * Copyright 2010 by Adam Mayer  <adam@makerbot.com>
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

#ifndef BOARDS_MB40_MOTHERBOARD_HH_
#define BOARDS_MB40_MOTHERBOARD_HH_

#include "DigiPots.hh"
#include "Types.hh"
#include "PSU.hh"
#include "Configuration.hh"
#include "Timeout.hh"
#include "LiquidCrystalSerial.hh"
#include "ButtonArray.hh"

/// Main class for Motherboard version 4.0+ (Gen4 electronics)
/// \ingroup HardwareLibraries
/// \ingroup MBv40
class Motherboard {
private:
  /// Static instance of the motherboard
  static Motherboard motherboard;

public:

  /// Get the motherboard instance.
  static Motherboard& getBoard() { return motherboard; }
  void initClocks();

private:
  
  /// Microseconds since board initialization
  volatile micros_t micros;
  
  /// Private constructor; use the singleton
  Motherboard();
  
  ButtonArray buttonArray;
  LiquidCrystalSerial lcd;
    
  micros_t restart_timeout;  
  

public:
  /// Reset the motherboard to its initial state.
  /// This only resets the board, and does not send a reset
  /// to any attached toolheads.
  void reset(bool hard_reset);

  /// initialize things that only need to be set up once, on boot
  void init();

  void runMotherboardSlice();

  /// Count the number of steppers available on this board.
  const int getStepperCount() const { return STEPPER_COUNT; }
    
  /// Get the number of microseconds that have passed since
  /// the board was initialized.  This value will wrap after
  /// 2**32 microseconds (ca. 70 minutes); callers should compensate for this.
  micros_t getCurrentMicros();

  /// Write an error code to the debug pin.
  void indicateError(int errorCode);
  /// Get the current error being displayed.
  uint8_t getCurrentError();
    
  /// set the interface LEDs to blink
  void interfaceBlink(int on_time, int off_time);

  /// Perform the timer interrupt routine.
  void doStepperInterrupt();
    
  /// update microsecond counter
  void UpdateMicros();

};



#endif // BOARDS_MB40_MOTHERBOARD_HH_
