// Compiler line (not using make)
// zcc +zx -clib=ansi -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav main.c
//zcc +zx  -clib=ansi -pragma-redirect:ansifont=_oemascii -pragma-define:ansifont_is_packed=0 -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav

#include <conio.h>
#include <stdio.h>
#include <spectrum.h>
#include <rs232.h>
#include <input.h>

//Font stuff
#asm  
SECTION data_user ;

PUBLIC _oemascii
_oemascii:
    BINARY "./src/oemascii.bin" ;  //  This BINARY holds the first 127 characters
    BINARY "./src/oemasciiext.bin" ;  // This BINARY holds the last 127 (128 - 255) characters, it gets parked behind the first set
#endasm

void scrollfix(uint_fast8_t col)
{  // Ivan Blajer trick, start address is the start of the last row
#asm
    ld hl,2 // First address on stack is return address
    add hl,sp // hence move two bytes below HL now points to where 
    ld a,(hl) // our passed variable col sits now put the value into A
    ld hl,23264 // this just fills the attribute area with the value of A
    ld de,23265 // display is 2 bytes!!!
    ld bc,32    //number of columns to colour
    ld (hl),a
    ldir
#endasm
}

void main(void)
{
  unsigned char inbyte, chkey, lastbyte;
  lastbyte=0;

  // quick initalise serial port
  rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
  rs232_init();

  // Clean up the screen
  zx_border(INK_BLACK);
  clg();
  zx_colour(PAPER_WHITE|INK_BLACK);

  // main loop
  printf("Terminal ready...");
  while(1)
  {
    if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)  //any incoming data capture and print
    {
      // filter input here

      if (lastbyte==0xff && inbyte==0xff)  //catch Telnet IAC drop it
      {
        inbyte=0;  //reset inbyte
      }
      else if (inbyte==0x0d)  // Catch Carriage Return
      {  // could be left out as 0x0d triggers a new line if printed to screen
        printf("\n");
        inbyte=0;  //reset inbyte
      }
      else if (inbyte==0x0a)  // Catch Line Feed
      {
        inbyte=0;  //reset inbyte
      }
      else  // default output the character to the screen
      {
        fputc_cons(inbyte);
      }

      lastbyte=inbyte;

      //  quick keyboard check if we are reading alot so we can interupt

      for (int i=5; i>0; i--)
      {
        chkey = getk();
        if (chkey != NULL)
        {
          rs232_put(chkey);
        }
      }

    }
    else //no incoming data check keyboard
    {
      for (int i=20; i>0; i--)
      {
        chkey = getk();
        if (chkey != NULL)
        {
          rs232_put(chkey);
        }
      }
    }

  }

}
