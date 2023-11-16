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
 * Modifications for Jetty Marlin compatability, authored by Dan Newman and
 * Jetty.
 */

#define __STDC_LIMIT_MACROS
#include "Steppers.hh"
#include "CBuf.hh"
#include "SoftI2cManager.hh"
#include <stdint.h>
#include <math.h>

#define A_STEPPER_MIN NullPin
#define A_STEPPER_MAX NullPin
#define B_STEPPER_MIN NullPin
#define B_STEPPER_MAX NullPin

extern int intdbg;
/// assume max vref is 1.95V  (allowable vref for max current rating of stepper
/// is 1.814)
#define DIGI_POT_MAX 118

#define AXIS_PIN_SET(axis)                                                     \
    {                                                                          \
        axis##_STEPPER_STEP, axis##_STEPPER_DIR, axis##_STEPPER_ENABLE,        \
            axis##_STEPPER_MIN, axis##_STEPPER_MAX, axis##_POT_PIN             \
    }

const float STEPS_PER_MM[2] = {
    STEPS_PER_MM_X, STEPS_PER_MM_Y,
};

namespace steppers {

class MovementCmd {
public:
    float target[2]; // only handling X/Y, let's dump the rest
    float velocity;
    MovementCmd(float x, float y, float vel) : target{x,y}, velocity(vel) {}
};

class PenCmd {
public:
    bool up;
};

class DwellCmd {
public:
    uint16_t milliseconds;
};

typedef enum {
    CT_NONE = 0, CT_MOVE, CT_PEN, CT_DWELL
} CommandType;
    
class Command {
public:
    CommandType type;
    union {
        MovementCmd move;
        PenCmd pen;
        DwellCmd dwell;
    };
    Command() : type(CT_NONE) {}
    Command(float x, float y, float vel) : type(CT_MOVE), move(MovementCmd(x,y,vel)) {}
};

CBuf<32, Command> cmd_q;
Command cur_cmd = Command();
float last_pos[2] = { 0,0 };


/// Stepper internals
typedef struct {
    int32_t position; // in steps
    int32_t target; // in steps
    int32_t velocity;
    int32_t acceleration; // ignore for the moment
} StepAxisInfo;

StepAxisInfo axis[MAX_STEPPERS];
uint32_t cycles_remaining_in_command = 0;

typedef struct {
    const Pin step;
    const Pin dir;
    const Pin en;
    const Pin min;
    const Pin max;
    const Pin pot;
} StepPins;

const StepPins stepPins[STEPPER_COUNT] = {AXIS_PIN_SET(X), AXIS_PIN_SET(Y),
                                          AXIS_PIN_SET(Z), AXIS_PIN_SET(A),
                                          AXIS_PIN_SET(B)};

void setPotValue(const Pin &pin, uint8_t val) {
    SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
    i2cPots.start(0b01011110 | I2C_WRITE, pin);
    if (val > DIGI_POT_MAX)
        val = DIGI_POT_MAX;
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
    for (StepAxisInfo &a : axis) {
        a.position = 0;
        a.velocity = 0;
        a.acceleration = 0;
    }
}

void next_cmd() {
    if (!cmd_q.empty()) {
        cur_cmd = cmd_q.dequeue();
    }
    switch(cur_cmd.type) {
    case CT_NONE:
    case CT_PEN:
    case CT_DWELL:
        return;
    case CT_MOVE:
        {
            float distance = 0;
            for (int i = 0; i < 2; i++) {
                float axis_d = cur_cmd.move.target[i] - last_pos[i];
                distance += axis_d * axis_d;
            }
            distance = sqrt(distance);
            // Velocity is ALWAYS IN MM/S
            cycles_remaining_in_command =
                (distance / cur_cmd.move.velocity) *  // time in seconds
                STEPPER_FREQ;
            for (int i = 0; i < 2; i++) {
                auto& a = axis[i];
                float axis_d_steps = (cur_cmd.move.target[i] - last_pos[i]) * STEPS_PER_MM[i];
                float vel =
                    (axis_d_steps / cycles_remaining_in_command) * // steps per cycle
                    (float)((int32_t)1<<15);
                a.velocity = vel;
            }
            for (int i = 0; i < 2; i++)
                last_pos[i] = cur_cmd.move.target[i];
            
        }
};

    // TODO
}

    
void init_pins() {
    for (const StepPins &pins : stepPins) {
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

/// Check if there's space on the movement queue for another move
/// or dwell
bool queue_ready() { return !cmd_q.full(); }

bool enqueue_move(float x, float y, float feedrate) {
    // Stepper loop frequency: 7812.5Hz
    cmd_q.queue(Command(x,y,feedrate));
    return true;
}

bool enqueue_dwell(uint16_t milliseconds) { return true; }

void do_interrupt() {
    cli();
    if (cycles_remaining_in_command == 0) {
        next_cmd();
    }
    if (cycles_remaining_in_command > 0) {
        for (int i = 0; i < 2; i++) {
            auto &a = axis[i];
            const auto &p = stepPins[i];
            p.dir.setValue( a.velocity > 0 );
            a.position += a.velocity;
            p.step.setValue(a.position & (1L << 15));
        }
        cycles_remaining_in_command--;
    }
    sei();
}
}; // namespace steppers
