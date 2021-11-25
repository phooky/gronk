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

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include "Motherboard.hh"
#include "Configuration.hh"
//#include "Steppers.hh"
//#include "Command.hh"
//#include "Interface.hh"
#include "SoftI2cManager.hh"
#include "Piezo.hh"
#include "RGB_LED.hh"
#include <avr/eeprom.h>
#include <util/delay.h>

/// Instantiate static motherboard instance
Motherboard Motherboard::motherboard;

/// Create motherboard object
Motherboard::Motherboard() :
  lcd(LCD_STROBE, LCD_DATA, LCD_CLK)
{
}

 
#define ENABLE_TIMER_INTERRUPTS     TIMSK2 |= (1<<OCIE2A);	\
  TIMSK5 |= (1<<OCIE5A)
 
#define DISABLE_TIMER_INTERRUPTS    TIMSK2 &= ~(1<<OCIE2A);	\
  TIMSK5 &= ~(1<<OCIE5A)
 

void Motherboard::init(){
  SoftI2cManager::getI2cManager().init();

  micros = 0;
  initClocks();

  // Configure the debug pins.
  DEBUG_PIN.setDirection(true);
  DEBUG_PIN1.setDirection(true);
  DEBUG_PIN2.setDirection(true);
  DEBUG_PIN3.setDirection(true);	
  DEBUG_PIN4.setDirection(true);
  DEBUG_PIN5.setDirection(true);
  DEBUG_PIN6.setDirection(true);
		
}

void Motherboard::initClocks(){

  // set up piezo timer
  Piezo::shutdown_timer();
		
  // Reset and configure timer 5,  stepper
  // interrupt timer.
  TCCR5A = 0x00;
  TCCR5B = 0x0A; // no prescaling
  TCCR5C = 0x00;
  OCR5A = 0x2000; //INTERVAL_IN_MICROSECONDS * 16;
  TIMSK5 = 0x02; // turn on OCR5A match interrupt
	
  // Reset and configure timer 2, the microsecond timer, advance_timer and debug LED flasher timer.
  TCCR2A = 0x02; //CTC  //0x00;  
  TCCR2B = 0x04; //prescaler at 1/64  //0x0A; /// prescaler at 1/8
  OCR2A = 25; //Generate interrupts 16MHz / 64 / 25 = 10KHz  //INTERVAL_IN_MICROSECONDS;  // TODO: update PWM settings to make overflowtime adjustable if desired : currently interupting on overflow
  OCR2B = 0;
  TIMSK2 = 0x02; // turn on OCR2A match interrupt

#ifdef MODEL_REPLICATOR2
  // reset and configure timer 3, the Extruders timer
  // Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
  // Prescaler: 1/64 (250 KHz)
  TCCR3A = 0b00000001;  
  TCCR3B = 0b00001011; /// set to PWM mode
  OCR3A = 0;
  OCR3C = 0;
  TIMSK3 = 0b00000000; // no interrupts needed
#else
  // reset and configure timer 1, the Extruder Two PWM timer
  // Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
  // Prescaler: 1/64 (250 KHz)
  TCCR1A = 0b00000001;  
  TCCR1B = 0b00001011; /// set to PWM mode
  OCR1A = 0;
  OCR1B = 0;
  TIMSK1 = 0b00000000; // no interrupts needed
  
  // reset and configure timer 4, the Extruder One PWM timer
  // Mode: Fast PWM with TOP=0xFF (8bit) (WGM3:0 = 0101), cycle freq= 976 Hz
  // Prescaler: 1/64 (250 KHz)
  TCCR4A = 0b00000001;  
  TCCR4B = 0b00001011; /// set to PWM mode
  OCR4A = 0;
  OCR4B = 0;
  TIMSK4 = 0b00000000; // no interrupts needed
#endif
}

#define ONE_MINUTE 60000000
#define ONE_SECOND 1000000 

