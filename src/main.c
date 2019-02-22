// Compiler line (not using make)
// zcc +zx -clib=ansi -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav main.c
// zcc +zx -clib=ansi -pragma-redirect:ansifont=_oemascii -pragma-define:ansifont_is_packed=0 -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav

#include <conio.h>
#include <stdio.h>
#include <spectrum.h>
#include <rs232.h>
#include <input.h>  //#include <input/input_zx.h>
#include <string.h>
#include <sound.h>  // sound for keyboard click

//GLOBALS
static unsigned char inbyte, chkey, lastbyte;
static char rxdata[18];
static uint_fast8_t ExtendKeyFlag, CursorFlag;
static int cursorX, cursorY;

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

/*
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
*/

void newline_attr()  //ACTIVE
{//Allen Albright's method
    unsigned char row_attr;  //static and global these?  Probably change it to a 8 bit int...
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

  // I should probably change the loop counter in here to something static and global this is called alot
  unsigned char i,j;
  for (i=0;i<=10;i++)
  {
      bit_click();
      for (j=0;j<4;j++) 
      {
        //nothing
      }
  }
	
}

void DrawCursor(void)
{
  cursorX = wherex();
  cursorY = wherey();
  if (cursorX <79)
  {
    fputc_cons(219);
    gotoxy(cursorX,cursorY);
  }
}

void ClearCursor(void)
{
  if (cursorX <79)
  {
    fputc_cons(32); //space
    fputc_cons(8);  //backspace!
  }
}

/*
void KeyRead(unsigned char time_ms, unsigned char repeat)  //NOT IN USE
{
  //zx_border(INK_GREEN);  //DEBUG-TIMING
  do
  {
    chkey = NULL;
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
    else if (chkey == 0x0A)  //Enter key remapping
    {
      keyboard_click();
      rs232_put(0x0D);  //enter
    }
    else if (chkey != NULL)
    {
      keyboard_click();
      rs232_put(chkey);
    }
    chkey = NULL;
  }while(--repeat!=0);
}
*/

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

     if (ExtendKeyFlag == 0)  // Not extended mode
      {
        zx_border(INK_BLACK); 


        if (chkey != NULL)  //Key hit translations
        {
          keyboard_click();        
          // Some key presses need translating to other codes OR ESC[ sequences
          if      (chkey == 0x0C) {txdata[txbytes] = 0x08;} // Key Back Space (0x0C Form feed > Back space)
          else if (chkey == 0x0A) {txdata[txbytes] = 0x0D;} // Key ENTER (0x0A NL line feed, new line > 0x13 Carriage Return)


/*
        THIS ALL NEEDS A RE-THINK!  move to extended mode 1?

          else if (chkey == 0x19) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'D'; txbytes+2;} // Cursor key LEFT      
          else if (chkey == 0x1A) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'B'; txbytes+2;} // Cursor key DOWN
          else if (chkey == 0x1B) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'A'; txbytes+2;} // Cursor key UP
          else if (chkey == 0x1C) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'C'; txbytes+2;} // Cursor key RIGHT
          else if (chkey == 0x04) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = '5'; txdata[++txbytes] = '~'; txbytes+3;} // Page UP
          else if (chkey == 0x05) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = '6'; txdata[++txbytes] = '~'; txbytes+3;} // Page DOWN
*/          
          else if (chkey == 0x0E) {ExtendKeyFlag++; txbytes-1;}  // Symbol shift condition
          else                    {txdata[txbytes] = chkey;}                   // UNCHANGED
          ++txbytes;
        }
        chkey = NULL;
      }
      else if (ExtendKeyFlag == 1)  // Level 1 extended mode - PC Keys
      {
        zx_border(INK_GREEN); 
        printf("ExtendKeyFlag = %d \n",ExtendKeyFlag);

        if        (chkey == 0x0E) {ExtendKeyFlag++;}  // Next extended mode      
        else if   (chkey == 'e')  {zx_border(INK_MAGENTA); txdata[txbytes] = 0x1b; txbytes++; ExtendKeyFlag = 0;}  //  Escape key
        else if   (chkey == 'c')  {zx_border(INK_MAGENTA); txdata[txbytes] = 0x1b; txbytes++; ExtendKeyFlag = 0;}// CTRL key
        //ALT 
        //F1 - F10 (F11 F12) ?
        //TAB
        //Page Up
        //Page Down
        //Home
        //End
        //Insert
        
      }
      else if (ExtendKeyFlag == 2)  // Level 2 extended mode - Control + Key combo
      {
        zx_border(INK_CYAN); 
        printf("ExtendKeyFlag = %d \n",ExtendKeyFlag);

        if        (chkey == 0x0E) {ExtendKeyFlag++;}  // Next extended mode
        

      }
      else  // RESET
      {
        zx_border(INK_WHITE);
        ExtendKeyFlag = 0;
      }
    }
    else 
    {
      zx_border(INK_RED);
      repeat = 1;
    }    
  }while(--repeat!=0);

  
//zx_border(INK_WHITE);  //DEBUG-TIMING
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
  //zx_border(INK_BLACK);  //DEBUG-TIMING
}

