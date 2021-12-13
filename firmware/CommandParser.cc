#include "CommandParser.hh"
#include "UART.hh"

Command command;

Command& cmd() { return command; }

uint8_t paramIdx(char code) {
  switch (code) {
  case 'X': return X;
  case 'Y': return Y;
  case 'Z': return Z;
  case 'F': return F;
  case 'S': return S;
  default: return PARAM_LAST;
  }
}

bool is_int(char c) { return (c >= '0') && (c <= '9'); }

void reset_command() {
  for (int i = 0; i < PARAM_LAST; i++) {
    command.params[i].reset();
    command.mode = SCAN_FOR_CMD;
  }
}
  
bool check_for_command() {
  while (UART::available()) {
    uint8_t c = UART::read_byte();
    if (c == '\n') { // command complete
      return true;
    }
    switch (command.mode) {
    case SCAN_FOR_CMD:
      if (c == 'G' || c == 'M') {
	command.cmdCode = c; command.mode = SCAN_FOR_CMD_PARAM;
      } else command.mode = BAD_CMD;
      break;
    case SCAN_FOR_CMD_PARAM:
      if (c == ' ') command.mode = SCAN_FOR_CODE;
      else if (is_int(c)) command.cmdValue = (command.cmdValue*10) + (c - '0');
      else command.mode = BAD_CMD;
      break;
    case SCAN_FOR_CODE:
      if (c == ' ') break;
      command.curParam = paramIdx(c);
      command.mode = (command.curParam == PARAM_LAST)?BAD_CMD:SCAN_FOR_CODE_INT;
      break;
    case SCAN_FOR_CODE_INT:
      if (c == '.') command.mode = SCAN_FOR_CODE_FRAC;
      else if (c == ' ') command.mode = SCAN_FOR_CODE;
      else if (is_int(c)) command.cp().integer = (command.cp().integer * 10) + (c - '0');
      else command.mode = BAD_CMD;
      break;
    case SCAN_FOR_CODE_FRAC:
    default:
      break;
    }
  }
  return false;
}
								
      
	
	  
      
    
