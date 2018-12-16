// Compiler line (not using make)
// zcc +zx -clib=ansi -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav main.c
//zcc +zx  -clib=ansi -pragma-redirect:ansifont=_oemascii -pragma-define:ansifont_is_packed=0 -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav

#include <conio.h>
#include <stdio.h>
#include <spectrum.h>
#include <rs232.h>
#include <input.h>
#include <string.h>
#include <sound.h>  // sound for keyboard click

//GLOBALS
unsigned char inbyte, chkey, lastbyte;
char rxdata[18];

//static int bytes,i;
static unsigned char bytes,bytecount;

//Font stuff
#asm  
SECTION data_user ;

PUBLIC _oemascii
_oemascii:
      //Files must be loaded in order of lowest value to highest
      BINARY "./src/oemasciiext1.bin.chr" ;
      BINARY "./src/oemasciiext2.bin.chr" ;
      BINARY "./src/oemasciiext3.bin.chr" ;
#endasm

void scrollfix(uint_fast8_t col)  //NOT IN USE
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

void newline_attr()  //ACTIVE
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

void keyboard_click(void)  //ACTIVE
{//Thomas Cherryhomes key click

  unsigned char i,j;
  for (i=0;i<=10;i++)
    {
      bit_click();
      for (j=0;j<4;j++)
	{
	}
    }

}

void KeyRead(unsigned char time_ms, unsigned char repeat)  //NOT IN USE
{
  //zx_border(INK_GREEN);  //DEBUG-TIMING
  do
  {
    if(time_ms>0)
    {
      in_Pause(time_ms);
    }

    chkey = getk();
    if(chkey ==0x0C) // Key == Back Space (0x0C == Form feed)
    {
      keyboard_click();
      rs232_put(0x08);
    }
    else if (chkey != NULL)
    {
      keyboard_click();
      rs232_put(chkey);
    }
    //chkey = NULL;
  }while(--repeat!=0);
}


void KeyReadMulti(unsigned char time_ms, unsigned char repeat)  //ACTIVE
{
  unsigned char txdata[20];  //Way more than needed in testing i can hardly hit 2 characters
  unsigned char txbytes = 0;
  txdata[0]=NULL;
  //zx_border(INK_RED);  //DEBUG-TIMING
  do
  {
    if(time_ms>0)
    {
      in_Pause(time_ms);
    }

    if (txbytes < sizeof(txdata))
    {    
      chkey = getk();
      if (chkey != NULL)  //Key hit translations
      {
        keyboard_click();        
        // Some key presses need translating to other codes OR ESC[ sequences
        if      (chkey == 0x0C) {txdata[txbytes] = 0x08;} // Key == Back Space (0x0C == Form feed)
        else if (chkey == 0x08) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'D'; txbytes+2;} // Cursor key LEFT
        else if (chkey == 0x0A) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'B'; txbytes+2;} // Cursor key DOWN
        else if (chkey == 0x0B) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'A'; txbytes+2;} // Cursor key UP
        else if (chkey == 0x09) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'C'; txbytes+2;} // Cursor key RIGHT
        else {txdata[txbytes] = chkey;}                   // UNCHANGED
        ++txbytes;
      }
      chkey = NULL;
    }
    else 
    {
      repeat = 0;
    }    
  }while(--repeat!=0);

  
zx_border(INK_WHITE);  //DEBUG-TIMING
  if(txbytes>0)
  {
    //printf("txbytes = %d",txbytes);  //DEBUG-TXBUFFER
    //zx_border(INK_YELLOW);  //DEBUG-TIMING
    //zx_border(txbytes);  //DEBUG-TXBUFFER
    for (unsigned char j=0;j<txbytes;j++)
    {
      rs232_put(txdata[j]);
      //printf("%c",txdata[j]);  //DEBUG-TXBUFFER
    }   
    //printf("\n");  //DEBUG-TXBUFFER
  } 
  zx_border(INK_BLACK);  //DEBUG-TIMING
}



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
  cprintf("\033[?25h");  //Show cursor?
  

  // main loop
  printf("Terminal ready...");
  while(1)
  {
    //RXDATA
    
    if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)  //any incoming data capture and print
    {
      //zx_border(INK_WHITE);  //DEBUG-TIMING
      rxdata[0]=inbyte;         //Buffer the first character
      //bytes = 10;               //Maximum number of bytes to read in.
      bytes = sizeof(rxdata);

      //for (int i=1;i<bytes;i++)   //Loop to buffer in up to 10 characters
      bytecount=1;
      do
      {
        if (rs232_get(&inbyte) != RS_ERR_NO_DATA)  //If character add it to the buffer
        {
          rxdata[bytecount]=inbyte;
        }
        else  //Else no character record the number of bytes we have collected
        {
          bytes = bytecount;
          bytecount = sizeof(rxdata)+1; //kill the for loop
        }

      }while(++bytecount<bytes);
      

      //DRAW SCREEN
      
      //for (int i=0;i<bytes;i++)   //Loop to output the buffer
      bytecount=0;
      do
      {
        //zx_border(INK_BLACK); //DEBUG-TIMING
        inbyte = rxdata[bytecount];

        // filter input here

        if (lastbyte==0xff && inbyte==0xff)  //catch Telnet IAC drop it
        //if(inbyte==0xff)
        {
          //fputc_cons(inbyte);
          inbyte=0;  //reset inbyte
        }
        else if (inbyte == 0x09) // TAB
        {
          //fputc_cons(0x07);  // BEEP-DEBUG
          fputc_cons(inbyte);

        }        
        else if (inbyte == 0x0c) // Clear screen and home cursor
        {
          //fputc_cons(0x07);  // BEEP-DEBUG
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
          //fputc_cons(0x07);  // BEEP-DEBUG
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

        //QUICK keyboard check if we are reading alot so we can interupt
        //zx_border(INK_CYAN);  //DEBUG
        //KeyRead(0,1);
        KeyReadMulti(0,2);

      }while(++bytecount<bytes);
    }
    else //no incoming data check keyboard
    {
      //zx_border(INK_RED);  //DEBUG
      //KeyRead(10,20);
      KeyReadMulti(10,30);
    }

  }

}
