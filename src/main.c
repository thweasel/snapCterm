// Compiler line (not using make)
// zcc +zx -clib=ansi -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav main.c
// zcc +zx -clib=ansi -pragma-redirect:ansifont=_oemascii -pragma-define:ansifont_is_packed=0 -pragma-define:ansicolumns=80 -lndos -lm -lrs232plus -create-app -subtype=wav

#include <spectrum.h>
#include <sys/types.h>
#include <conio.h>
#include <stdio.h>
#include <rs232.h>
#include <input.h>  //#include <input/input_zx.h>
#include <string.h>
#include <sound.h>  // sound for keyboard click
//#include <sound/bit.h>
#include <stdlib.h>
//#include <ulaplus.h>  //ULA Plus support


//Statics

//Transmission TX & RX
static unsigned char chkey, inbyte;  // deleted bytecount lastbyte -- To delete
static unsigned char rxdata[4096], ; //  RXDATA -- 10[/] 20[/] 40[-] 80[x]  @9600 ~18 @19200 ~50/60
static unsigned char txdata[20], txbytes, txbyte_count; //  TX DATA -- 20

static uint16_t rxbytes, rxbyte_count, rxdata_Size;

//ESC Code registers & variables -- Protocol()
static uint_fast8_t   ESC_Num_Int_Size=8, ESC_Num_String_Size=8;
static uint_fast8_t   ESC_Code, CSI_Code, Custom_Code;
static unsigned char  ESC_Num_String[8];                      //  ESC code number string 4[X] 8[-]
static uint_fast8_t   ESC_Num_String_Index;                   //  Index for the ESC_Num_String
static uint8_t        ESC_Num_Int[8];                         //  ESC code number strings as ints
static uint_fast8_t   ESC_Num_Int_Index,ESC_Num_Int_Counter;  //  Index for the ESC_Num_Int, Counter for processing the ESC_Num_Int


//To Sort
static unsigned char *CursorAddr;
static uint_fast8_t ExtendKeyFlag, CursorFlag, CursorMask, MonoFlag, KlashCorrectToggle;  // deleted - ESCFlag -
static int cursorX, cursorY;
static uint BaudRate;
static uint_fast8_t BaudOption;  
static uint_fast8_t RunFlag;  

//Scroll fix & Attribute painting -- newline_attr() and mono()
static unsigned char row_attr, *attr; 
static unsigned char *RXAttr, *TXAttr, *KBAttr;

//SGR registers
static uint_fast8_t ClashCorrection, Bold, Inverse, BlinkSlow, BlinkFast, ForegroundColour, BackgroundColour;


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


void ESC_CSI_6n(void)
{//Report cursor possition -- ESC [ ROW ; COLUMN R  Row and column need to be as TEXT
  char s[2];        

  *TXAttr = PAPER_GREEN;
  cursorY = wherey();
  cursorX = wherex();

  rs232_put(0x1b);          // ESC

  rs232_put(0x5b);          // [           

  if(cursorY/10!=0)
  {
    itoa(cursorY/10,s,10);
    rs232_put(s[0]);        // # (10s)
  }
  itoa(cursorY%10,s,10);
  rs232_put(s[0]);          // # (1s)

  rs232_put(0x3b);          // ;           

  if(cursorX/10!=0)         // # (10s)
  {
    itoa(cursorX/10,s,10);
    rs232_put(s[0]);
  }
  itoa(cursorX%10,s,10);    // # (1s)
  rs232_put(s[0]);

  rs232_put('R');           // R
  *TXAttr = PAPER_BLACK;
}

