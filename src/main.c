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
#include <stdlib.h>

//GLOBALS
static unsigned char inbyte, chkey, lastbyte;
static unsigned char *CursorAddr;
static char rxdata[20];  //18?
static uint_fast8_t ExtendKeyFlag, CursorFlag, CursorMask, ESCFlag;
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
      for (j=0;j<4;j++) { }
  }
	
}

void DrawCursor(void)  // Version 2
{
  
  cursorX = wherex();
  cursorY = wherey();

  if (CursorFlag==0)  {CursorFlag=1;}
  else                {CursorFlag=0;}

  if (cursorX <79)
  {
	  CursorMask = cursorX%8;  
	  switch (CursorMask)
	  {
		  case 0 :
		  CursorAddr = zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (224);	   
		  break;
		  case 1 :
		  CursorAddr=zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (28);
		  break;
		  case 2 :  //SPLIT over 2 bytes
		  CursorAddr=zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (3);
		  CursorAddr=zx_pxy2saddr(cursorX*3+3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (128);
		  break;
		  case 3 :
		  CursorAddr=zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (112);
		  break;
		  case 4 :
		  CursorAddr=zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (14);
		  break;
		  case 5 :  //SPLIT over 2 bytes
		  CursorAddr=zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (1);
		  CursorAddr=zx_pxy2saddr(cursorX*3+3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (192);
		  break;
		  case 6 :
		  CursorAddr=zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (56);
		  break;
		  case 7 :
		  CursorAddr=zx_pxy2saddr(cursorX*3, cursorY*8+7,255);
		  *CursorAddr= *CursorAddr ^ (7);
		  break;		  
		  }
  }
}


void ClearCursor(void)  // Version 2  --  Call this when not putting none printing characters on the screen
{
  if(CursorFlag==1) {DrawCursor();}
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

     if (ExtendKeyFlag == 0)  // Not extended mode
      {
        zx_border(INK_BLACK); 


        if (chkey != NULL)  //Key hit translations
        {
          keyboard_click();        
          // Some key presses need translating to other codes OR ESC[ sequences
          if      (chkey == 0x0C) {txdata[txbytes] = 0x08;} // Key Back Space (0x0C Form feed > Back space)
          else if (chkey == 0x0A) {txdata[txbytes] = 0x0D;} // Key ENTER (0x0A NL line feed, new line > 0x13 Carriage Return)

        //THIS ALL NEEDS A RE-THINK!  move to extended mode 1?

          else if (chkey == 0x08) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'D'; txbytes+2;}                         // Cursor key LEFT      
          else if (chkey == 0x0a) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'B'; txbytes+2;}                         // Cursor key DOWN
          else if (chkey == 0x0b) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'A'; txbytes+2;}                         // Cursor key UP
          else if (chkey == 0x09) {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = 'C'; txbytes+2;}                         // Cursor key RIGHT

          else if (chkey == 0x0E) {ExtendKeyFlag++; txbytes-1;}                                                                                   // Symbol shift condition
          else                    {txdata[txbytes] = chkey;}                                                                                      // UNCHANGED
          ++txbytes;
        }
        chkey = NULL;
      }
      else if (ExtendKeyFlag == 1)  // Level 1 extended mode - PC Keys
      {
        zx_border(INK_GREEN); 
        //printf("ExtendKeyFlag = %d \n",ExtendKeyFlag);

        if        (chkey == 0x0E) {ExtendKeyFlag++;}                                                                                              // Next extended mode      
        else if   (chkey == 'e')  {zx_border(INK_MAGENTA); txdata[txbytes] = 0x1b; txbytes++; ExtendKeyFlag = 0;}                                 // Escape key
        else if   (chkey == 'c')  {ExtendKeyFlag++;}                                                                                              // CTRL key
        else if   (chkey == 'u') {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = '5'; txdata[++txbytes] = '~'; txbytes+3;} // Page UP
        else if   (chkey == 'd') {txdata[txbytes] = 0x1b; txdata[++txbytes] = 0x5b; txdata[++txbytes] = '6'; txdata[++txbytes] = '~'; txbytes+3;} // Page DOWN
        //ALT 
        //F1 - F10 (F11 F12) ?
        //TAB
        //Home
        //End
        //Insert
        
      }
      else if (ExtendKeyFlag == 2)  // Level 2 extended mode - CTRL + Key combo
      {
        zx_border(INK_CYAN); 
        //printf("ExtendKeyFlag = %d \n",ExtendKeyFlag);

        if        (chkey == 0x0E) {ExtendKeyFlag++;}  // Next extended mode
        else if   (chkey == ' ')                   {txdata[txbytes] = 0; txbytes = txbytes + 1;}
        else if   (chkey == 'a' || chkey == 'A')   {txdata[txbytes] = 1; txbytes = txbytes + 1;}
        else if   (chkey == 'b' || chkey == 'B')   {txdata[txbytes] = 2; txbytes = txbytes + 1;}
        else if   (chkey == 'c' || chkey == 'C')   {txdata[txbytes] = 3; txbytes = txbytes + 1;}
        else if   (chkey == 'd' || chkey == 'D')   {txdata[txbytes] = 4; txbytes = txbytes + 1;}
        else if   (chkey == 'e' || chkey == 'E')   {txdata[txbytes] = 5; txbytes = txbytes + 1;}
        else if   (chkey == 'f' || chkey == 'F')   {txdata[txbytes] = 6; txbytes = txbytes + 1;}
        else if   (chkey == 'g' || chkey == 'G')   {txdata[txbytes] = 7; txbytes = txbytes + 1;}
        else if   (chkey == 'h' || chkey == 'H')   {txdata[txbytes] = 8; txbytes = txbytes + 1;}
        else if   (chkey == 'i' || chkey == 'I')   {txdata[txbytes] = 9; txbytes = txbytes + 1;}
        else if   (chkey == 'j' || chkey == 'J')   {txdata[txbytes] = 10; txbytes = txbytes + 1;}
        else if   (chkey == 'k' || chkey == 'K')   {txdata[txbytes] = 11; txbytes = txbytes + 1;}
        else if   (chkey == 'l' || chkey == 'L')   {txdata[txbytes] = 12; txbytes = txbytes + 1;}
        else if   (chkey == 'm' || chkey == 'M')   {txdata[txbytes] = 13; txbytes = txbytes + 1;}
        else if   (chkey == 'n' || chkey == 'N')   {txdata[txbytes] = 14; txbytes = txbytes + 1;}
        else if   (chkey == 'o' || chkey == 'O')   {txdata[txbytes] = 15; txbytes = txbytes + 1;}
        else if   (chkey == 'p' || chkey == 'P')   {txdata[txbytes] = 16; txbytes = txbytes + 1;}
        else if   (chkey == 'q' || chkey == 'Q')   {txdata[txbytes] = 17; txbytes = txbytes + 1;}
        else if   (chkey == 'r' || chkey == 'R')   {txdata[txbytes] = 18; txbytes = txbytes + 1;}
        else if   (chkey == 's' || chkey == 'S')   {txdata[txbytes] = 19; txbytes = txbytes + 1;}
        else if   (chkey == 't' || chkey == 'T')   {txdata[txbytes] = 20; txbytes = txbytes + 1;}
        else if   (chkey == 'u' || chkey == 'U')   {txdata[txbytes] = 21; txbytes = txbytes + 1;}
        else if   (chkey == 'v' || chkey == 'V')   {txdata[txbytes] = 22; txbytes = txbytes + 1;}
        else if   (chkey == 'w' || chkey == 'W')   {txdata[txbytes] = 23; txbytes = txbytes + 1;}
        else if   (chkey == 'x' || chkey == 'X')   {txdata[txbytes] = 24; txbytes = txbytes + 1;}
        else if   (chkey == 'y' || chkey == 'Y')   {txdata[txbytes] = 25; txbytes = txbytes + 1;}
        else if   (chkey == 'z' || chkey == 'Z')   {txdata[txbytes] = 26; txbytes = txbytes + 1;}
        //else if   (chkey == '')   {;}
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
  ESCFlag=0;

  // quick initalise serial port
  rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
  rs232_init();

  // Clean up the screen
  zx_border(INK_BLACK);
  clrscr();
  zx_colour(PAPER_BLACK|INK_WHITE);
  
  //ANSI ESCAPE codes TO SET UP
  cprintf("\033[37;40m");  // esc [ ESC SEQUENCE (Foreground)White;(Background)Black m (to terminate)
  //cprintf("\033[?25h");    //Show cursor?
  
 
  //title();
  
  while(1)  // MAIN PROGRAM LOOP
  {

    DrawCursor();
    //RXDATA
    
    if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)  //any incoming data capture and print
    {
      //zx_border(INK_WHITE);  //DEBUG-TIMING
      rxdata[0]=inbyte;         //Buffer the first character
      bytes = sizeof(rxdata);

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


//  TODO in Draw Screen (ESC code catching)
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

      //DRAW SCREEN
      
      bytecount=0;
      do
      {
        ClearCursor();
        //zx_border(INK_BLACK); //DEBUG-TIMING
        inbyte = rxdata[bytecount];

        //Catch the start of ESC [  --  Need to stop Cursor movement to preserve the ESC [ sequence (drawing and deleting puts to the console)
        if (inbyte == 0x1b)
        {
//          ClearCursor();   // hide cursor
          ESCFlag = 1;  // no cursor moves
        }

        if(lastbyte == 0x1b && inbyte != 0x5b)
        {
          ESCFlag = 0;  // just an escape fine not an ESC [ sequence (reset)
        }

       
        //  Putting to console
        if(inbyte >= 0x40 && inbyte <= 0x7f && inbyte != 0x5b && ESCFlag == 1)  // Catch the END of ESC [  --  Turn Cursor move ON -- Filtering [ (0x5b)
        {
          if(inbyte == 0x6e && lastbyte == 0x36)  //Report cursor possition -- ESC [ ROW ; COLUMN R  Row and column need to be as TEXT
          {
            char s[2];        

            cursorY = wherey();
            cursorX = wherex();

            rs232_put(0x1b);          // ESC
            rs232_put(0x5b);          // [
            
            if(cursorY/10!=0)
            {
              itoa(cursorY/10,s,10);
              rs232_put(s[0]);
            }
            itoa(cursorY%10,s,10);
            rs232_put(s[0]);

            rs232_put(0x3b);          // ;
            
            
            if(cursorX/10!=0)         // # (10s)
            {
              itoa(cursorX/10,s,10);
              rs232_put(s[0]);
            }
            itoa(cursorX%10,s,10);    // # (1s)
            rs232_put(s[0]);

            //rs232_put(0x52);        // R
            rs232_put('R');
          }
//        ClearCursor();
          fputc_cons(inbyte);
          ESCFlag = 0;
//          DrawCursor();
        }
        else if (ESCFlag == 1)  //  During ESC [  --  Cursor move OFF
        {
          //ClearCursor();
          fputc_cons(inbyte);
        }
        else if(ESCFlag == 0)  //  No ESC [  --  Cursor move ON
        {
          
//          ClearCursor();  //  Seems to fix cases where the Cursor isnt over written (if its showing)
          // filter input here

          if (lastbyte==0xff && inbyte==0xff)  //catch Telnet IAC drop it
          //if(inbyte==0xff)
          {
            //fputc_cons(inbyte);
            inbyte=0;  //reset inbyte
          }
          else if(inbyte == 0x08)  //Backspace
          {
    //        ClearCursor();
            fputc_cons(inbyte);
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
    //        ClearCursor();
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
            //ClearCursor();
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
          //DrawCursor();
        }
        else
        {
          printf("!!! PANIC !!!");
          return;
        }

        lastbyte=inbyte;

        //QUICK keyboard check if we are reading alot so we can interupt
        //zx_border(INK_CYAN);  //DEBUG
        //KeyRead(0,1);
        DrawCursor();
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
