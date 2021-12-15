///
/// GCode interpretation
///

/// We're handling a very limited subset of gcodes.
//
/// All distances are in mm. G20/21 are ignored.
/// We're always in absolute positioning mode. G90/91 are ignored.

/// ----- IMPLEMENTED -----
/// M17 [X] [Y] [Z]                 -- enable steppers
/// M18 [X] [Y] [Z]                 -- disable steppers
/// -------- TO DO --------
/// G0  [Xval] [Yval] [Zval] [Fval] -- move
/// G1  [Xval] [Yval] [Zval] [Fval] -- move
/// G4  [Sval]                      -- dwell (in seconds)
/// G28 [X] [Y] [Z]                 -- home
/// G92 [Xval] [Yval] [Zval]        -- set current position

