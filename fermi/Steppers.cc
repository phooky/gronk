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
#include <stdint.h>

#define A_STEPPER_MIN NullPin
#define A_STEPPER_MAX NullPin
#define B_STEPPER_MIN NullPin
#define B_STEPPER_MAX NullPin

#define AXIS_PIN_SET(axis) { \
  axis ## _STEPPER_STEP,     \
    axis ## _STEPPER_DIR,    \
    axis ## _STEPPER_ENABLE, \
    axis ## _STEPPER_MIN,    \
    axis ## _STEPPER_MAX }

namespace steppers {

  typedef struct {
    int32_t position;
    int16_t velocity;
    int16_t acceleration;
  } StepAxisInfo;

  StepAxisInfo axis[MAX_STEPPERS];

  typedef struct {
    Pin step;
    Pin dir;
    Pin en;
    Pin min;
    Pin max;
  } StepPins;

  const StepPins stepPins[STEPPER_COUNT] = {
    AXIS_PIN_SET(X),
    AXIS_PIN_SET(Y),
    AXIS_PIN_SET(Z),
    AXIS_PIN_SET(A),
    AXIS_PIN_SET(B) };

  void init_pots() {
  }
  /*
  /// Set up the digipot pins 
  DigiPots digi_pots[STEPPER_COUNT] = {
    DigiPots(X_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
    DigiPots(Y_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
    DigiPots(Z_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
    DigiPots(A_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
    DigiPots(B_POT_PIN, eeprom_offsets::DIGI_POT_SETTINGS),
  };

  void initPots(){
  // set digi pots to stored default values
  for ( int i = 0; i < STEPPER_COUNT; i++ ) {
  digi_pots[i].init(i);
  }
  }
  */

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
      pins.dir.setValue(false);
      pins.dir.setDirection(true);
    }
  }

  void init() {
    init_pots();
    init_pins();
    reset_axes();
    // init pots
    
  }

  void enable(uint8_t which, bool enable) {
    stepPins[which].en.setValue(!enable);
  }    

};

