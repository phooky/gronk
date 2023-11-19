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

#include "UART.hh"
#include "CommandParser.hh"
#include <string>
#include <iostream>

using std::string;
namespace UART {

string test_data;
string::const_iterator test_data_iter;

void set_test_string(string data) {
    test_data = data;
    test_data_iter = test_data.begin();
}

// Stubs probably unnecessary
void initialize() {}
bool write(uint8_t byte) { return true; }
int write_buffer(uint8_t *buf, uint8_t length) { return length; }
int write_string(const char *str, const bool nl) { return 0; }
/// Return the amount of data available on the uart
int available() { return test_data.end() - test_data_iter; }

/// Returns the number of bytes read
int read(uint8_t *buf, uint8_t n) {
    if (available() < n) {
        n = available();
    }
    int nc = n;
    while (n) {
        *(buf++) = *(test_data_iter++);
        n--;
    }
    return nc;
}

uint8_t read_byte() { return *(test_data_iter++); }

void enable(bool enabled) {}

} // namespace UART

int main() {
    UART::set_test_string("G00 X10 P5 Y10 F20\n\r");
    if (check_for_command()) {
        std::cout << "x "<< cmd().params[X] << " p " << cmd().params[P] << " code "<<cmd().cmdCode;
        std::cout << cmd().cmdValue << " ok" << std::endl;
    }
}
