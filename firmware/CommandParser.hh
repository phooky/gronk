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
} ParseState;

enum { X = 0, Y = 1, Z = 2, F = 3, S = 4, P = 5, PARAM_LAST };
uint8_t paramIdx(char code);

class CommandCode {
public:
    char code;
    uint8_t value;
    CommandCode() : code(0), value(0) {}
};

class Command {
private:
    float params[PARAM_LAST];
    CommandCode cc;
    uint8_t curParam;
    uint8_t param_flags;
    ParseState mode;
    // FP state
    float frac_val;
    bool neg;
    // convenience fns
    float &cp() { return params[curParam]; }
    void finish_fp_parse();
public:
    bool has_param(uint8_t which) { return (param_flags & (1 << which)) != 0; }
    const float& operator[] (uint8_t which) { return params[which]; }
    const CommandCode& code() { return cc; }
    bool process_byte(uint8_t c);
    bool is_ok() { return mode != BAD_CMD; }
    void reset();
};

bool check_for_command();
Command &cmd();
void set_echo(bool);
