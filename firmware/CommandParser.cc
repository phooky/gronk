#include "CommandParser.hh"
#include "UART.hh"

Command command;

Command &cmd() { return command; }

bool echo = false;

void set_echo(bool on) { echo = on; }

uint8_t paramIdx(char code) {
    switch (code) {
    case 'X':
        return X;
    case 'Y':
        return Y;
    case 'Z':
        return Z;
    case 'F':
        return F;
    case 'S':
        return S;
    case 'P':
        return P;
    default:
        return PARAM_LAST;
    }
}

bool is_int(char c) { return (c >= '0') && (c <= '9'); }

void Command::reset() {
    for (int i = 0; i < PARAM_LAST; i++) {
        params[i] = 0;
    }
    cmdCode = 0;
    cmdValue = 0;
    mode = SCAN_FOR_CMD;
    neg = false;
    frac_val = 0.1;
}

void finish_fp_parse() {
    if (command.neg)
        command.cp() = -command.cp();
    command.neg = false;
    command.frac_val = 0.1;
}

bool process_byte(uint8_t c);

bool check_for_command() {
    while (UART::available()) {
        uint8_t c = UART::read_byte();
        if (echo) {
            UART::write(c);
            if (c == '\r') UART::write('\n');
        }
        if (process_byte(c)) {
            return true;
        }
    }
    return false;
}

bool process_byte(uint8_t c) {
    if (c == '\n' || c == '\r') {
        // End of command.
        finish_fp_parse(); // complete any in-process parsing
        return true;
    }
    if (c == ' ' || c == '\t') {
        return false; // spaces are always ignored in gcodes
    }
    if (c == ';') command.mode = SCAN_COMMENT;

    if (command.mode == SCAN_FOR_CMD) {
        if (c == 'G' || c == 'M') {
            command.cmdCode = c;
            command.mode = SCAN_FOR_CMD_PARAM;
            command.cmdValue = 0;
        } else
            command.mode = BAD_CMD;
        return false;
    }

    if (command.mode == SCAN_FOR_CMD_PARAM) {
        if (is_int(c)) {
            command.cmdValue = (command.cmdValue * 10) + (c - '0');
            return false;
        } else {
            command.mode = SCAN_FOR_CODE;
        }
    }
    if (command.mode == SCAN_FOR_CODE_FIRST) {
        command.mode = SCAN_FOR_CODE_INT;
        if (c == '-') {
            command.neg = true;
            return false;
        }
    }
    if (command.mode == SCAN_FOR_CODE_INT) {
        if (c == '.') {
            command.mode = SCAN_FOR_CODE_FRAC;
            return false;
        }
        if (is_int(c)) {
            command.cp() = (command.cp() * 10) + (c - '0');
            return false;
        }
        finish_fp_parse();
        command.mode = SCAN_FOR_CODE;
    }
    if (command.mode ==  SCAN_FOR_CODE_FRAC) {
        if (is_int(c)) {
            command.cp() += (c - '0') * command.frac_val;
            command.frac_val /= 10;
            return false;
        }
        finish_fp_parse();
        command.mode = SCAN_FOR_CODE;
    }
    if (command.mode == SCAN_FOR_CODE) {
        command.curParam = paramIdx(c);
        command.mode = (command.curParam == PARAM_LAST)
            ? BAD_CMD
            : SCAN_FOR_CODE_FIRST;
    }
    return false;
}
