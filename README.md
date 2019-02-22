# snapCterm
ZX Spectrum Terminal

Basics of an ANSI 80 column serial terminal for the plus machines (not IF1 could be compiler for IF1)

ASCII characters (OEM extended), ANSI ESC codes & COLOUR.  Includes classic spectrum colour clash.

This project is not complete and the source code is a litter of comments!  Work is still on going, the basic features are working but there is still more to do.  Some of the Esc codes need to be caught and handled as the library used doesn't support them.  The spectrum keyboard is missing most of the Keys found on modern keyboards.  A mechanism to handle this needs to be finished, so ew have CRTL/page up/page down etc.

Could possibly do with a menu to wrap the terminal in.  So that settings can be changed or dialing can be done without knowing the AT codes!  May be a help system?
