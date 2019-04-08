#include <spectrum.h>
#include <conio.h>
#include <input.h>  //#include <input/input_zx.h>

//Statics

//Transmission TX & RX
unsigned char chkey, inbyte;  // deleted bytecount lastbyte -- To delete
unsigned char rxdata[4096], ; //  RXDATA -- 10[/] 20[/] 40[-] 80[x]  @9600 ~18 @19200 ~50/60
unsigned char txdata[20], txbytes, txbyte_count; //  TX DATA -- 20
unsigned char kbdata[20], kbbytes, kbbyte_count; //  Keyboard buffer and counters
uint16_t rxbytes, rxbyte_count, rxdata_Size;
uint BaudRate;
uint_fast8_t BaudOption; 


//ESC Code registers & variables -- Protocol()
uint_fast8_t   ESC_Num_Int_Size, ESC_Num_String_Size;
uint_fast8_t   ESC_Code, CSI_Code, Custom_Code;
unsigned char  ESC_Num_String[8];                      //  ESC code number string 4[X] 8[-]
uint_fast8_t   ESC_Num_String_Index;                   //  Index for the ESC_Num_String
uint8_t        ESC_Num_Int[8];                         //  ESC code number strings as ints
uint_fast8_t   ESC_Num_Int_Index,ESC_Num_Int_Counter;  //  Index for the ESC_Num_Int, Counter for processing the ESC_Num_Int

//To Sort
unsigned char *CursorAddr;
uint_fast8_t ExtendKeyFlag, CursorFlag, CursorMask, MonoFlag, KlashCorrectToggle;  // deleted - ESCFlag -
int cursorX, cursorY;
uint_fast8_t RunFlag;  



//Scroll fix & Attribute painting -- newline_attr() and mono()
unsigned char row_attr, *attr; 
unsigned char *RXAttr, *TXAttr, *KBAttr;

//SGR registers
uint_fast8_t ClashCorrection, Bold, Underline, Inverse, BlinkSlow, BlinkFast, ForegroundColour, BackgroundColour;


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

  ESC_Num_Int_Size = sizeof(ESC_Num_Int);
  ESC_Num_String_Size = sizeof(ESC_Num_String);

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

void Native_Support(void)  // For pushing ESC codes which wont cause scrolling attr issues
{
  fputc_cons(inbyte);
  Protocol_Reset_All();
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

void Clear_Keyboard_buffer(void)
{
  do  //  PURGE KEY BUFFER OF EVERY THING!!!
  {
    chkey = getk();
  }while (chkey != NULL);
  chkey=NULL;
}

void Hardware_Detect(void)
{
  gotoxy(0,21);

  
  switch (zx_type())
  {
    case 0:
      cprintf("-48K ");
      break;
    case 1:
      cprintf("-128K ");
      break;
    case 2:
      cprintf("-TS2068 ");
      break;
    default:
      cprintf("-Failed type check ");
      break;
  }

  switch (zx_model())
  {
    case 0:
      cprintf("-UNKNOWN ");
      break;
    case 1:
      cprintf("-48K ");
      break;
    case 2:
      cprintf("-128K or +2 ");
      break;
    case 3:
      cprintf("-+2A or Pentagon ");
      break;
    case 4:
      cprintf("-+3 ");
      break;
    case 5:
      cprintf("-+2A/ +3 with bus fixed for games ");
      break;
    case 6:
      cprintf("-TS2068 ");
      break;       
    default:
      cprintf("-Failed Model check ");
      break;
  }
  if (zx_128mode())       {cprintf("-128K Mode ");}
  if (zx_issue3())        {cprintf("-issue3 ");}
  if (zx_printer())        {cprintf("-Printer ");}
  if (zx_soundchip())      {cprintf("-SoundChip ");}
  //if (zx_timexsound())     {cprintf("TimexSound ");} // CRASH
  if (zx_kempstonmouse())  {cprintf("-KempstonMouse ");}
  if (zx_fullerstick())    {cprintf("-Fullerstick Joystick");}
  if (zx_kempston())       {cprintf("-Kempston Joystick ");}
  //if (zx_iss_stick())      {cprintf("ISS Joystick ");}  // CRASH
  if (zx_multiface())      {cprintf("-Multiface ");}
  if (zx_disciple())       {cprintf("-Disciple Floppy ");}
  //if (zx_plus3fdc())       {cprintf("Plus3 Floppy");}  // CRASH
  //if (zx_trd())            {cprintf("TRDOS ");}  // WONT BUILD
  if (zx_extsys())         {cprintf("-BASIC has been moved by an Extension");}
}

void Help(void)
{
  cprintf("\033[2J\033[0m");
  cprintf("\n\nTo toggle the extend mode press and hold Symbol Shift and tap Caps Shift,\nthe border will change from black to green. \nExtend mode keys interpret as below.");
  cprintf("\n\nC - CTRL Key (Control mode Cyan border)\nK - Clash correction (Toggle On/Off)\nM - Mono mode (1 > 7 > Colour mode)\nT - Tab key\nE - Escape key\nU - Page UP\nD - Page DOWN\nCursor keys (5 6 7 8) - Left, Down, Up Right\nR - Reset");
  cprintf("\n\nBlack border - Normal mode\nGreen border - Extended mode\nCyan border - CTRL+ mode");
  cprintf("\n\n  - ANY KEY TO CONTINUE - ");
  in_WaitForKey();
  
  Clear_Keyboard_buffer();

}

void Draw_Menu(void)
{
  cprintf("\033[2J\033[0m");
  cprintf("       snapCterm -- menu\n");
  cprintf("\n1 - Baud :  4800    9600    19200   38400  > %u",BaudRate);
  cprintf("\n2 - Buffer size Small / Big                > "); if(rxdata_Size==18){cprintf("Small (%u bytes)",rxdata_Size);}else{cprintf("BIG (%u bytes)",rxdata_Size);}
  cprintf("\n3 - Clash correction  ON / OFF             > "); if(KlashCorrectToggle == 1){cprintf("ON");}  else{cprintf("OFF");}
  cprintf("\n4 - Mono mode OFF 1 2 3 4 5 6 7            > "); if(MonoFlag==0){cprintf("OFF");} else{cprintf("%d",MonoFlag);}
  cprintf("\n5 - HELP!");
  //cprintf("\n6 - Phonebook");
  cprintf("\n\n\n");
  cprintf("\n9 - Hardware detect                        > ");
  cprintf("\n\n   Space bar - ! GO TERMINAL ! \n");
  cprintf("\n\nPress a Number to change settings");

  Clear_Keyboard_buffer();

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
          if(MonoFlag==0){cprintf("OFF");} else{cprintf("\033[K %d",MonoFlag);}
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
        case '39': // Phonebook
          gotoxy(44,10);
          cprintf("\033[K OK - HW Detect");
          
          Hardware_Detect();
          break;                                        
     
        default:
          //gotoxy(0,20);
          //cprintf("\033[K*Keycode* - %d",chkey);
          break;
        
       
      } 
      mono();
    }
  }while (chkey != 32);    // Space key
  
  Clear_Keyboard_buffer();
  
  clrscr();
  cprintf("\07");
  chkey=NULL;
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
  printf("                              ! - BETA VERSION 1.1 - !       \n\n\n");
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

