// Compiler line (not using make)
// zcc +zx -clib=ansi -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav main.c
// zcc +zx -clib=ansi -pragma-redirect:ansifont=_oemascii -pragma-define:ansifont_is_packed=0 -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav

#include <spectrum.h>
#include <sys/types.h>
#include <conio.h>
#include <stdio.h>
#include <input.h>  //#include <input/input_zx.h>
#include <string.h>
#include <sound.h>  // sound for keyboard click
//#include <sound/bit.h>
#include <stdlib.h>
//#include <ulaplus.h>  //ULA Plus support
#include "snapCterm_Common.h"



#ifdef __SNET__
#include "snapCterm_SNet.h"
#endif

#ifdef __RS232__
#include "snapCterm_RS232.h"
#endif


//  See snapCterm_Common.c/.h for Global Vars
  
char version_num[] = "Beta 2.0.4";

#ifdef __SNET__
char edition[] = "Spectranet";
#endif

#ifdef __RS232__
  #ifdef __IF1__
    char edition[] = "RS232 IF1";
  #endif
  #ifdef __128K__
    char edition[] = "RS232 128K/PLUS";
  #endif
  #ifdef __PLUS3__
    char edition[] = "RS232 MIGHTY +3";
  #endif
#endif



//Font stuff
#asm  
SECTION data_user ;
PUBLIC _oemascii
_oemascii:
      //Files must be loaded in order of lowest value to highest
      BINARY "../src/oemasciiext1.bin.chr" ;
      BINARY "../src/oemasciiext2.bin.chr" ;
      BINARY "../src/oemasciiext3.bin.chr" ;
#endasm

void title(void)
{ 
  char titlescroll = 24;
  
  for (uint8_t z=0;z<10;++z) {DrawCursor();  in_Pause(50);}
  printf("\033[32;40m");
  printf("[\373].80 columns  [\373].ASCII oem set  [\373].ANSI  [\373].colour clash   \n\n");
  for (uint8_t z=0;z<10;++z) {DrawCursor();  in_Pause(100);}  
  printf("Terminal ready... ");
  for (uint8_t z=0;z<10;++z) {DrawCursor();  in_Pause(100);}
  printf("\07"); in_Pause(50);
  ClearCursor();
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
  gotoxy(40-(sizeof(edition)+sizeof(version_num)-16/2),wherey());
  printf("! - %s %s - !\n\n\n",edition,version_num);
  printf("\033[1m\033[33;40m");
  printf("    BY: Owen Reynolds 2018                      \n\n");
  printf("CREDIT: Thomas Cherryhomes @ IRATA.ONLINE       \n\n");
  printf("\033[1m\033[37;40m");
  printf("                        Built using Z88DK - C compiler for Z80s         \n\n");
  printf("\033[1m\033[34;40m");
  printf("               - Join us on Facebook - Z88DK ZX Spectrum user group -   \n\n\033[1m\033[37;40m");
  printf("                        -\\/\\/\\- ANY KEY TO CONTINUE -/\\/\\/- \033[37;40m");
  in_WaitForKey();
  Clear_Keyboard_buffer();

  do
  {
    printf("\r");  
    newline_attr();
  }while(--titlescroll!=0);
}


