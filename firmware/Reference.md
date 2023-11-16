

Table of supported G codes

| G-code | Meaning                      | Implemented |
|--------|------------------------------|-------------|
| G0     | Ordinary movement            | X           |
| G1     | Fast travel                  | X           |
| G92    | Set current position as zero | X           |
|        |                              |             |

Table of supported M codes

| M-code          | Meaning                                          | Implemented |
|-----------------|--------------------------------------------------|-------------|
| M03             | Plotter pen down                                 |             |
| M04             | Plotter pen up                                   |             |
| M17 [X] [Y] [Z] | Enable steppers. If none specified, enable all.  | X           |
| M18 [X] [Y] [Z] | Disable steppers. If none specified, enable all. | X           |
| M230            | Turn on command echoing.                         | X           |
| M231            | Turn off command echoing.                        | X           |
| M255            | Reset machine.                                   |             |






/// ----- IMPLEMENTED -----
/// M17 [X] [Y] [Z]                 -- enable steppers
/// M18 [X] [Y] [Z]                 -- disable steppers
/// -------- TO DO --------
/// G0  [Xval] [Yval] [Zval] [Fval] -- move
/// G1  [Xval] [Yval] [Zval] [Fval] -- move
/// G4  [Sval]                      -- dwell (in seconds)
/// G28 [X] [Y] [Z]                 -- home
/// G92 [Xval] [Yval] [Zval]        -- set current position
