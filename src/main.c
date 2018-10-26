#include <conio.h>
#include <stdio.h>
#include <spectrum.h>
#include <rs232.h>
#include <input.h>

void main(void)
{
  unsigned char inbyte, chkey, chl, chc;

  chl=0;
  chc=0;

  // quick initalise serial port
  rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
  rs232_init();

  // Clean up the screen
  zx_border(INK_BLACK);
  clg();
  zx_colour(PAPER_BLACK|INK_WHITE);


  // main loop
  printf("Terminal ready...");
  while(1)
  {
    if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)  //any incoming data capture and print
    {
      // filter input here


      chc = inbyte;

      if (chl==0xff && chc==0xff)  //catch Telnet IAC
      {
        //drop the character
        chl=0;  //reset last character chc will over write
      }
      else
      {
        // default output everything
        //putch(inbyte);
        printf("%c",inbyte);
        //fputc_cons(inbyte);
      }

      //  quick keyboard check if we are reading alot so we can interupt

      for (int i=0; i<1; i++)
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
      for (int i=0; i<20; i++)
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