void Protocol(void)
{
  if (ESC_Code==1) // 0x1b ESC
  {// ESC_Code -- set (True)    
    if (CSI_Code==1) // 0x5b [ (CSI)
    {// ESC_Code CSI_Code -- set
      if (Custom_Code==1) // 0x3b ? (Custom)
      {//ESC_Code CSI_Code SET -- Custom_Code -- set (True)
        if (inbyte >= '0' && inbyte >= '9')
        {// ESC [ ? "0-9"  -- OK Z88DK
          if (ESC_Num_String_Index < ESC_Num_String_Size)
          {
            //Push_inbyte2screen();
            fputc_cons(inbyte);
            ESC_Num_String[ESC_Num_String_Index] = inbyte;  
            ESC_Num_String_Index++;
          }
          else
          {
            printf("!!! ESC_Num_String Buffer over flow - in ESC [ ? 0-9 !!!");
          }  
        }
        else if (inbyte == ';')  
        {// ESC [ ## ; -- Number pair breaks  -- OK Z88DK
          //Push_inbyte2screen();
          ESC_Num_Str2Int();
          fputc_cons(inbyte);
        }
        else
        {// ESC [ ? "Unknown"
          Native_Support();
          bit_beep(1000,30);     // DEBUG -- BEEP CODE
        }
      }
      else
      {//ESC_Code CSI_Code SET -- Custom_Code -- not set (False)
        if (inbyte == 0x3f) 
        {// ESC [ ? -- Custom Code
          Custom_Code = 1;
          fputc_cons(inbyte);
          //Push_inbyte2screen();
        }
        else if (inbyte >= '0' && inbyte <= '9') 
        {// ESC [ "0-9"  -- OK Z88DK
          if(ESC_Num_String_Index < ESC_Num_String_Size)
          {
            //Push_inbyte2screen();
            fputc_cons(inbyte);
            ESC_Num_String[ESC_Num_String_Index] = inbyte;           
            ESC_Num_String_Index++;
          }
          else
          {
            printf("!!! ESC_Num_String Buffer over flow - in ESC [ 0-9 !!!");
          }
        }
        else if (inbyte == ';')  
        {// ESC [ ## ; -- Number pair breaks  -- OK Z88DK
          //Push_inbyte2screen();
          ESC_Num_Str2Int();
          fputc_cons(inbyte);
        }
        else if (inbyte == 'm')
        {// ESC [ # m -- Select Graphic Rendition  -- OK Z88DK
          //Process Handle inbyte as normal > Reverse Clash Correction & inject then re-inject last ESC code > Update SGR > Detect clash/Correct clash & inject

          //Handle the inbyte as usual
          ESC_Num_Str2Int();
          fputc_cons(inbyte);

          if(KlashCorrectToggle == 1)
          {//Reverse colour clash correction   
            
            if (ClashCorrection == 1)
            { 
              ClashCorrection = 0;             
              cprintf("\033[%d;%d;%dm",Bold,ForegroundColour,BackgroundColour);
              if(Underline  == 1){cprintf("\033[4m");}
              if(BlinkSlow  == 1){cprintf("\033[5m");}
              if(BlinkFast  == 1){cprintf("\033[6m");}
              if(Inverse    == 1){cprintf("\033[7m");}
              

              //Replay the ESC last code
              printf("\033[");
              ESC_Num_Int_Counter = 0;
              while (ESC_Num_Int_Counter < ESC_Num_Int_Index)
              {
                itoa(ESC_Num_Int[ESC_Num_Int_Counter],ESC_Num_String,10);
                cprintf("%s",ESC_Num_String);
                fputc_cons(';');
                ESC_Num_Int_Counter++;            
              }
              fputc_cons('m');
            }
          }

          // Update SGR Registers
          ESC_Num_Int_Counter=0;
          while (ESC_Num_Int_Counter < ESC_Num_Int_Index)
          {
            if      (ESC_Num_Int[ESC_Num_Int_Counter] == 0)   {Bold = 0; BlinkSlow = 0; BlinkFast = 0;if(Underline==1){cprintf("\033[24m");Underline=0;} if(Inverse==1){cprintf("\033[27m");Inverse=0;} ForegroundColour=37; BackgroundColour=40;}  // Reset all & inverse if needed  || 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 1)   {Bold = 1;}     // Set Bold (Bright)  
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 2)   {Bold = 0;}     // Set Faint
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 4)   {Underline = 1;}
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 5)   {BlinkSlow = 1;} 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 6)   {BlinkFast = 1;} 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 7)   {Inverse = 1;} 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 21)  {Bold = 0;}
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 22)  {Bold = 0;}     // Normal intensity (rest Faint and Bold)
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 24)  {Underline = 0;}
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 25)  {BlinkSlow = 0; BlinkFast =0;}
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 27)  {Inverse = 0;}     
            else if (ESC_Num_Int[ESC_Num_Int_Counter] >= 30 && ESC_Num_Int[ESC_Num_Int_Counter] <= 39 ) {ForegroundColour = ESC_Num_Int[ESC_Num_Int_Counter];}
            else if (ESC_Num_Int[ESC_Num_Int_Counter] >= 40 && ESC_Num_Int[ESC_Num_Int_Counter] <= 49 ) {BackgroundColour = ESC_Num_Int[ESC_Num_Int_Counter];}
            ESC_Num_Int_Counter++;            
          }

          if(KlashCorrectToggle == 1)
          {//Detect Clash and Inject correction
            if (Bold == 1)  // Clash only occurs when Bold is used
            {
              if(ForegroundColour == (BackgroundColour-10))  // Clash happens when FG & BG are the same and Bold is used to Bright the Text for contrast
              {
                ClashCorrection = 1;  // Flag we have changed the colours to correct clash
                //  Colour Clash correction Table
                switch (ForegroundColour)
                {
                  case 30:  //BLACK   0 30 40   >>  White no bold
                      cprintf("\033[0;37;40m");
                    break;
                  case 31:  //RED     1 31 41   >>  Magenta
                      cprintf("\033[0;35;41m");
                    break;
                  case 32:  //GREEN   2 32 42   >>  Cyan
                      cprintf("\033[0;36;42m");
                    break;
                  case 33:  //YELLOW  3 33 43   >>  White Bold
                      cprintf("\033[1;37;43m");
                    break;
                  case 34:  //BLUE    4 34 44   >>  Cyan
                      cprintf("\033[0;36;44m");
                    break;
                  case 35:  //MAGENTA 5 35 45   >>  White
                      cprintf("\033[0;37;45m");
                    break;
                  case 36:  //CYAN    6 36 46   >>  White
                      cprintf("\033[0;37;46m");
                    break;
                  case 37:  //WHITE   7 37 47   >>  Yellow Bold
                      cprintf("\033[1;37;43m");
                    break;
                  default:
                      printf("!!!ERROR!!! -- Switch statement in KlashCorrection");
                    break;
                }
              }
            }
          }

          Protocol_Reset_All();
        }
        else if (inbyte == 'H')
        {// ESC [ ## ; ## H -- Cursor Position  -- OK Z88DK
          Native_Support();
        }
        else if (inbyte == 'n')  
        {// ESC [ # n -- Device Status Report  -- BROKEN Z88DK (mode 6 only and its broken!)
          // Check the CSI number for action to perform
          switch(atoi(ESC_Num_String))
          {
            case 6 :
              //ESC_CSI_6n();
              txdata[0];
              txbytes = 0;
              cursorY = wherey();
              cursorX = wherex();
              sprintf(txdata,"\033[%d;%dR\0",cursorY,cursorX);
              txbytes = strlen(txdata);
              TX();
            break;
            default :
              //  Beep ?
            break;
          }
          Native_Support();  // May need to change the inbyte to ! and push that to reset ESC sequence?
        }
        else if (inbyte == '!')
        {// ESC [ ! -- Soft Reset  (might be !p not !)
          Native_Support();
        }
        else if (inbyte == 'A')
        {// ESC [ # A -- Cursor UP  -- OK Z88DK
          Native_Support();
        }
        else if (inbyte == 'B')
        {// ESC [ # B -- Cursor DOWN  -- OK Z88DK
          Native_Support();
        }
        else if (inbyte == 'C')
        {// ESC [ # C -- Cursor FORWARD  -- OK Z88DK
          Native_Support();
        }
        else if (inbyte == 'D')
        {// ESC [ # D -- Cursor BACKWARDS  -- OK Z88DK
          Native_Support();
        }
        else if (inbyte == 'J')
        {// ESC [ # J -- Erase in Display  -- OK Z88DK (Clears screen only)        
          switch(atoi(ESC_Num_String))
            {
              /*
              case 0:
                //Esc[0J	Clear screen from cursor down
              break;
              case 1:
                //Esc[1J	Clear screen from cursor up
              break;
              */
              case 2 :  //Esc[2J	Clear entire screen	ED2
                Native_Support();
              break;
              default :
                //Esc[J	Clear screen from cursor down not implmented so botched with ESC[2J!
                fputc_cons('2');
                Native_Support();                
              break;
            }
        }
        else if (inbyte == 'K')
        {// ESC [ # K -- Clear to end of line  -- OK Z88DK (Clear line from cursor right only)
          /*
          Esc[K	Clear line from cursor right
          Esc[0K	Clear line from cursor right
          Esc[1K	Clear line from cursor left
          Esc[2K	Clear entire line
          */
          Native_Support();
        }
        else if (inbyte == 'c')
        {// ESC [ c -- Identify Terminal
          Native_Support();
        }
        else if (inbyte == 's')
        {// ESC [ s -- Save Cursor Location  -- OK Z88DK ( s-j )
          Native_Support();
        }
        else if (inbyte == 'u')
        {// ESC [ u -- Restore Cursor Location  -- OK Z88DK ( u-k )
          Native_Support();
        }
        else 
        {// ESC [ "Unknown"
          //Push_inbyte2screen();
          fputc_cons(inbyte);
          Protocol_Reset_All();
          bit_beep(500,30);     // DEBUG -- BEEP CODE
        }
      }
    }
    else 
    {// ESC_Code CSI_Code -- not set (False)
      if (inbyte == 0x5b) //  [ 
      {// Set -- CSI_Code True 
        CSI_Code = 1;
        fputc_cons(inbyte);
        //Push_inbyte2screen();
      }   
      else 
      {// ESC "Unknown"
        //Push_inbyte2screen();
        fputc_cons(inbyte);
        Protocol_Reset_All();

        bit_beep(100,30);        // DEBUG -- BEEP CODE
      }
    }
  }
  else 
  {// ESC_Code -- not set (False)
    if (inbyte == 0x1b) // ESC
    {// Set -- ESC_Code True
      ESC_Code = 1;
      fputc_cons(inbyte);
      //Push_inbyte2screen();
    }
    else // Just another character to show
    {
      if (inbyte != 0x0d) // We ignore Line Feeds
      {
        Push_inbyte2screen();  //Pushes to screen and checks attribs & fixes
      }
    }
    
  }

}

