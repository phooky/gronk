

Table of supported G codes

| G-code | Meaning                      |
|--------|------------------------------|
| G0     | Ordinary movement            |
| G1     | Fast travel                  |
| G92    | Set current position as zero |

Table of supported M codes

| M-code          | Meaning                                          |
|-----------------|--------------------------------------------------|
| M03             | Plotter pen down                                 |
| M04             | Plotter pen up                                   |
| M17 [X] [Y] [Z] | Enable steppers. If none specified, enable all.  |
| M18 [X] [Y] [Z] | Disable steppers. If none specified, enable all. |
| M230            | Turn on command echoing.                         |
| M231            | Turn off command echoing.                        |






/// ----- IMPLEMENTED -----
/// M17 [X] [Y] [Z]                 -- enable steppers
/// M18 [X] [Y] [Z]                 -- disable steppers
/// -------- TO DO --------
/// G0  [Xval] [Yval] [Zval] [Fval] -- move
/// G1  [Xval] [Yval] [Zval] [Fval] -- move
/// G4  [Sval]                      -- dwell (in seconds)
/// G28 [X] [Y] [Z]                 -- home
/// G92 [Xval] [Yval] [Zval]        -- set current position
