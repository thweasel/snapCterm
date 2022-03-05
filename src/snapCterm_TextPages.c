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
// VERSION INFORMATION AND EDITION

char version_num[] = "Beta 2.2";

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


#ifdef __80col__
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

void Help(void)
{
  cprintf("\033[2J\033[0m");
  cprintf("\n\nTo toggle the extend mode press and hold Symbol Shift and tap Caps Shift,\nthe border will change from black to green. \nExtend mode keys interpret as below.");
  cprintf("\n\nC - CTRL Key (Control mode Cyan border)\nK - Clash correction (Toggle On/Off)\nM - Mono mode (1 > 7 > Colour mode)\nT - Tab key\nE - Escape key\nU - Page UP\nD - Page DOWN\nCursor keys (5 6 7 8) - Left, Down, Up, Right\nR - Reset");
  cprintf("\n\nBlack border - Normal mode\nGreen border - Extended mode\nCyan border - CTRL+ mode (Toggle Symbol Shift + Caps Shift to exit)");
  cprintf("\n  - ANY KEY TO CONTINUE - ");
  in_WaitForKey();
  
  Clear_Keyboard_buffer();

}

#endif

#ifdef __40col__
void title(void)
{ 
  char titlescroll = 24;
  
  for (uint8_t z=0;z<10;++z) {DrawCursor();  in_Pause(50);}
  printf("\033[32;40m");
  printf("[\373].40 columns  [\373].ASCII oem set  \n[\373].ANSI  [\373].colour clash   \n");
  for (uint8_t z=0;z<10;++z) {DrawCursor();  in_Pause(100);}  
  printf("Terminal ready... ");
  for (uint8_t z=0;z<10;++z) {DrawCursor();  in_Pause(100);}
  printf("\07"); in_Pause(50);
  ClearCursor();
  printf("\033[37;40m");
  printf("\n\033[1m");
  //      ----------==========----------==========
  printf("\n");
  printf("            -----------\n");
  printf("             snapCterm \n");
  printf("            -----------\n");
  printf("\n");
  printf("\n");
  printf("\033[1m\033[31;40m");
  gotoxy(20-(sizeof(edition)+sizeof(version_num)-16/2),wherey());
  printf("! - %s %s - !\n\n\n",edition,version_num);
  printf("\033[1m\033[33;40m");
  printf("    BY: Owen Reynolds 2018                      \n");
  printf("CREDIT: Thomas Cherryhomes @IRATA.ONLINE       \n\n");
  printf("\033[1m\033[37;40m");
  printf("Built using Z88DK - C compiler for Z80s\n");
  printf("\033[1m\033[34;40m");
  printf("Facebook: Z88DK ZX Spectrum user group -\n\n\033[1m\033[37;40m");
  printf("  -\\/\\/\\- ANY KEY TO CONTINUE -/\\/\\/-\033[37;40m");
  in_WaitForKey();
  Clear_Keyboard_buffer();

  do
  {
    printf("\r");  
    newline_attr();
  }while(--titlescroll!=0);
}

void Help(void)
{
  cprintf("\033[2J\033[0m");
  cprintf("\n\nTo toggle the extend mode press and hold\nSymbol Shift and tap Caps Shift, the\nborder will change from black to green. \nExtend mode keys interpret as below.");
  cprintf("\n\nC - CTRL Key (Control mode Cyan border)\nK - Clash correction (Toggle On/Off)\nM - Mono mode (1 > 7 > Colour mode)\nT - Tab key\nE - Escape key\nU - Page UP\nD - Page DOWN\nCursor keys (5678) - Left Down Up Right\nR - Reset");
  cprintf("\n\nBlack border - Normal mode\nGreen border - Extended mode\nCyan border - CTRL+ mode (Toggle Symbol Shift + Caps Shift to exit)");
  cprintf("\n  - ANY KEY TO CONTINUE - ");
  in_WaitForKey();
  
  Clear_Keyboard_buffer();

}

#endif


