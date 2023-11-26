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
    param_flags = 0;
    cc = CommandCode();
    mode = SCAN_FOR_CMD;
    neg = false;
    frac_val = 0.1;
}

void Command::finish_fp_parse() {
    if (neg)
        cp() = -cp();
    neg = false;
    frac_val = 0.1;
}

bool check_for_command() {
    while (UART::available()) {
        uint8_t c = UART::read_byte();
        if (echo) {
            UART::write(c);
            if (c == '\r') UART::write('\n');
        }
        if (cmd().process_byte(c)) {
            return true;
        }
    }
    return false;
}

bool Command::process_byte(uint8_t c) {
    if (c == '\n' || c == '\r') {
        // End of command.
        finish_fp_parse(); // complete any in-process parsing
        return true;
    }
    if (c == ' ' || c == '\t') {
        return false; // spaces are always ignored in gcodes
    }
    if (c == ';') mode = SCAN_COMMENT;

    if (mode == SCAN_FOR_CMD) {
        if (c == 'G' || c == 'M') {
            cc.code = c;
            mode = SCAN_FOR_CMD_PARAM;
            cc.value = 0;
        } else
            mode = BAD_CMD;
        return false;
    }

    if (mode == SCAN_FOR_CMD_PARAM) {
        if (is_int(c)) {
            cc.value = (cc.value * 10) + (c - '0');
            return false;
        } else {
            mode = SCAN_FOR_CODE;
        }
    }
    if (mode == SCAN_FOR_CODE_FIRST) {
        mode = SCAN_FOR_CODE_INT;
        if (c == '-') {
            neg = true;
            return false;
        }
    }
    if (mode == SCAN_FOR_CODE_INT) {
        if (c == '.') {
            mode = SCAN_FOR_CODE_FRAC;
            return false;
        }
        if (is_int(c)) {
            cp() = (cp() * 10) + (c - '0');
            return false;
        }
        finish_fp_parse();
        mode = SCAN_FOR_CODE;
    }
    if (mode ==  SCAN_FOR_CODE_FRAC) {
        if (is_int(c)) {
            cp() += (c - '0') * frac_val;
            frac_val /= 10;
            return false;
        }
        finish_fp_parse();
        mode = SCAN_FOR_CODE;
    }
    if (mode == SCAN_FOR_CODE) {
        curParam = paramIdx(c);
        param_flags |= 1 << curParam;
        mode = (curParam == PARAM_LAST)
            ? BAD_CMD
            : SCAN_FOR_CODE_FIRST;
    }
    return false;
}