void KeyReadMulti(unsigned char time_ms, unsigned char repeat)  //ACTIVE  --  TX loop needs work & more Key mappings
{
  //Moved the reset below to after TX
  //kbbytes = 0;
  //kbdata[0]=NULL;
  //zx_border(INK_YELLOW);  //  DEBUG-TIMING
  *KBAttr = PAPER_CYAN;
  do
  {
    if(time_ms>0)
    {
      *KBAttr = PAPER_CYAN + BRIGHT;
      in_Pause(time_ms);
    }

    if (kbbytes < sizeof(kbdata))
    {    
      chkey = getk();
      if (chkey != NULL)  //Key hit translations
      {
        keyboard_click();  
        if      (ExtendKeyFlag == 0)  // Not extended mode
        {
          if      (chkey == 0x0E) {ExtendKeyFlag++;}                                  // Symbol shift condition
          else if (chkey == 0x0C) {kbdata[kbbytes] = 0x08   ;kbbytes = kbbytes+1;}    // Key Back Space (0x0C Form feed > Back space)
          else if (chkey == 0x0A) {kbdata[kbbytes] = 0x0D   ;kbbytes = kbbytes+1;}    // Key ENTER (0x0A NL line feed, new line > 0x13 Carriage Return)        

          else if (chkey == 0xC3) {kbdata[kbbytes] = '|'   ;kbbytes = kbbytes+1;}
          else if (chkey == 0xC5) {kbdata[kbbytes] = ']'   ;kbbytes = kbbytes+1;}
          else if (chkey == 0xC6) {kbdata[kbbytes] = '['   ;kbbytes = kbbytes+1;}
          else if (chkey == 0xCD) {kbdata[kbbytes] = '\\'   ;kbbytes = kbbytes+1;}
          else if (chkey == 0xE2) {kbdata[kbbytes] = '~'   ;kbbytes = kbbytes+1;}
          
          else                    {kbdata[kbbytes] = chkey  ;kbbytes = kbbytes+1;}    // UNCHANGED
        }
        else if (ExtendKeyFlag == 1)  // Level 1 extended mode - PC Keys
        {
          if      (chkey == 0x0E)   {ExtendKeyFlag=0;}                                                                                                            // Exit Extend modes      
          else if (chkey == 'c')    {ExtendKeyFlag=2;}
          else if (chkey == 'k')    {if (KlashCorrectToggle == 0 && MonoFlag == 0){KlashCorrectToggle=1;ExtendKeyFlag=0;} else{KlashCorrectToggle=0;ExtendKeyFlag=0;}}  // Colour Clash correction
          else if (chkey == 'm')    {ToggleMono();}  //  Need to flag this out not call the function                                                                                                           // CTRL > CTRL Extend mode
          
          else if (chkey == 't')    {kbdata[kbbytes] = 0x09; kbbytes = kbbytes+1;}                                                                                // TAB key
          
          else if (chkey == 'e')    {kbdata[kbbytes] = 0x1b; kbbytes = kbbytes+1; ExtendKeyFlag = 0;}                                                             // Escape key
          
          else if (chkey == 'u')    {kbdata[kbbytes] = 0x1b; kbdata[kbbytes+1] = 0x5b; kbdata[kbbytes+2] = '5'; kbdata[kbbytes+3] = '~'; kbbytes = kbbytes+4;}    // Page UP
          else if (chkey == 'd')    {kbdata[kbbytes] = 0x1b; kbdata[kbbytes+1] = 0x5b; kbdata[kbbytes+2] = '6'; kbdata[kbbytes+3] = '~'; kbbytes = kbbytes+4;}    // Page DOWN
          else if (chkey == 'r')    {RunFlag=0;}    // RESET
          
          //CURSOR
          else if (chkey == 0x08)   {kbdata[kbbytes] = 0x1b; kbdata[kbbytes+1] = 'O'; kbdata[kbbytes+2] = 'D'; kbbytes = kbbytes+3;}                              // Cursor key LEFT      
          else if (chkey == 0x0a)   {kbdata[kbbytes] = 0x1b; kbdata[kbbytes+1] = 'O'; kbdata[kbbytes+2] = 'B'; kbbytes = kbbytes+3;}                              // Cursor key DOWN  -- Clash with ENTER
          else if (chkey == 0x0b)   {kbdata[kbbytes] = 0x1b; kbdata[kbbytes+1] = 'O'; kbdata[kbbytes+2] = 'A'; kbbytes = kbbytes+3;}                              // Cursor key UP
          else if (chkey == 0x09)   {kbdata[kbbytes] = 0x1b; kbdata[kbbytes+1] = 'O'; kbdata[kbbytes+2] = 'C'; kbbytes = kbbytes+3;}                              // Cursor key RIGHT

          //ALT 
          //F1 - F10 (F11 F12) ?
          //Home
          //End
          //Insert
          
        }
        else if (ExtendKeyFlag == 2)  // Level 2 extended mode - CTRL + Key combo
        {
          if        (chkey == 0x0E)                  {ExtendKeyFlag=0;}  // EXIT to normal mode
          else if   (chkey == ' ')                   {kbdata[kbbytes] =  0; kbbytes = kbbytes + 1;}
          else if   (chkey == 'a' || chkey == 'A')   {kbdata[kbbytes] =  1; kbbytes = kbbytes + 1;}
          else if   (chkey == 'b' || chkey == 'B')   {kbdata[kbbytes] =  2; kbbytes = kbbytes + 1;}
          else if   (chkey == 'c' || chkey == 'C')   {kbdata[kbbytes] =  3; kbbytes = kbbytes + 1;}
          else if   (chkey == 'd' || chkey == 'D')   {kbdata[kbbytes] =  4; kbbytes = kbbytes + 1;}
          else if   (chkey == 'e' || chkey == 'E')   {kbdata[kbbytes] =  5; kbbytes = kbbytes + 1;}
          else if   (chkey == 'f' || chkey == 'F')   {kbdata[kbbytes] =  6; kbbytes = kbbytes + 1;}
          else if   (chkey == 'g' || chkey == 'G')   {kbdata[kbbytes] =  7; kbbytes = kbbytes + 1;}
          else if   (chkey == 'h' || chkey == 'H')   {kbdata[kbbytes] =  8; kbbytes = kbbytes + 1;}
          else if   (chkey == 'i' || chkey == 'I')   {kbdata[kbbytes] =  9; kbbytes = kbbytes + 1;}
          else if   (chkey == 'j' || chkey == 'J')   {kbdata[kbbytes] = 10; kbbytes = kbbytes + 1;}
          else if   (chkey == 'k' || chkey == 'K')   {kbdata[kbbytes] = 11; kbbytes = kbbytes + 1;}
          else if   (chkey == 'l' || chkey == 'L')   {kbdata[kbbytes] = 12; kbbytes = kbbytes + 1;}
          else if   (chkey == 'm' || chkey == 'M')   {kbdata[kbbytes] = 13; kbbytes = kbbytes + 1;}
          else if   (chkey == 'n' || chkey == 'N')   {kbdata[kbbytes] = 14; kbbytes = kbbytes + 1;}
          else if   (chkey == 'o' || chkey == 'O')   {kbdata[kbbytes] = 15; kbbytes = kbbytes + 1;}
          else if   (chkey == 'p' || chkey == 'P')   {kbdata[kbbytes] = 16; kbbytes = kbbytes + 1;}
          else if   (chkey == 'q' || chkey == 'Q')   {kbdata[kbbytes] = 17; kbbytes = kbbytes + 1;}
          else if   (chkey == 'r' || chkey == 'R')   {kbdata[kbbytes] = 18; kbbytes = kbbytes + 1;}
          else if   (chkey == 's' || chkey == 'S')   {kbdata[kbbytes] = 19; kbbytes = kbbytes + 1;}
          else if   (chkey == 't' || chkey == 'T')   {kbdata[kbbytes] = 20; kbbytes = kbbytes + 1;}
          else if   (chkey == 'u' || chkey == 'U')   {kbdata[kbbytes] = 21; kbbytes = kbbytes + 1;}
          else if   (chkey == 'v' || chkey == 'V')   {kbdata[kbbytes] = 22; kbbytes = kbbytes + 1;}
          else if   (chkey == 'w' || chkey == 'W')   {kbdata[kbbytes] = 23; kbbytes = kbbytes + 1;}
          else if   (chkey == 'x' || chkey == 'X')   {kbdata[kbbytes] = 24; kbbytes = kbbytes + 1;}
          else if   (chkey == 'y' || chkey == 'Y')   {kbdata[kbbytes] = 25; kbbytes = kbbytes + 1;}
          else if   (chkey == 'z' || chkey == 'Z')   {kbdata[kbbytes] = 26; kbbytes = kbbytes + 1;}
          //else if   (chkey == '')   {;}
        }
        else  // RESET  --  Can not happen?
        {
          ExtendKeyFlag = 0;
          printf("!!!PANIC!!! - KeyReadMulti -");
        }
      }

      switch(ExtendKeyFlag)
      {
        case 0 :    // Normal
          zx_border(INK_BLACK);
        break;
        case 1 :    // Extended
          zx_border(INK_GREEN);
        break;
        case 2 :    // CTRL + 
          zx_border(INK_CYAN);
        break;
        default :   // ERROR
          zx_border(INK_MAGENTA);
          ExtendKeyFlag=0;
        break;
      }
      chkey = NULL;
    }
    else 
    {
      zx_border(INK_RED);
      repeat = 1;
    }  
  }while(--repeat!=0);
  

  //zx_border(INK_WHITE);  //DEBUG-TIMING

  if (kbbytes>0 && ESC_Code==0)
  {
    strcpy(txdata,kbdata);
    txbytes=kbbytes;
    TX();
    kbbytes = 0;
    kbdata[0]=NULL;
  }
  //zx_border(INK_BLACK);  //DEBUG-TIMING
  *KBAttr = PAPER_BLACK;
}