/// Reset the motherboard to its initial state.
/// This only resets the board, and does not send a reset
/// to any attached toolheads.
void Motherboard::reset(bool hard_reset) {
  indicateError(0); // turn on blinker

  // Init steppers
  //uint8_t axis_invert = eeprom::getEeprom8(eeprom_offsets::AXIS_INVERSION, 0);
  // Z holding indicates that when the Z axis is not in
  // motion, the machine should continue to power the stepper
  // coil to ensure that the Z stage does not shift.
  // Bit 7 of the AXIS_INVERSION eeprom setting
  // indicates whether or not to use z holding; 
  // the bit is active low. (0 means use z holding,
  // 1 means turn it off.)
  //bool hold_z = (axis_invert & (1<<7)) == 0;
  //steppers::setHoldZ(hold_z);

  // Initialize the host and  UARTs
  //UART::getHostUART().enable(true);
  //UART::getHostUART().in.reset();
	
  micros = 0;

  // get heater timeout from eeprom - the value is stored in minutes 
  //restart_timeout = (eeprom::getEeprom8(eeprom_offsets::HEATER_TIMEOUT_ON_CANCEL, 0) * ONE_MINUTE) + ONE_SECOND;

  // Interface LEDs

  INTERFACE_LED_ONE.setDirection(true);
  INTERFACE_LED_TWO.setDirection(true);
  
  INTERFACE_LED_ONE.setValue(true);
  INTERFACE_LED_TWO.setValue(true);

    if(hard_reset) {
      _delay_us(3000000);
    }

  // interface LEDs default to full ON
  interfaceBlink(25,15);
  
  // only call the piezo buzzer on full reboot start up
  // do not clear heater fail messages, the user should not be able to soft reboot from heater fail
  if(hard_reset)
    {
      RGB_LED::init();
		
      Piezo::playTune(TUNE_STARTUP);
		
    } 	
  



}



/// Get the number of microseconds that have passed since
/// the board was booted.
micros_t Motherboard::getCurrentMicros() {
  micros_t micros_snapshot;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    micros_snapshot = micros;
  }
  return micros_snapshot;
}

/// Run the motherboard interrupt
void Motherboard::doStepperInterrupt() {

  //TODO: we have pause implemented here - make sure things stil work
  //We never ignore interrupts on pause, because when paused, we might 
  //extrude filament to change it or fix jams

  DISABLE_TIMER_INTERRUPTS;
  sei();
  
  //steppers::doStepperInterrupt();
  
  cli();
  ENABLE_TIMER_INTERRUPTS;
 
#ifdef ANTI_CLUNK_PROTECTION
  //Because it's possible another stepper interrupt became due whilst
  //we were processing the last interrupt, and had stepper interrupts
  //disabled, we compare the counter to the requested interrupt time
  //to see if it overflowed.  If it did, then we reset the counter, and
  //schedule another interrupt for very shortly into the future.
  if ( TCNT5 >= OCR5A ) {

    OCR5A = 0x01;   //We set the next interrupt to 1 interval, because this will cause the interrupt to  fire again
    //on the next chance it has after exiting this interrupt, i.e. it gets queued.

    TCNT5 = 0;  //Reset the timer counter

		//debug_onscreen1 ++;
  }
#endif
}



// main motherboard loop
void Motherboard::runMotherboardSlice() {
	

}

//Frequency of Timer 2
//100 = (1.0 / ( 16MHz / 64 / 25 = 10KHz)) * 1000000
#define MICROS_INTERVAL 100

void Motherboard::UpdateMicros(){
  micros += MICROS_INTERVAL;//_IN_MICROSECONDS;

}

/// Timer three comparator match interrupt
ISR(TIMER5_COMPA_vect) {
  Motherboard::getBoard().doStepperInterrupt();
}



/// Number of times to blink the debug LED on each cycle
volatile uint8_t blink_count = 0;

/// number of cycles to hold on and off in each interface LED blink
uint8_t interface_on_time = 0;
uint8_t interface_off_time = 0;

