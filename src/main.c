#include <conio.h>
#include <stdio.h>
#include <spectrum.h>
#include <rs232.h>
#include <input.h>

void main(void)
{
  unsigned char inbyte, chkey;

  //quick initalise serial port

  rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
  rs232_init();

  //main loop
  printf("Terminal ready...");

  while(1)
  {
    //delay to slow the loop giving the key press time

    //in_Pause(20);

    //read keyboard and send
    for (int i=0; i<20; i++)
    {
      chkey = getk();
      if (chkey != NULL)
      {
        rs232_put(chkey);
      }
    }
    //chkey = NULL;  //clear the key buffer?

    //read in any data at the port

    if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)
    {
      // filter input here


      // default output everything
      //putch(inbyte);
      //printf("%c",inbyte);
      fputc_cons(inbyte);
    }
    else
    {
      //in_Wait(1);
      in_Pause(10);  //slow the code when no data incoming
    }



  }

}
