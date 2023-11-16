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
    default:
        return PARAM_LAST;
    }
}

bool is_int(char c) { return (c >= '0') && (c <= '9'); }

void reset_command() {
    for (int i = 0; i < PARAM_LAST; i++) {
        command.params[i] = 0;
        command.mode = SCAN_FOR_CMD;
    }
    command.neg = false;
    command.frac_val = 0.1;
}

void finish_fp_parse() {
    if (command.neg)
        command.cp() = -command.cp();
    command.neg = false;
    command.frac_val = 0.1;
}

bool check_for_command() {
    while (UART::available()) {
        uint8_t c = UART::read_byte();
        if (echo) {
            UART::write(c);
            if (c == '\r') UART::write('\n');
        }
        bool end_of_command = (c == '\n' || c == '\r');
        // treat a command completion as if it had a trailing space
        // (this will complete any parameter scans)
        if (end_of_command)
            c = ' ';
        switch (command.mode) {
        case SCAN_FOR_CMD:
            if (c == 'G' || c == 'M') {
                command.cmdCode = c;
                command.mode = SCAN_FOR_CMD_PARAM;
                command.cmdValue = 0;
            } else
                command.mode = BAD_CMD;
            break;
        case SCAN_FOR_CMD_PARAM:
            if (c == ' ')
                command.mode = SCAN_FOR_CODE;
            else if (is_int(c))
                command.cmdValue = (command.cmdValue * 10) + (c - '0');
            else
                command.mode = BAD_CMD;
            break;
        case SCAN_FOR_CODE:
            if (c == ' ')
                break;
            command.curParam = paramIdx(c);
            command.mode = (command.curParam == PARAM_LAST)
                               ? BAD_CMD
                               : SCAN_FOR_CODE_FIRST;
            break;
        case SCAN_FOR_CODE_FIRST:
            if (c == '-')
                command.neg = true;
            else if (c == ' ')
                command.cp() += 1; // It's a flag, treat like a bool
            command.mode = SCAN_FOR_CODE_INT;
        case SCAN_FOR_CODE_INT:
            if (c == '.')
                command.mode = SCAN_FOR_CODE_FRAC;
            else if (c == ' ')
                command.mode = SCAN_FOR_CODE;
            else if (is_int(c))
                command.cp() = (command.cp() * 10) + (c - '0');
            else
                command.mode = BAD_CMD;
            break;
        case SCAN_FOR_CODE_FRAC:
            if (c == ' ') {
                finish_fp_parse();
                command.mode = SCAN_FOR_CODE;
            } else if (is_int(c)) {
                command.cp() += (c - '0') * command.frac_val;
                command.frac_val /= 10;
            } else
                command.mode = BAD_CMD;
        default:
            break;
        }
        if (end_of_command)
            return true;
    }
    return false;
}