/// The current state of the debug LED
enum {
  BLINK_NONE,
  BLINK_ON,
  BLINK_OFF,
  BLINK_PAUSE
};

/// state trackers for blinking LEDS
int blink_state = BLINK_NONE;
int interface_blink_state = BLINK_NONE;

/// Write an error code to the debug pin.
void Motherboard::indicateError(int error_code) {
  if (error_code == 0) {
    blink_state = BLINK_NONE;
    DEBUG_PIN.setValue(false);
  }
  else if (blink_count != error_code) {
    blink_state = BLINK_OFF;
  }
  blink_count = error_code;
}

// set on / off period for blinking interface LEDs
// if both times are zero, LEDs are full on, if just on-time is zero, LEDs are full OFF
void Motherboard::interfaceBlink(int on_time, int off_time){
	
  if(off_time == 0){
    interface_blink_state = BLINK_NONE;
    INTERFACE_LED_ONE.setValue(true);
    INTERFACE_LED_TWO.setValue(true);
  }else if(on_time == 0){
    interface_blink_state = BLINK_NONE;
    INTERFACE_LED_ONE.setValue(false);
    INTERFACE_LED_TWO.setValue(false);
  } else{
    interface_on_time = on_time;
    interface_off_time = off_time;
    interface_blink_state = BLINK_ON;
  }

}

/// Get the current error code.
uint8_t Motherboard::getCurrentError() {
  return blink_count;
}

/// Timer2 overflow cycles that the LED remains on while blinking
#define OVFS_ON 18
/// Timer2 overflow cycles that the LED remains off while blinking
#define OVFS_OFF 18
/// Timer2 overflow cycles between flash cycles
#define OVFS_PAUSE 80

/// Number of overflows remaining on the current blink cycle
int blink_ovfs_remaining = 0;
/// Number of blinks performed in the current cycle
int blinked_so_far = 0;
/// Number of overflows remaining on the current overflow blink cycle
int interface_ovfs_remaining = 0;

uint8_t blink_overflow_counter = 0;

volatile micros_t m2;

/// Timer 2 overflow interrupt
ISR(TIMER2_COMPA_vect) {
	
  Motherboard::getBoard().UpdateMicros();
	
  if(blink_overflow_counter++ <= 0xA4)
    return;
	
  blink_overflow_counter = 0;
			
  /// Debug LEDS on Motherboard
  if (blink_ovfs_remaining > 0) {
    blink_ovfs_remaining--;
  } else {
    if (blink_state == BLINK_ON) {
      blinked_so_far++;
      blink_state = BLINK_OFF;
      blink_ovfs_remaining = OVFS_OFF;
      DEBUG_PIN.setValue(false);
    } else if (blink_state == BLINK_OFF) {
      if (blinked_so_far >= blink_count) {
	blink_state = BLINK_PAUSE;
	blink_ovfs_remaining = OVFS_PAUSE;
      } else {
	blink_state = BLINK_ON;
	blink_ovfs_remaining = OVFS_ON;
	DEBUG_PIN.setValue(true);
      }
    } else if (blink_state == BLINK_PAUSE) {
      blinked_so_far = 0;
      blink_state = BLINK_ON;
      blink_ovfs_remaining = OVFS_ON;
      DEBUG_PIN.setValue(true);
    }
  }
  /// Interface Board LEDs
  if( interface_ovfs_remaining > 0){
    interface_ovfs_remaining--;
  } else {
    if (interface_blink_state == BLINK_ON) {
      interface_blink_state = BLINK_OFF;
      interface_ovfs_remaining = interface_on_time;
      INTERFACE_LED_ONE.setValue(true);
      INTERFACE_LED_TWO.setValue(true);
    } else if (interface_blink_state == BLINK_OFF) {
      interface_blink_state = BLINK_ON;
      interface_ovfs_remaining = interface_off_time;
      INTERFACE_LED_ONE.setValue(false);
      INTERFACE_LED_TWO.setValue(false);
    } 
  } 
}