/*

void abort() {
	//Stop the stepper subsystem and get the current position
	//after stopping
	quickStop();

  is_running = false;
  is_homing = false;
	
	stepperAxisInit(false);

	setSegmentAccelState(acceleration);
	deprimeEnable(true);
}

/// Define current position as given point
void definePosition(const Point& position_in) {
	Point position_offset = position_in;

	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		stepperAxis[i].hasDefinePosition = true;

	}

	plan_set_position(position_offset[X_AXIS], position_offset[Y_AXIS], position_offset[Z_AXIS], position_offset[A_AXIS], position_offset[B_AXIS]);
}


/// Define current position as given point
/// do not apply toolhead offsets
void defineHomePosition(const Point& position_in) {
	Point position_offset = position_in;

	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		stepperAxis[i].hasDefinePosition = true;
	}

	plan_set_position(position_offset[X_AXIS], position_offset[Y_AXIS], position_offset[Z_AXIS], position_offset[A_AXIS], position_offset[B_AXIS]);
}

/// Get the last position of the planner
/// This is also the target position of the last command that was sent with
/// setTarget / setTargetNew / setTargetNewExt
/// Note this isn't the position of the hardware right now, use getStepperPosition for that.
/// If the pipeline buffer is empty, then getPlannerPosition == getStepperPosition
const Point getPlannerPosition() {
	Point p;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		p = Point(planner_position[X_AXIS], planner_position[Y_AXIS], planner_position[Z_AXIS],
			  planner_position[A_AXIS], planner_position[B_AXIS] );

}


/// Get current position
const Point getStepperPosition() {
	int32_t position[STEPPER_COUNT];

	st_get_position(&position[X_AXIS], &position[Y_AXIS], &position[Z_AXIS], &position[A_AXIS], &position[B_AXIS]);

	Point p = Point(position[X_AXIS], position[Y_AXIS], position[Z_AXIS], position[A_AXIS], position[B_AXIS]);

	return p;
}



void setHoldZ(bool holdZ_in) {
	holdZ = holdZ_in;
}


void setTarget(const Point& target, int32_t dda_interval) {

	//Clip the Z axis so that it can't move outside the build area.
	//Addresses a specific issue with old start.gcode for the replicator.
	//It has a G1 Z155 command that was slamming the platform into the floor.  
	planner_target[Z_AXIS] = stepperAxis_clip_to_max(Z_AXIS, planner_target[Z_AXIS]);

	//Calculate the maximum steps of any axis and store in planner_master_steps
	//Also calculate the step deltas (planner_steps[i]) at the same time.
	int32_t max_delta = 0;
	planner_master_steps_index = 0;
	for (int i = 0; i < STEPPER_COUNT; i++) {
		planner_steps[i] = labs(planner_target[i] - planner_position[i]);

		if ( planner_steps[i] > max_delta) {
			planner_master_steps_index = i;
			max_delta = planner_steps[i];
		}
	}
	planner_master_steps = (uint32_t)max_delta;

	if ( planner_master_steps == 0 ) {
#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		//To keep in sync with the simulator
		current_move_index ++;
#endif
		return;
	}

	//dda_rate is the number of dda steps per second for the master axis
	uint32_t dda_rate = (uint32_t)(1000000 / dda_interval);

	plan_buffer_line(0, dda_rate, toolIndex, false, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize) is_running = true;
	else                                          is_running = false;
}


void setTargetNew(const Point& target, int32_t us, uint8_t relative) {
	//Add on the tool offsets and convert relative moves into absolute moves
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		if ((relative & (1 << i)) != 0) {
			planner_target[i] = planner_position[i] + target[i];
		}else{
		  planner_target[i] = target[i];
    }
	}

	//Clip the Z axis so that it can't move outside the build area.
	//Addresses a specific issue with old start.gcode for the replicator.
	//It has a G1 Z155 command that was slamming the platform into the floor.  
	planner_target[Z_AXIS] = stepperAxis_clip_to_max(Z_AXIS, planner_target[Z_AXIS]);

        //Calculate the maximum steps of any axis and store in planner_master_steps
        //Also calculate the step deltas (planner_steps[i]) at the same time.
        int32_t max_delta = 0;
	planner_master_steps_index = 0;
        for (int i = 0; i < STEPPER_COUNT; i++) {
                planner_steps[i] = labs(planner_target[i] - planner_position[i]);

                if ( planner_steps[i] > max_delta) {
			planner_master_steps_index = i;
                        max_delta = planner_steps[i];
		}
        }
        planner_master_steps = (uint32_t)max_delta;

	if ( planner_master_steps == 0 ) {
#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		//To keep in sync with the simulator
		current_move_index ++;
#endif
		return;
	}

	int32_t  dda_interval	= us / max_delta;

	//dda_rate is the number of dda steps per second for the master axis
	uint32_t dda_rate	= (uint32_t)(1000000 / dda_interval);

	plan_buffer_line(0, dda_rate, toolIndex, false, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize)      is_running = true;
	else                                               is_running = false;
}


//Dda_rate is the number of dda steps per second for the master axis

void setTargetNewExt(const Point& target, int32_t dda_rate, uint8_t relative, float distance, int16_t feedrateMult64) {
	//Add on the tool offsets and convert relative moves into absolute moves
	for ( uint8_t i = 0; i < STEPPER_COUNT; i ++ ) {
		if ((relative & (1 << i)) != 0) {
			planner_target[i] = planner_position[i] + target[i];
		}else{
      planner_target[i] = target[i] + (*tool_offsets)[i];
    }
	}

	//Clip the Z axis so that it can't move outside the build area.
	//Addresses a specific issue with old start.gcode for the replicator.
	//It has a G1 Z155 command that was slamming the platform into the floor.  
	planner_target[Z_AXIS] = stepperAxis_clip_to_max(Z_AXIS, planner_target[Z_AXIS]);

        //Calculate the maximum steps of any axis and store in planner_master_steps
        //Also calculate the step deltas (planner_steps[i]) at the same time.
        int32_t max_delta = 0;
        planner_master_steps_index = 0;
        for (int i = 0; i < STEPPER_COUNT; i++) {
                planner_steps[i] = planner_target[i] - planner_position[i];
		int32_t abs_planner_steps = labs(planner_steps[i]);
		if (abs_planner_steps <= 0x7fff)
		     delta_mm[i] = FPMULT2(ITOFP(planner_steps[i]), axis_steps_per_unit_inverse[i]);
		else
		      // This typically only happens for LONG Z axis moves
		      // As such it typically happens three times per print
		     delta_mm[i] = FTOFP((float)planner_steps[i] * FPTOF(axis_steps_per_unit_inverse[i]));
                planner_steps[i] = abs_planner_steps;

                if ( planner_steps[i] > max_delta) {
			planner_master_steps_index = i;
                        max_delta = planner_steps[i];
		}
        }
        planner_master_steps = (uint32_t)max_delta;

	if (( planner_master_steps == 0 ) || ( distance == 0.0 )) {
#ifdef DEBUG_BLOCK_BY_MOVE_INDEX
		//To keep in sync with the simulator
		current_move_index ++;
#endif
		return;
	}

	//Handle distance
	planner_distance = FTOFP(distance);

	//Handle feedrate
	FPTYPE feedrate = 0;

	if ( acceleration ) {
		feedrate = ITOFP((int32_t)feedrateMult64);

		//Feed rate was multiplied by 64 before it was sent, undo
#ifdef FIXED
			feedrate >>= 6;
#else
			feedrate /= 64.0;
#endif
	}

	plan_buffer_line(feedrate, dda_rate, toolIndex, acceleration && segmentAccelState, toolIndex);

	if ( movesplanned() >=  plannerMaxBufferSize)      is_running = true;
	else                                               is_running = false;
}


//Step positions for homing.  We shift by >> 1 so that we can add
//tool_offsets without overflow
#define POSITIVE_HOME_POSITION ((INT32_MAX - 1) >> 1)
#define NEGATIVE_HOME_POSITION ((INT32_MIN + 1) >> 1)

/// Start homing

void startHoming(const bool maximums, const uint8_t axes_enabled, const uint32_t us_per_step) {
	setSegmentAccelState(false);

	Point target = getStepperPosition();

    for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
      if ((axes_enabled & (1<<i)) == 0) {
			  axis_homing[i] = false;
		  } else {
        target[i] = (maximums) ? POSITIVE_HOME_POSITION : NEGATIVE_HOME_POSITION;
        axis_homing[i] = true;
        if(i == Z_AXIS) {z_homing = true;}
        stepperAxis[i].hasHomed = true;
      }
    }

	setTarget(target, us_per_step);

  is_homing = true;
}


/// Enable/disable the given axis.
void enableAxis(uint8_t index, bool enable) {
        if (index < STEPPER_COUNT) {
		stepperAxisSetEnabled(index, enable);
        }
}


/// Returns a bit mask for all axes enabled
uint8_t allAxesEnabled(void) {
	return axesEnabled;
}


/// set digital potentiometer for stepper axis
void setAxisPotValue(uint8_t index, uint8_t value){
	if (index < STEPPER_COUNT) {
		digi_pots[index].setPotValue(value);
	}
}


/// get the digital potentiometer for stepper axis
uint8_t getAxisPotValue(uint8_t index){
#ifndef SIMULATOR
	if (index < STEPPER_COUNT) {
		return digi_pots[index].getPotValue();
	}
#endif
	return 0;
}

/// Reset the digital potentiometer for stepper axis to the stored eeprom value
void resetAxisPot(uint8_t index) {
#ifndef SIMULATOR
	if (index < STEPPER_COUNT) {
		digi_pots[index].resetPots();
	}
#endif
}

/// Toggle segment acceleration on or off
/// Note this is also off if acceleration variable is not set
void setSegmentAccelState(bool state) {
     segmentAccelState = state;
}




// endstop status bits: (7-0) : | N/A | N/A | z max | z min | y max | y min | x max | x min |

uint8_t getEndstopStatus() {
	uint8_t status = 0;

	status |= stepperAxisIsAtMaximum(Z_AXIS) << 5;
	status |= stepperAxisIsAtMinimum(Z_AXIS) << 4;
	status |= stepperAxisIsAtMaximum(Y_AXIS) << 3;
	status |= stepperAxisIsAtMinimum(Y_AXIS) << 2;
	status |= stepperAxisIsAtMaximum(X_AXIS) << 1;
	status |= stepperAxisIsAtMinimum(X_AXIS) << 0;

	return status;
}


// Enables and disables deprime
// Deprime is always disabled if acceleration is switch off in the eeprom

void deprimeEnable(bool enable) {
	st_deprime_enable(acceleration && enable);
}


void runSteppersSlice() {
        //Set the stepper interrupt timer
#if defined(DEBUG_ONSCREEN) && defined(TIME_STEPPER_INTERRUPT)
        debug_onscreen2 = debugTimer;
#endif
}


void doStepperInterrupt() {

	//is_running is determined when a buffer item is added, however
	//if st_interrupt deletes a buffer item, then is_running must have changed
	//and now be false, so we set it here
	if ( st_interrupt() ) is_running = false;

	//If we're homing, there's a few possibilities:
	//1. The homing is still running on one of the axis
	//2. The homing on all axis has stopped running, in which case, the block
	//is dead, so we delete it and sync up the planner position to the stepper position
	//Homing blocks aren't automatically deleted by st_interrupt because they are set to
	//positions of INT32_MAX/MIN.
	if ( is_homing ) {
		is_homing = false;
	
		//Are we still homing on one of the axis?
		for (uint8_t i = 0; i <= Z_AXIS; i++)
			is_homing |= axis_homing[i];

		//If we've finished homing, stop the stepper subsystem
		//and sync
		if ( ! is_homing ) {
			//Delete all blocks (should only be 1 homing block) and sync
			//planner position to stepper position
			quickStop();
      if(z_homing) {
        z_homed++;
        z_homing = false;
      }

			setSegmentAccelState(acceleration);
		}
	}

#if defined(DEBUG_ONSCREEN) && defined(TIME_STEPPER_INTERRUPT)
        DEBUG_TIMER_FINISH;
        debugTimer = DEBUG_TIMER_TCTIMER_USI;
#endif
}


void doExtruderInterrupt() {
	st_extruder_interrupt();
}

uint8_t isZHomed(){
  return z_homed;
}


}
*/
