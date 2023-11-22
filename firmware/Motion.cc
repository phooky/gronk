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
#include "Motion.hh"
#include "CBuf.hh"
#include "SoftI2cManager.hh"
#include <stdint.h>
#include <math.h>
#include "UART.hh"
#include <stdio.h>

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

namespace motion {

typedef int64_t Steps;

/// Motion commands for the command queue.

class MotionCmd {
public:
    enum CmdType { NONE, MOVE, DWELL, PEN };
    CmdType type;
    union {
        struct {
            Steps target[2];
            int32_t velocity[2];
            int32_t acceleration[2];
        } move;
        bool pen;
    };
    uint32_t time;
    MotionCmd() : type (CmdType::NONE) {}
};

CBuf<32,MotionCmd> motion_q;
MotionCmd cur_cmd; // command currently being executed


int64_t last_pos[2] = { 0,0 };

class StepAxisInfo {
public:
    Steps position; // in steps
    int32_t partial; // partial steps moved
    Steps target; // in steps
    int32_t velocity;
    int32_t acceleration; // ignore for the moment
    StepAxisInfo() { reset(); }
    void reset() {
        position = 0;
        partial = 0;
        target = 0;
        velocity = 0;
        acceleration = 0;
    }
};

StepAxisInfo axis[MAX_STEPPERS];

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
        a.reset();
    }
    last_pos[0] = last_pos[1] = 0;
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
    PEN_IN_A.setValue(false);
    PEN_IN_A.setDirection(true);
    PEN_IN_B.setValue(false);
    PEN_IN_B.setDirection(true);
    PEN_PWM.setValue(false);
    PEN_PWM.setDirection(true);
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

//// COMMAND QUEUE STUFF START
/// Check if there's space on the movement queue for another move
/// or dwell
bool queue_ready() { return !motion_q.full(); }

bool enqueue_move(float x, float y, float feedrate) {
    int64_t pt[2] = {
        (int64_t)(x * STEPS_PER_MM[0]),
        (int64_t)(y * STEPS_PER_MM[1]),
    };
    int64_t delta[2] = { pt[0] - last_pos[0], pt[1] - last_pos[1] };
    MotionCmd cmd;
    cmd.type = MotionCmd::CmdType::MOVE;
    // The feedrate conversion is a little painful, but whatever.
    double d0 = (double)(delta[0]/STEPS_PER_MM[0]);
    double d1 = (double)(delta[1]/STEPS_PER_MM[1]);
    double distance = sqrt(d0*d0 + d1*d1); // distance in mm
    double cycles = (distance / feedrate) * STEPPER_FREQ; // stepper interrupt count
    cmd.time = cycles;
    for (auto i = 0; i < 2; i++) {
        cmd.move.target[i] = pt[i];
        /// Constraint: the time is larger than the number of steps.
        /// partials are 24 bits in a 32-bit variable.
        cmd.move.velocity[i] = ((delta[i] * (1L<<24)) / (int64_t)cmd.time);
        cmd.move.acceleration[i] = 0;
        last_pos[i] = pt[i];

        //char buf[60];
        //sprintf(buf,"%ld",cmd.time);
        //UART::write_string(buf);
    }
    motion_q.queue(cmd);
    return true;
}

bool enqueue_dwell(float milliseconds) {
    MotionCmd cmd;
    cmd.type = MotionCmd::CmdType::DWELL;
    float cycles = (milliseconds/1000)*STEPPER_FREQ;
    cmd.time = cycles;
    motion_q.queue(cmd);
    return true;
}

bool enqueue_pen(bool up) {
    MotionCmd cmd;
    cmd.type = MotionCmd::CmdType::PEN;
    cmd.time = up?UP_CYCLES:DOWN_CYCLES;
    cmd.pen = up;
    motion_q.queue(cmd);
    return true;
}

//// COMMAND QUEUE END


void next_cmd() {
    // set pen back to idle power if previous motion was pen up/pen down
    if (cur_cmd.type == MotionCmd::CmdType::PEN) {
        if (cur_cmd.pen) {
            PEN_PWM.setValue(0);
        } else {
            PEN_PWM.setValue(1);
        }
    }
    if (!motion_q.empty()) {
        cur_cmd = motion_q.dequeue();
    } else {
        cur_cmd = MotionCmd(); // None
    }
    for (int i = 0; i < 2; i++) {
        auto& a = axis[i];
        if (cur_cmd.type == MotionCmd::CmdType::MOVE)
            a.velocity = cur_cmd.move.velocity[i];
        else
            a.velocity = 0;
    }
    if (cur_cmd.type == MotionCmd::CmdType::PEN) {
        PEN_IN_A.setValue(cur_cmd.pen);
        PEN_IN_B.setValue(!cur_cmd.pen);
        PEN_PWM.setValue(1);
   }
}

void do_interrupt() {
    cli();
    if (cur_cmd.time == 0 || cur_cmd.type == MotionCmd::CmdType::NONE) {
        next_cmd();
    } else {
        if (cur_cmd.time > 0) {
            if (cur_cmd.type == MotionCmd::CmdType::MOVE) {
                for (int i = 0; i < 2; i++) {
                    auto &a = axis[i];
                    const auto &p = stepPins[i];
                    p.dir.setValue(cur_cmd.move.velocity[i] > 0 );
                    a.partial += a.velocity;
                    p.step.setValue(a.partial & (1L << 24));
                }
            } else if (cur_cmd.type == MotionCmd::CmdType::PEN) {
                // Do pen raise/lower
            }
        }
        cur_cmd.time--;
    }
    sei();
}
}; // namespace motion
