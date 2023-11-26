#include <stdint.h>

typedef enum {
    SCAN_FOR_CMD,
    SCAN_FOR_CMD_PARAM,
    SCAN_FOR_CODE,
    SCAN_FOR_CODE_FIRST,
    SCAN_FOR_CODE_INT,
    SCAN_FOR_CODE_FRAC,
    SCAN_COMMENT,
    BAD_CMD,
    CMD_READY,
} ScanMode;

enum { X = 0, Y = 1, Z = 2, F = 3, S = 4, P = 5, PARAM_LAST };

class Command {
public:
    float params[PARAM_LAST];
    char cmdCode;
    uint8_t cmdValue;
    uint8_t curParam;
    ScanMode mode;
    // FP state
    float frac_val;
    bool neg;
    // convenience fns
    float &cp() { return params[curParam]; }
    void reset();
};

bool check_for_command();
Command &cmd();
void set_echo(bool);
