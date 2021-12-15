#include <stdint.h>
#include "Fixed32.hh"

typedef enum {
  SCAN_FOR_CMD,
  SCAN_FOR_CMD_PARAM,
  SCAN_FOR_CODE,
  SCAN_FOR_CODE_FIRST,
  SCAN_FOR_CODE_INT,
  SCAN_FOR_CODE_FRAC,
  BAD_CMD,
  CMD_READY,
} ScanMode;

enum {
  X = 0,
  Y = 1,
  Z = 2,
  F = 3,
  S = 4,
  PARAM_LAST
};

typedef struct {
  Fixed32 params[PARAM_LAST];
  char cmdCode;
  uint8_t cmdValue;
  uint8_t curParam;
  ScanMode mode;
  // FP state
  uint16_t frac_val;
  bool neg;
  // convenience fns
  Fixed32& cp() { return params[curParam]; }
} Command;

bool check_for_command();
void reset_command();
Command& cmd();