void title(void)
{ 
  char titlescroll = 24;
  printf("\033[32;40m");
  printf("[\373].80 columns  [\373].ASCII oem set  [\373].ANSI  [\373].colour clash   \n\n");
  printf("Terminal ready... ");
  printf("\033[37;40m");
  printf("\n\033[1m");
  printf("                                     _____  _                          \r");
  printf("                                    /  __ \\| |                         \r");
  printf("           ___  _ __    __ _  _ __  | /  \\/| |_  ___  _ __  _ __ ___   \r");
  printf("          / __|| '_ \\  / _` || '_ \\ | |    | __|/ _ \\| '__|| '_ ` _ \\  \r\033[0m");
  printf("          \\__ \\| | | || (_| || |_) || \\__/\\| |_|  __/| |   | | | | | | \r");
  printf("          |___/|_| |_| \\__,_|| .__/  \\____/ \\__|\\___||_|   |_| |_| |_| \r");
  printf("                             | |                                       \r");
  printf("                             |_|                                       \r");
  printf("\n");
  printf("\033[1m\033[31;40m");
  printf("                         !  -  PRE - ALPHA - VERSION - !       \n\n\n");
  printf("\033[1m\033[33;40m");
  printf("    BY: Owen Reynolds 2018                      \n\n");
  printf("CREDIT: Thomas Cherryhomes @ IRATA.ONLINE       \n\n");
  printf("\033[1m\033[37;40m");
  printf("                        Built using Z88DK - C compiler for Z80s         \n\n");
  printf("\033[1m\033[34;40m");
  printf("               - Join us on Facebook - Z88DK ZX Spectrum user group -   \n\n\033[1m\033[37;40m");
  printf("                        -\\/\\/\\- ANY KEY TO CONTINUE -/\\/\\/- \033[37;40m");

  in_WaitForKey();
  do
  {
    printf("\r");  
    newline_attr();
  }while(--titlescroll!=0);

}

void main(void)
{
  //Init globals clean
  lastbyte=0;
  ExtendKeyFlag=0;
  CursorFlag=0;

  // quick initalise serial port
  rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
  rs232_init();

  // Clean up the screen
  zx_border(INK_BLACK);
  clrscr();
  zx_colour(PAPER_BLACK|INK_WHITE);
  
  //ANSI ESCAPE codes TO SET UP
  cprintf("\033[37;40m");  // esc [ ESC SEQUENCE (Foreground)White;(Background)Black m (to terminate)
  cprintf("\033[?25h");    //Show cursor?
  

  // main loop
 
  title();
  
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
      
      bytecount=0;
      do
      {

//  TODO
//  Need to handle Device Status requests.  
/*
http://www.termsys.demon.co.uk/vtansi.htm
https://en.wikipedia.org/wiki/ANSI_escape_code#Escape_sequences

Device Status

The following codes are used for reporting terminal/display settings, and vary depending on the implementation:

Query Device Code	      <ESC>[c                 Requests a Report Device Code response from the device.
Report Device Code	    <ESC>[{code}0c          Generated by the device in response to Query Device Code request.

Query Device Status	    <ESC>[5n                Requests a Report Device Status response from the device.
Report Device OK	      <ESC>[0n                Generated by the device in response to a Query Device Status request; indicates that device is functioning correctly.
Report Device Failure	  <ESC>[3n                Generated by the device in response to a Query Device Status request; indicates that device is functioning improperly.

Query Cursor Position	  <ESC>[6n                Requests a Report Cursor Position response from the device.
Report Cursor Position	<ESC>[{ROW};{COLUMN}R   Generated by the device in response to a Query Cursor Position request; reports current cursor position.


*/
  

        //zx_border(INK_BLACK); //DEBUG-TIMING
        inbyte = rxdata[bytecount];

     
        //Catch the start of ESC [  --  Need to stop Cursor movement to preserve the ESC [ sequence (drawing and deleting puts to the console)
        if (inbyte == 0x1b)
        {
          ClearCursor();   // hide cursor
          CursorFlag = 1;  // no cursor moves
        }

        if(lastbyte == 0x1b && inbyte != 0x5b)
        {
          CursorFlag = 0;  // just an escape fine
        }

       
        //  Putting to console
        if(inbyte >= 0x40 && inbyte <= 0x7f && inbyte != 0x5b && CursorFlag == 1)  // Catch the end of ESC [  --  Turn Cursor move ON -- Filtering [ (0x5b)
        {
          fputc_cons(inbyte);
          DrawCursor();
          CursorFlag = 0;
        }
        else if (CursorFlag == 1)  //  During ESC [  --  Cursor move OFF
        {
          fputc_cons(inbyte);
        }
        else if(CursorFlag == 0)  //  No ESC [  --  Cursor move ON
        {
          
          ClearCursor();  //You cant do this, as it breaks the ESC codes
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
          DrawCursor();
        }
        else
        {
          printf("!!! PANIC !!!");
          return;
        }

        //if (CursorFlag == 3){CursorFlag=0;}  //Reset the Cursor flag        //First attemp reset condition

        lastbyte=inbyte;

        //QUICK keyboard check if we are reading alot so we can interupt
        //zx_border(INK_CYAN);  //DEBUG
        //KeyRead(0,1);
        KeyReadMulti(0,2);  //broken

      }while(++bytecount<bytes);
    }
    else //no incoming data check keyboard
    {
      //zx_border(INK_RED);  //DEBUG
      //KeyRead(10,20);
      KeyReadMulti(10,30);  //broken  -- conflict on 0x0A
    }

  }

}
