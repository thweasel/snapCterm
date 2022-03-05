#ifdef __RS232__
#include <conio.h>
#include <spectrum.h>
#include <input.h>  //#include <input/input_zx.h>
#include <rs232.h>
#include "./include/snapCterm_Common.h"

void CommsInit(void)
{
  rs232_close();
  // quick initalise serial port
  switch (BaudOption)
            {
              case 1:
                rs232_params(RS_BAUD_1200|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                BaudRate = 1200;
                break;
              case 2:
                rs232_params(RS_BAUD_2400|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                BaudRate = 2400;
                break;
              case 3:
                rs232_params(RS_BAUD_4800|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                BaudRate = 4800;
                break;
              case 4:
                BaudRate = 9600;
                rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);  //  Works solid
                break;
              case 5:
                BaudRate = 19200;
                rs232_params(RS_BAUD_19200|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                break;
              case 6:
                BaudRate = 38400;
                rs232_params(RS_BAUD_38400|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                break;          
              default:
                BaudRate = 9600;
                rs232_params(RS_BAUD_9600|RS_STOP_1|RS_BITS_8,RS_PAR_NONE);
                BaudOption = 4;
                break;
            }
  
  rs232_init();
  io_init=1;
}

void RX(void)
{
  *RXAttr = PAPER_RED;      //Indicate RX started    
  if(rs232_get(&inbyte)!=RS_ERR_NO_DATA)  //any incoming data capture 
  {
    *RXAttr = PAPER_RED + BRIGHT;
    //zx_border(INK_WHITE);  //DEBUG-TIMING
    rxbytes = rxdata_Size;    //Use the buffer size selected as the limit of character to catch
    rxdata[0]=inbyte;         //Buffer the first character
    rxbyte_count=1;           //Offset the counter for the first bytes we have
    
    
    
    do
    {
      if (rs232_get(&inbyte) != RS_ERR_NO_DATA)  //If character add it to the buffer
      {
        rxdata[rxbyte_count]=inbyte;
      }
      else  //Else no character record the number of bytes we have collected and exit
      {
        rxbytes = rxbyte_count;  //  Drop RX Bytes to the number of bytes we got before RS_ERR_NO_DATA rxbyte_count will exceed and exit loop
      }
    }while(++rxbyte_count<rxbytes);
   
  }
  *RXAttr = PAPER_BLACK;     //Indicate RX ended
}

void TX(void)
{
    //zx_border(INK_YELLOW);  //DEBUG-TIMING
    *TXAttr = PAPER_GREEN;
    txbyte_count = 0;
    do
    {
    rs232_put(txdata[txbyte_count]);
    }while(++txbyte_count<txbytes);
    txbytes = 0;
    txdata[0] = NULL;
    *TXAttr = PAPER_BLACK;
}

void Draw_Menu(void)
{
  cprintf("\033[2J\033[0m");
  cprintf("\n        = snapCterm = RS232 = \n");
  cprintf("\n1 Baud:1200 2400 4800 9600 19200 38400\n   > %u",BaudRate);
  cprintf("\n2 Buffer size Small / Big             \n   > "); if(rxdata_Size==18){cprintf("Small (%u bytes)",rxdata_Size);}else{cprintf("BIG (%u bytes)",rxdata_Size);}
  cprintf("\n3 Clash correction  ON / OFF          \n   > "); if(KlashCorrectToggle == 1){cprintf("ON");}  else{cprintf("OFF");}
  cprintf("\n4 Mono mode OFF 1 2 3 4 5 6 7         \n   > "); if(MonoFlag==0){cprintf("OFF");} else{cprintf("%d",MonoFlag);}
  cprintf("\n5 HELP!");
  //cprintf("\n6 - Phonebook");
  cprintf("\n\n\n");
  //cprintf("\n9 - Hardware detect                        > ");
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
                BaudRate = 1200;
                break;
              case 2:
                BaudRate = 2400;
                break;
              case 3:
                BaudRate = 4800;
                break;
              case 4:
                BaudRate = 9600;
                break;
              case 5:
                BaudRate = 19200;
                break;
              case 6:
                BaudRate = 38400;
                break;          
              default:
                BaudRate = 1200;
                BaudOption = 1;
                break;
            }
          
          gotoxy(4,4);
          printf("\033[K %u",BaudRate);
          
          break;
        case '32': // Buffer size
          if(rxdata_Size==no_buf)
            {rxdata_Size=small_buf;} 
          else if(rxdata_Size==small_buf)
            {rxdata_Size=big_buf;} 
          else if(rxdata_Size==big_buf)
            {rxdata_Size=no_buf;}
          else
            {rxdata_Size=small_buf;}
          
          gotoxy(4,6);
          cprintf("\033[K ");
          if(rxdata_Size==no_buf)
            {cprintf("No Buffer",rxdata_Size);} 
          else if(rxdata_Size==small_buf)
            {cprintf("small (%u bytes)",rxdata_Size);} 
          else if(rxdata_Size==big_buf)
            {cprintf("BIG (%u bytes)",rxdata_Size);}
          else
            {cprintf("-- (%u bytes)",rxdata_Size);}
          break;
        case '33': // Clash corrections
          gotoxy(4,8);
          cprintf("\033[K ");
          if(KlashCorrectToggle == 0) {KlashCorrectToggle=1; cprintf("ON");} else {KlashCorrectToggle=0; cprintf("OFF");}          
          break;
        case '34': // Mono mode
          ToggleMono();
          gotoxy(4,10);
          cprintf("\033[K ");
          if(MonoFlag==0){cprintf("OFF");} else{cprintf("\033[K %d",MonoFlag);}
          mono();
          break;
        case '35': // HELP!
          gotoxy(4,12);
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
        case '39': // Hardware Detection
          gotoxy(4,11);
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

#endif