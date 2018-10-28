// Compiler line (not using make)
// zcc +zx -clib=ansi -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav main.c

#include <conio.h>
#include <stdio.h>
#include <spectrum.h>
#include <rs232.h>
#include <input.h>

void main(void)
{
  unsigned char inbyte, chkey, lastbyte, chl, chc;
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
