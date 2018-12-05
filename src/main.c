// Compiler line (not using make)
// zcc +zx -clib=ansi -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav main.c
//zcc +zx  -clib=ansi -pragma-redirect:ansifont=_oemascii -pragma-define:ansifont_is_packed=0 -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav

#include <conio.h>
#include <stdio.h>
#include <spectrum.h>
#include <rs232.h>
#include <input.h>
#include <string.h>


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

void newline_attr()
{//Allen Albright's method
    unsigned char row_attr;
    unsigned char *attr;

    row_attr = 23;
    attr = zx_cyx2aaddr(23,31);

    do
    {
        if (7 != *attr)
        memset(attr - 31, 7, 32);

        attr = zx_aaddrcup(attr);
    }
    while (row_attr-- != 0);

}

unsigned char inbyte, chkey, lastbyte;
static uint8_t bytes=10;
char rxdata[10];

void main(void)
{
  
  lastbyte=0;

  // quick initalise serial port
  rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
  rs232_init();

  // Clean up the screen
  zx_border(INK_BLACK);
  clg();
  zx_colour(PAPER_BLACK|INK_WHITE);
  
  //ANSI ESCAPE codes TO SET UP
  cprintf("\033[37;40m");  // esc [ ESC SEQUENCE (Foreground)White;(Background)Black m (to terminate)

  // main loop
  printf("Terminal ready...");
  while(1)
  {
    if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)  //any incoming data capture and print
    {
      zx_border(INK_RED);
      rxdata[0]=inbyte;         //Buffer the first character
      bytes = 10;               //Maximum number of bytes to read in.

      for (uint8_t i=1;i<bytes;i++)   //Loop to buffer in up to 10 characters
      {
        if (rs232_get(&inbyte) != RS_ERR_NO_DATA)  //If character add it to the buffer
        {
          rxdata[i]=inbyte;
        }
        else  //Else no character record the number of bytes we have collected
        {
          bytes = i;
          i = 254; //kill the for loop
        }

      }      

      for (uint8_t i=0;i<bytes;i++)   //Loop to output the buffer
      {
        inbyte = rxdata[i];

        // filter input here

        if (lastbyte==0xff && inbyte==0xff)  //catch Telnet IAC drop it
        //if(inbyte==0xff)
        {
          //fputc_cons(inbyte);
          inbyte=0;  //reset inbyte
        }
        else if (inbyte == 0x09) // TAB
        {
          //fputc_cons(0x07);  // BEEP
          fputc_cons(inbyte);

        }        
        else if (inbyte == 0x0c) // Clear screen and home cursor
        {
          //fputc_cons(0x07);  // BEEP
          fputc_cons(inbyte);
        }
        else if (inbyte==0x0d)  // Carriage Return
        {  // could be left out as 0x0d triggers a new line if printed to screen
          
          fputc_cons(inbyte);
          newline_attr();

        }
        else if (inbyte==0x0a)  // Line Feed
        { // DO NOTHING
          //fputc_cons(inbyte);
          //newline_attr();   
          //fputc_cons(0x07);  // BEEP
        }
        else if (lastbyte==0x0d && inbyte==0x0a)  // Carriage Return && Line Feed combo
        {
          fputc_cons(inbyte);
          newline_attr();          
          //inbyte=0;  //reset inbyte
        }
        else  // default output the character to the screen 
        {
          fputc_cons(inbyte);
          if (7 != zx_attr(23,31))  // FIX SCROLL ISSUE
          {
            newline_attr();
          }
        }

        lastbyte=inbyte;

        //  quick keyboard check if we are reading alot so we can interupt
        zx_border(INK_CYAN);
        chkey = getk();
        if (chkey != NULL)
        {
          rs232_put(chkey);
          chkey = NULL;
        }

      }
    }
    else //no incoming data check keyboard
    {
      zx_border(INK_CYAN);
      for (int i=20; i>0; i--)
      {
         in_Pause(1);

        chkey = getk();
        if (chkey != NULL)
        {
          rs232_put(chkey);
          chkey = NULL;
        }
      }
    }

  }

}
