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
 *
 * Modifications for Jetty Marlin compatability, authored by Dan Newman and Jetty.
 */

#define __STDC_LIMIT_MACROS
#include "Steppers.hh"
#include "SoftI2cManager.hh"
#include <stdint.h>

#define A_STEPPER_MIN NullPin
#define A_STEPPER_MAX NullPin
#define B_STEPPER_MIN NullPin
#define B_STEPPER_MAX NullPin

extern int intdbg;
///assume max vref is 1.95V  (allowable vref for max current rating of stepper is 1.814)
#define DIGI_POT_MAX	118

#define AXIS_PIN_SET(axis) { \
  axis ## _STEPPER_STEP,     \
    axis ## _STEPPER_DIR,    \
    axis ## _STEPPER_ENABLE, \
    axis ## _STEPPER_MIN,    \
    axis ## _STEPPER_MAX,    \
    axis ## _POT_PIN }

namespace steppers {

  typedef struct {
    int32_t position;
    int16_t velocity;
    int16_t acceleration;
  } StepAxisInfo;

  StepAxisInfo axis[MAX_STEPPERS];

  typedef struct {
    const Pin step;
    const Pin dir;
    const Pin en;
    const Pin min;
    const Pin max;
    const Pin pot;
  } StepPins;

  const StepPins stepPins[STEPPER_COUNT] = {
    { X_STEPPER_STEP, X_STEPPER_DIR, X_STEPPER_ENABLE,
      X_STEPPER_MIN, X_STEPPER_MAX, X_POT_PIN },
    AXIS_PIN_SET(Y),
    AXIS_PIN_SET(Z),
    AXIS_PIN_SET(A),
    AXIS_PIN_SET(B) };

  void setPotValue(const Pin& pin, uint8_t val)
  {
    SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
    i2cPots.start(0b01011110 | I2C_WRITE, pin);
    if (val > DIGI_POT_MAX) val = DIGI_POT_MAX;
    i2cPots.write(val, pin);
    i2cPots.stop(); 
  }

  void init_pots() {
    // XYAB settings should be around 118
    // Z setting should be around 40
    setPotValue(stepPins[0].pot, 118);
    setPotValue(stepPins[1].pot, 118);
  }

  /// This is the bit of the position that is copied to the
  /// step line. All the bits to the left encode the actual
  /// position; those to the right are below the resolution
  /// of the physical system.
  const uint8_t STEP_BIT = 16;

  void reset_axes() {
    for (StepAxisInfo& a : axis) {
      a.position = 0;
      a.velocity = 0;
      a.acceleration = 0;
    }
  }

  void init_pins() {
    for (const StepPins& pins : stepPins) {
      // Make sure each stepper is initialized in a disabled state.
      // Enable is active low!!!
      pins.en.setValue(true);
      pins.en.setDirection(true);
      pins.step.setValue(false);
      pins.step.setDirection(true);
      pins.dir.setValue(true);
      pins.dir.setDirection(true);
    }
  }

  void init() {
    init_pots();
    init_pins();
    reset_axes();    
  }

  void enable(uint8_t which, bool enable) {
    stepPins[which].en.setValue(!enable);
  }    

  void set_velocity(uint8_t which, int16_t velocity) {
    cli();
    axis[which].velocity = velocity;
    sei();
  }

  void do_interrupt() {
    cli();
    for (int i = 0; i < 2; i++) {
      auto& a = axis[i]; const auto& p = stepPins[i];
      p.dir.setValue(!(a.velocity & (1L<<15)));
      a.position += a.velocity;
      p.step.setValue(a.position & (1L<<15));
    }
    sei();
  }
};
