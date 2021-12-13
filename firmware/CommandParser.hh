#include <stdint.h>

typedef enum {
  SCAN_FOR_CMD,
  SCAN_FOR_CMD_PARAM,
  SCAN_FOR_CODE,
  SCAN_FOR_CODE_INT,
  SCAN_FOR_CODE_FRAC,
  BAD_CMD,
  CMD_READY,
} ScanMode;

class Fixed32 {
public:
  uint16_t integer;
  uint16_t fraction;
  void reset() { integer = fraction = 0; }
};

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
  // convenience fns
  Fixed32& cp() { return params[curParam]; }
} Command;

bool check_for_command();
Command& cmd();
