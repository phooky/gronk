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

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "Pin.hh"
#include "Configuration.hh"
#include "SoftI2cManager.hh"
#include "RGB_LED.hh"
#include "LiquidCrystalSerial.hh"
#include "ButtonArray.hh"

void reset(bool hard_reset) {
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    uint8_t resetFlags = MCUSR & 0x0f;
    // check for brown out reset flag and report if true
		
    // clear watch dog timer and re-enable
    if(hard_reset)
      { 
	// ATODO: remove disable
	wdt_disable();
	MCUSR = 0x0;
	wdt_enable(WDTO_8S); // 8 seconds is max timeout
      }
  }
}

const unsigned int JS_CENTER_VAL = 509;
const unsigned int JS_LOW_VAL = 405;
const unsigned int JS_HIGH_VAL = 615;
const unsigned int JS_DEAD_ZONE = 10;
const unsigned int JS_MAX_ZONE = 10;
unsigned int js_positions[2] = { 0xffff, 0xffff };
unsigned char js_chan = 0; // current channel being read

void init_analog() {
  // Clear MUX5 in ADCSRB.
  ADCSRB &= ~(1<<MUX5);
  // Set prescaler bits ADPS in ADCSRA.
  // Try to get 200KHz. 16M/200K = 80x. 128x works.
  // ADCSRA: ADPS2-0 = 111b
  // ADCSRA: ADEN = 1b
  ADCSRA = (1<<ADEN) | (1<<ADPS2 ) | (1<<ADPS1) | (1<<ADPS0);
}

void start_analog_conversion(unsigned char channel) {
  // Write channel to ADMUX.
  // ADMUX: REFS1/0 = b01 (AVCC VREF)
  // ADMUX: ADLAR = 0 (ADCH/L are right-justified)
  // ADMUX: MUX4-0 = ADCX channel (ADC0-ADC7)
  ADMUX = (1 << REFS0) | channel;
  // ADSC is set to start conversion. This bit is cleared when the
  // conversion is done.
  // ADIE is set to trigger completion interrupt.
  ADCSRA |= 1 << ADIE | 1 << ADSC;
}

// PF0-2
unsigned int read_analog(unsigned char channel) {
  // Write channel to ADMUX.
  // ADMUX: REFS1/0 = b01 (AVCC VREF)
  // ADMUX: ADLAR = 0 (ADCH/L are right-justified)
  // ADMUX: MUX4-0 = ADCX channel (ADC0-ADC7)
  ADMUX = (1 << REFS0) | channel;
  // ADSC is set to start conversion. This bit is cleared when the
  // conversion is done.
  ADCSRA |= 1 << ADSC;
  while ( ADCSRA & (1<<ADSC)) {
  }
  // When conversion completes, an interrupt can be triggered. (ADIF)
  // Result is in ADCH and ADCL. ADCL *must* be read first (if more than
  // 8 bits are needed.
  unsigned int result = ADCL;
  result |= (ADCH & 0x03) << 8;
  return result;
}

void init_timers() {
  // Reset and configure timer 2, the microsecond timer, advance_timer and debug LED flasher timer.
  TCCR2A = 0x02; //CTC  //0x00;  
  TCCR2B = 0x04; //prescaler at 1/64  //0x0A; /// prescaler at 1/8
  OCR2A = 25; //Generate interrupts 16MHz / 64 / 25 = 10KHz  //INTERVAL_IN_MICROSECONDS;
  // TODO: update PWM settings to make overflowtime adjustable if desired : currently interupting on overflow
  OCR2B = 0;
  TIMSK2 |= 0x02; // turn on OCR2A match interrupt
}

int main() {
  INTERFACE_POWER.setDirection(true);
  INTERFACE_POWER.setValue(false);

  INTERFACE_LED_ONE.setDirection(true);
  INTERFACE_LED_TWO.setDirection(true);
  INTERFACE_LED_ONE.setValue(true);
  INTERFACE_LED_TWO.setValue(true);

  reset(true);
  SoftI2cManager::getI2cManager().init();
  init_timers();
  init_analog();
  start_analog_conversion(js_chan);
  sei();
  RGB_LED::init();
  RGB_LED::setCustomColor(0x10,0x88,0x10);

  ButtonArray::init();
  
  LiquidCrystalSerial lcd(LCD_STROBE, LCD_DATA, LCD_CLK);
  lcd.begin(LCD_SCREEN_WIDTH,LCD_SCREEN_HEIGHT);
  lcd.clear(); lcd.home();
  lcd.setCursor(0,0);
  if (ButtonArray::pressed() & LEFT) {
    lcd.writeString("LEFT on start.");
  } else {
    lcd.writeString("Hello world.");
  }

  while (1) {
    wdt_reset();
    _delay_ms(200);
    lcd.clear();
    lcd.writeString("Port A:");
    lcd.writeInt(js_positions[0]);
    lcd.setCursor(0,1);
    lcd.writeString("Port B:");
    lcd.writeInt(js_positions[1]);
  }
  return 0;
}

ISR(TIMER2_COMPA_vect) {
}

ISR(ADC_vect) {
  unsigned int result = ADCL;
  result |= (ADCH & 0x03) << 8;
  js_positions[js_chan] = result;
  js_chan ^= 0x01;
  start_analog_conversion(js_chan);
}
