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

#ifndef MOTION_HH_
#define MOTION_HH_

#include "Configuration.hh"
#include "Pin.hh"
#include "Types.hh"
#include <stdint.h>
#include <stdlib.h>

namespace motion {

enum { X, Y };
void reset_axes();
void init();
void enable(uint8_t which, bool enable = true);
void setPotValue(const Pin &pin, uint8_t val);
/// Return the current position of the given axis. This will be unreliable if the
/// axis is still in motion!
double get_axis_position(uint8_t which);

/// Check if there's space on the movement queue for another move
/// or dwell
bool queue_ready();
bool enqueue_move(float x, float y, float feed);
bool enqueue_dwell(float milliseconds);
bool enqueue_pen(bool up);
bool queue_done();

void do_interrupt();
}; // namespace motion

#endif // MOTION_HH_