void DrawCursor(void)
{
  cursorX = wherex();
  cursorY = wherey();

  if    (CursorFlag==0) {CursorFlag=1;}
  else                  {CursorFlag=0;}

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

void ClearCursor(void)  // Call this when not putting none printing characters on the screen
{
  if(CursorFlag==1) {DrawCursor();}
}

void newline_attr(void)
{//Allen Albright's method  -- Extended to cover MONO mode
    row_attr = 23;
    attr = zx_cyx2aaddr(23,31);
    //if(MonoFlag > 0 && MonoFlag < 8)
    if(MonoFlag != 0)  // Quick but loose
    {
      *RXAttr = MonoFlag;
      *TXAttr = MonoFlag;
      *KBAttr = MonoFlag;
      do
      {
        memset(attr - 31, MonoFlag, 32);
        attr = zx_aaddrcup(attr);
      }
      while (row_attr-- != 0);
    }
    else 
    {
      *RXAttr = 7;
      *TXAttr = 7;
      *KBAttr = 7;
      do
      {        
        if (7 != *attr)
        memset(attr - 31, 7, 32);
        attr = zx_aaddrcup(attr);
      }
      while (row_attr-- != 0);
    }
}

void mono(void)   //Sweep the screen with mono attribute
{
    if(MonoFlag > 0 && MonoFlag < 8)
    {
      row_attr = 23;
      attr = zx_cyx2aaddr(23,31);
      do
      {
          memset(attr - 31, MonoFlag, 32);
          attr = zx_aaddrcup(attr);
      }
      while (row_attr-- != 0);
    }
    else {MonoFlag=0;}
}

void ToggleMono(void)
{
  MonoFlag++; 
  if(MonoFlag>7)              {ExtendKeyFlag=0;} 
  if(KlashCorrectToggle==1)   {KlashCorrectToggle=0;}
  mono();
} 

void Push_inbyte2screen(void)
{
    fputc_cons(inbyte);
    if (MonoFlag > 0)
    {
      if (7 != zx_attr(23,31))  // FIX SCROLL ISSUE
      {
        newline_attr();
      }
    }
    else
    {
      if (MonoFlag != zx_attr(23,31))  // FIX SCROLL ISSUE
      {
        newline_attr();
      }
    }
}

void Protocol_Reset_All(void)  //  To be called at end of ESC code processing
{
  ESC_Code = 0;
  CSI_Code = 0;
  Custom_Code = 0;
  
  if (ESC_Num_String_Index > 0)
  {
    do
    {
      ESC_Num_String[ESC_Num_String_Index] = NULL;
    }while(--ESC_Num_String_Index > 0);  // Clears Array and resets the index  
  }
  
  if(ESC_Num_Int_Index > 0)
  {
    do
    {
      ESC_Num_Int[ESC_Num_Int_Index] = NULL;
    } while (--ESC_Num_Int_Index > 0);  // Clears Array and resets the index  
  }
}

void Native_Support(void)  // For pushing ESC codes which wont cause scrolling attr issues
{
  fputc_cons(inbyte);
  Protocol_Reset_All();
}

void ESC_Num_Str2Int(void)
{
  if(ESC_Num_Int_Index<ESC_Num_Int_Size)
  {
    ESC_Num_Int[ESC_Num_Int_Index] = atoi(ESC_Num_String);
    ESC_Num_Int_Index++;
  }
  else
  {
    printf("!!! ESC_Num_Int Buffer over flow - in ESC_Num_Str2Int !!!");
  }
  
  if (ESC_Num_String_Index > 0)
  {
    do
    {
      ESC_Num_String[ESC_Num_String_Index] = NULL;
    }while(--ESC_Num_String_Index > 0);  // Clears Array and the index counter 
  }
}

void Protocol(void)
{
  if (ESC_Code==1) // 0x1b ESC
  {// ESC_Code -- set (True)    
    if (CSI_Code==1) // 0x5b [ (CSI)
    {// ESC_Code CSI_Code -- set
      if (Custom_Code==1) // 0x3b ? (Custom)
      {//ESC_Code CSI_Code SET -- Custom_Code -- set (True)
        if(inbyte >= '0' && inbyte >= '9')
        {// ESC [ ? "0-9"  -- OK Z88DK
          if(ESC_Num_String_Index < ESC_Num_String_Size)
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
        else if(inbyte == ';')  
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
        if(inbyte == 0x3f) 
        {// ESC [ ? -- Custom Code
          Custom_Code = 1;
          fputc_cons(inbyte);
          //Push_inbyte2screen();
        }
        else if(inbyte >= '0' && inbyte <= '9') 
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
        else if(inbyte == ';')  
        {// ESC [ ## ; -- Number pair breaks  -- OK Z88DK
          //Push_inbyte2screen();
          ESC_Num_Str2Int();
          fputc_cons(inbyte);
        }
        else if(inbyte == 'm')
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
              cprintf("\033[%d;%d;%d;%d;%d;%dm",Bold,BlinkSlow,BlinkFast,Inverse,ForegroundColour,BackgroundColour);

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
            if      (ESC_Num_Int[ESC_Num_Int_Counter] == 0)   {Bold = 0; BlinkSlow = 0; BlinkFast = 0; if(Inverse==1){cprintf("\033[27m");Inverse=0;} ForegroundColour=37; BackgroundColour=40;}  // Reset all & inverse if needed  || 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 1)   {Bold = 1;}     // Set Bold (Bright)  
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 2)   {Bold = 0;}     // Set Faint
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 5)   {BlinkSlow = 0;} 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 6)   {BlinkFast = 0;} 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 7)   {Inverse = 1;} 
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 21)  {Bold = 0;}
            else if (ESC_Num_Int[ESC_Num_Int_Counter] == 22)  {Bold = 0;}     // Normal intensity (rest Faint and Bold)
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
        else if (inbyte == 'n')  
        {// ESC [ # n -- Device Status Report  -- BROKEN Z88DK (mode 6 only and its broken!)
          // Check the CSI number for action to perform
          switch(atoi(ESC_Num_String))
          {
            case 6 :
              ESC_CSI_6n();
            break;
            default :
              //  Beep ?
            break;
          }
          Native_Support();  // May need to change the inbyte to ! and push that to reset ESC sequence?
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
      if(inbyte == 0x5b) //  [ 
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

void keyboard_click(void)  //ACTIVE
{//Thomas Cherryhomes key click
  unsigned char i,j;
  for (i=0;i<=10;i++)
  {
      bit_click();
      for (j=0;j<4;j++) {/*NOP*/}
  }
}

void KeyReadMulti(unsigned char time_ms, unsigned char repeat)  //ACTIVE  --  TX loop needs work & more Key mappings
{
  //Moved the reset below to after TX
  //txbytes = 0;
  //txdata[0]=NULL;
  //zx_border(INK_YELLOW);  //  DEBUG-TIMING
  *KBAttr = PAPER_CYAN;
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
        if      (ExtendKeyFlag == 0)  // Not extended mode
        {
          if      (chkey == 0x0E) {ExtendKeyFlag++;}                                  // Symbol shift condition
          else if (chkey == 0x0C) {txdata[txbytes] = 0x08   ;txbytes = txbytes+1;}    // Key Back Space (0x0C Form feed > Back space)
          else if (chkey == 0x0A) {txdata[txbytes] = 0x0D   ;txbytes = txbytes+1;}    // Key ENTER (0x0A NL line feed, new line > 0x13 Carriage Return)        
          else                    {txdata[txbytes] = chkey  ;txbytes = txbytes+1;}    // UNCHANGED
        }
        else if (ExtendKeyFlag == 1)  // Level 1 extended mode - PC Keys
        {
          if      (chkey == 0x0E)   {ExtendKeyFlag=0;}                                                                                                            // Exit Extend modes      
          else if (chkey == 'c')    {ExtendKeyFlag=2;}
          else if (chkey == 'k')    {if (KlashCorrectToggle == 0 && MonoFlag == 0){KlashCorrectToggle=1;ExtendKeyFlag=0;} else{KlashCorrectToggle=0;ExtendKeyFlag=0;}}  // Colour Clash correction
          else if (chkey == 'm')    {ToggleMono();}  //  Need to flag this out not call the function                                                                                                           // CTRL > CTRL Extend mode
          
          else if (chkey == 't')    {txdata[txbytes] = 0x09; txbytes = txbytes+1;}                                                                                // TAB key
          
          else if (chkey == 'e')    {txdata[txbytes] = 0x1b; txbytes = txbytes+1; ExtendKeyFlag = 0;}                                                             // Escape key
          
          else if (chkey == 'u')    {txdata[txbytes] = 0x1b; txdata[txbytes+1] = 0x5b; txdata[txbytes+2] = '5'; txdata[txbytes+3] = '~'; txbytes = txbytes+4;}    // Page UP
          else if (chkey == 'd')    {txdata[txbytes] = 0x1b; txdata[txbytes+1] = 0x5b; txdata[txbytes+2] = '6'; txdata[txbytes+3] = '~'; txbytes = txbytes+4;}    // Page DOWN
          else if (chkey == 'r')    {RunFlag=0;}    // RESET
          
          //CURSOR
          else if (chkey == 0x08)   {txdata[txbytes] = 0x1b; txdata[txbytes+1] = 'O'; txdata[txbytes+2] = 'D'; txbytes = txbytes+3;}                              // Cursor key LEFT      
          else if (chkey == 0x0a)   {txdata[txbytes] = 0x1b; txdata[txbytes+1] = 'O'; txdata[txbytes+2] = 'B'; txbytes = txbytes+3;}                              // Cursor key DOWN  -- Clash with ENTER
          else if (chkey == 0x0b)   {txdata[txbytes] = 0x1b; txdata[txbytes+1] = 'O'; txdata[txbytes+2] = 'A'; txbytes = txbytes+3;}                              // Cursor key UP
          else if (chkey == 0x09)   {txdata[txbytes] = 0x1b; txdata[txbytes+1] = 'O'; txdata[txbytes+2] = 'C'; txbytes = txbytes+3;}                              // Cursor key RIGHT

          //ALT 
          //F1 - F10 (F11 F12) ?
          //Home
          //End
          //Insert
          
        }
        else if (ExtendKeyFlag == 2)  // Level 2 extended mode - CTRL + Key combo
        {
          if        (chkey == 0x0E)                  {ExtendKeyFlag=0;}  // EXIT to normal mode
          else if   (chkey == ' ')                   {txdata[txbytes] =  0; txbytes = txbytes + 1;}
          else if   (chkey == 'a' || chkey == 'A')   {txdata[txbytes] =  1; txbytes = txbytes + 1;}
          else if   (chkey == 'b' || chkey == 'B')   {txdata[txbytes] =  2; txbytes = txbytes + 1;}
          else if   (chkey == 'c' || chkey == 'C')   {txdata[txbytes] =  3; txbytes = txbytes + 1;}
          else if   (chkey == 'd' || chkey == 'D')   {txdata[txbytes] =  4; txbytes = txbytes + 1;}
          else if   (chkey == 'e' || chkey == 'E')   {txdata[txbytes] =  5; txbytes = txbytes + 1;}
          else if   (chkey == 'f' || chkey == 'F')   {txdata[txbytes] =  6; txbytes = txbytes + 1;}
          else if   (chkey == 'g' || chkey == 'G')   {txdata[txbytes] =  7; txbytes = txbytes + 1;}
          else if   (chkey == 'h' || chkey == 'H')   {txdata[txbytes] =  8; txbytes = txbytes + 1;}
          else if   (chkey == 'i' || chkey == 'I')   {txdata[txbytes] =  9; txbytes = txbytes + 1;}
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
  *KBAttr = PAPER_BLACK;

  //zx_border(INK_WHITE);  //DEBUG-TIMING

  if(txbytes>0 && ESC_Code==0)//ONLY TX when not RX an ESC Code.  only exception is replying to ESC[6n function
  {
      //zx_border(INK_YELLOW);  //DEBUG-TIMING
      *TXAttr = PAPER_GREEN;
      txbyte_count = 0;
      do
      {
        rs232_put(txdata[txbyte_count]);
      }while(++txbyte_count<txbytes);
      *TXAttr = PAPER_BLACK;
      txbytes = 0;
      txdata[0]=NULL;
  }
  //zx_border(INK_BLACK);  //DEBUG-TIMING
}

void demotitle(void)
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
  printf("                              ! - BETA - VERSION - !       \n\n\n");
  printf("\033[1m\033[33;40m");
  printf("    BY: Owen Reynolds 2018                      \n\n");
  printf("CREDIT: Thomas Cherryhomes @ IRATA.ONLINE       \n\n");
  printf("\033[1m\033[37;40m");
  printf("                        Built using Z88DK - C compiler for Z80s         \n\n");
  printf("\033[1m\033[34;40m");
  printf("               - Join us on Facebook - Z88DK ZX Spectrum user group -   \n\n\033[1m\033[37;40m");
  printf("                        -\\/\\/\\- ANY KEY TO CONTINUE -/\\/\\/- \033[37;40m");
  in_WaitForKey();
  void=getk();
  do
  {
    printf("\r");  
    newline_attr();
  }while(--titlescroll!=0);
}
/*
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
  printf("                              ! - BETA - VERSION - !       \n\n\n");
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
*/

void Help()
{
  cprintf("\033[2J\033[0m");
  cprintf("\n\nTo toggle the extend mode press and hold Symbol Shift and tap Caps Shift,\nthe border will change from black to green. \nExtend mode keys interpret as below.");
  cprintf("\n\nC - CTRL Key (Control mode Cyan border)\nK - Clash correction (Toggle On/Off)\nM - Mono mode (1 > 7 > Colour mode)\nT - Tab key\nE - Escape key\nU - Page UP\nD - Page DOWN\nCursor keys (5 6 7 8) - Left, Down, Up Right\nR - Reset");
  cprintf("\n\nBlack border - Normal mode\nGreen border - Extended mode\nCyan border - CTRL+ mode");
  cprintf("\n\n  - ANY KEY TO CONTINUE - ");
  in_WaitForKey();
}
void Reset(void)
{
  RunFlag = 1;  
  BaudOption = 2;
  BaudRate = 9600;
  ExtendKeyFlag=0;
  CursorFlag=0;
  
  RXAttr = zx_cyx2aaddr(0,31);
  TXAttr = zx_cyx2aaddr(1,31);
  KBAttr = zx_cyx2aaddr(2,31);

  ESC_Code=0;
  CSI_Code=0;
  Custom_Code=0;

  ESC_Num_String_Index=0;
  ESC_Num_Int_Index=0;
  ESC_Num_Int_Counter=0;
  ClashCorrection = 0;
  KlashCorrectToggle=1;

  rxdata_Size=4096;
  rxbytes=0;
  rxbyte_count=0;

  txbytes=0;
  txbyte_count=0;
  
  // Clean up the screen
  zx_border(INK_BLACK);
  zx_colour(PAPER_BLACK|INK_WHITE);
  clrscr();
  
  //ANSI ESCAPE codes TO SET UP
  cprintf("\033[37;40m");  // esc [ ESC SEQUENCE (Foreground)White;(Background)Black m (to terminate)
  //SGR Register setup
  ForegroundColour = 37;
  BackgroundColour = 40;
  Bold = 0;
  Inverse = 0;
  BlinkSlow = 0;
  BlinkFast = 0;


}

void SetPort()
{
  rs232_close();
  // quick initalise serial port
  switch (BaudOption)
            {
              case 1:
                rs232_params(RS_BAUD_4800|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                BaudRate = 4800;
                break;
              case 2:
                BaudRate = 9600;
                rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);  //  Works solid
                break;
              case 3:
                BaudRate = 19200;
                rs232_params(RS_BAUD_19200|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                break;
              case 4:
                BaudRate = 38400;
                rs232_params(RS_BAUD_38400|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                break;          
              default:
                BaudRate = 9600;
                rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                BaudOption = 2;
                break;
            }
  
  rs232_init();
}

void Draw_Menu(void)
{
  cprintf("\033[2J\033[0m");
  cprintf("       snapCterm -- menu\n");
  cprintf("\n1 - Baud :  4800    9600    19200   38400  > %u",BaudRate);
  cprintf("\n2 - Buffer size Small / Big                > "); if(rxdata_Size==18){cprintf("Small (%u bytes)",rxdata_Size);}else{cprintf("BIG (%u bytes)",rxdata_Size);}
  cprintf("\n3 - Clash correction  ON / OFF             > "); if(KlashCorrectToggle == 1){cprintf("ON");}  else{cprintf("OFF");}
  cprintf("\n4 - Mono mode OFF 1 2 3 4 5 6 7            > "); if(MonoFlag==0){cprintf("ON");} else{cprintf("%d",MonoFlag);}
  cprintf("\n5 - HELP!");
  //cprintf("\n6 - Phonebook");
  cprintf("\n\n   Space bar - ! GO TERMINAL ! \n");

}

void menu(void)
{
  Draw_Menu();

  do
  {
    in_WaitForKey();
    chkey = NULL;
    chkey=getk();
    if(chkey != NULL)
    {
      keyboard_click();
      //cprintf("\n%x",chkey);
      switch (chkey)
      {
        case '31': // Baud rate     
          BaudOption++;
          switch (BaudOption)
          {
            case 1:
              BaudRate = 4800;
              break;
            case 2:
              BaudRate = 9600;
              break;
            case 3:
              BaudRate = 19200;
              break;
            case 4:
              BaudRate = 38400;
              break;          
            default:
              BaudRate = 4800;
              BaudOption = 1;
              break;
          }
          gotoxy(44,2);
          printf("\033[K %u",BaudRate);
          break;
        case '32': // Buffer size
          if (rxdata_Size==4096){rxdata_Size=18;}else{rxdata_Size=4096;}
          gotoxy(44,3);
          cprintf("\033[K ");
          if(rxdata_Size==18){cprintf("Small (%u bytes)",rxdata_Size);}else{cprintf("BIG (%u bytes)",rxdata_Size);}
          break;
        case '33': // Clash corrections
          gotoxy(44,4);
          cprintf("\033[K ");
          if(KlashCorrectToggle == 0) {KlashCorrectToggle=1; cprintf("ON");} else {KlashCorrectToggle=0; cprintf("OFF");}          
          break;
        case '34': // Mono mode
          ToggleMono();
          gotoxy(44,5);
          cprintf("\033[K ");
          if(MonoFlag==0){cprintf("ON");} else{cprintf("\033[K %d",MonoFlag);}
          mono();
          break;
        case '35': // HELP!
          gotoxy(44,6);
          cprintf("\033[K Help");
          Help();
          Draw_Menu();
          break; 
 /*       case '36': // Phonebook
          gotoxy(44,7);
          cprintf("\033[K Phonebook");
          
          Draw_Menu();
          break;                                        
  */    
        default:
          //gotoxy(0,20);
          //cprintf("\033[K*Keycode* - %d",chkey);
          break;
        
       
      } 
      mono();
    }
  }while (chkey != 32);    // Space key
  clrscr();
  cprintf("\07");
  chkey=NULL;
}

void main(void)
{
  zx_border(INK_BLACK);
  zx_colour(PAPER_BLACK|INK_WHITE);
  clrscr();
  //title();  //  -- TITLE --  
  demotitle();

  do    // MAIN PROGRAM LOOP
  {
    //Init statics clean
    Reset();

    menu();
    
    SetPort();
  
    do  // Terminal mode
    {
      DrawCursor();

      //RXDATA  --  move to function?
      if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)  //any incoming data capture and print
      {
        //zx_border(INK_WHITE);  //DEBUG-TIMING

        rxdata[0]=inbyte;         //Buffer the first character
        rxbytes = rxdata_Size;
        
        *RXAttr = PAPER_RED;
        rxbyte_count=1;
        do
        {
          if (rs232_get(&inbyte) != RS_ERR_NO_DATA)  //If character add it to the buffer
          {
            rxdata[rxbyte_count]=inbyte;
          }
          else  //Else no character record the number of bytes we have collected
          {
            rxbytes = rxbyte_count;
            rxbyte_count = rxdata_Size+1; //kill the for loop
          }

        }while(++rxbyte_count<rxbytes);
        *RXAttr = PAPER_BLACK;
        //DRAW SCREEN  (Technically process the RX Buffer, act on ESC code and push text to screen)
        
        rxbyte_count=0;
        do
        {       
          ClearCursor();  // Blank characters wont over write the cursor if its showing
          //zx_border(INK_BLACK); //DEBUG-TIMING
          inbyte = rxdata[rxbyte_count];
          Protocol();  // process inbyte

          //QUICK keyboard check if we are reading alot so we can interupt
          
            //zx_border(INK_CYAN);  //DEBUG
            KeyReadMulti(0,1);  // 2 Reading the keyboard here seemed to break in to ESC [ some times.

        }while(++rxbyte_count<rxbytes);
      }
      else //no incoming data check keyboard
      {
        //zx_border(INK_RED);  // DEBUG
        KeyReadMulti(10,30);   // 10,30
        //KeyReadMulti(0,1);
      }

      if(MonoFlag != 0 && MonoFlag != zx_attr(23,31)) {mono();} // Mono flag set, if attr in corner dont match sweep the screen

    }while(RunFlag == 1);
  
  } while (1==1);
}