void Process_RXdata(void)
{
  rxbyte_count=0;
  do
  {  
    *KBAttr = PAPER_BLUE;     
    ClearCursor();  // Blank characters wont over write the cursor if its showing
    //zx_border(INK_BLACK); //DEBUG-TIMING
    inbyte = rxdata[rxbyte_count];
    Protocol();  // process inbyte

    //QUICK keyboard check if we are reading alot so we can interupt
    //zx_border(INK_CYAN);  //DEBUG
    KeyReadMulti(0,1);  // 2 Reading the keyboard here seemed to break in to ESC [ some times.

  }while(++rxbyte_count<rxbytes);

  rxbytes = 0;
  *KBAttr = PAPER_BLACK;
}

void main(void)
{

  bpoke(0x5C3B, bpeek(0x5C3B) | 0x8); /*Alistair's workaround to autoboot key mode issue */


  zx_border(INK_BLACK);
  zx_colour(PAPER_BLACK|INK_WHITE);
  clrscr();
  title();  //  -- TITLE --  

  do    // MAIN PROGRAM LOOP
  {
    //Init statics clean
    Reset();
    do
    {
      menu();    
      CommsInit();
    }while(io_init==0);
  
    do  // Terminal mode
    {
      DrawCursor();
      RX();

      if (rxbytes>0)
      {
        Process_RXdata();
      }
      else
      {
        KeyReadMulti(10,30);
      }
      
      if(MonoFlag != 0 && MonoFlag != zx_attr(23,31)) {mono();} // Mono flag set, if attr in corner dont match sweep the screen
    }while(RunFlag == 1);
  
  } while (1==1);
}
