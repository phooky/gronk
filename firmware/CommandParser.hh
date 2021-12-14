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

// 16.16
class Fixed32 {
public:
  union {
    struct {
      uint16_t frac_p;
      int16_t int_p;
    } v16;
    int32_t v32;
  } v;

  void reset() { v.v32 = 0; }
  void negate() { v.v32 = -v.v32; }

  int16_t& int_part() { return v.v16.int_p; }
  uint16_t& frac_part() { return v.v16.frac_p; }
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
  // FP state
  uint16_t frac_val;
  bool neg;
  // convenience fns
  Fixed32& cp() { return params[curParam]; }
} Command;

bool check_for_command();
Command& cmd();
