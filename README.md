# snapCterm
ZX Spectrum Terminal

Basics of an ANSI 80 column serial terminal for the plus machines (not IF1 could be compiler for IF1)

ASCII characters (OEM extended), ANSI ESC codes & COLOUR.  Includes classic spectrum colour clash.

I consider this a beta version, mainly because it doesn't have any file transfer support.  However, I think I have all the ESC codes covered now.  There is a nice colour correction mode, as we don't have two shades of black on the spectrum.

Another limitation of this terminal is it ONLY support ASCII NOT UTF8.  While you can telnet into a linux machine, some applications such as NANO cause problems.

**Builds with Z88DK 2.1 (07/02/2021)**

Future work:

File transfer support, for a mix of storage systems.
Fix the problems when connected to Linux using some console application like nano.
