#ifdef __SNET__
#include <conio.h>
#include <input.h>  //#include <input/input_zx.h>
#include <spectrum.h>
#include <sys/socket.h>
#include <sockpoll.h>
#include <netdb.h>
#include <string.h>
#include "snapCterm_Common.h"

uint_fast8_t io_initialized=0;

int sockfd, pfd, host_port;
struct sockaddr_in remoteaddr;
struct hostent *he;
char host_name[64];

void CommsInit(void)
{
    strcpy(host_name,"amstrad.simulant.uk");    //  will need to be removed menu option
    host_port = 464;                            //  will need to be removed menu option
    he=gethostbyname(host_name);
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    remoteaddr.sin_port=htons(host_port);
    remoteaddr.sin_addr.s_addr=he->h_addr;
    connect(sockfd,&remoteaddr,sizeof(struct sockaddr_in));    
    io_initialized=1;
}

void RX(void)
{
    *RXAttr = PAPER_RED;      //Indicate RX started    
    pfd=poll_fd(sockfd);
    if (pfd & POLLIN)
    {
        *RXAttr = PAPER_RED + BRIGHT;
        rxbytes=recv(sockfd,rxdata,rxdata_Size,0);
    }
    *KBAttr = PAPER_BLACK;
}

void TX(void)
{
    if (io_initialized==1)
    {    
        *TXAttr = PAPER_GREEN;
        txbyte_count = 0;
        do
        {
            send(sockfd,&txdata[txbyte_count],sizeof(unsigned char), 0);
        }while(++txbyte_count<txbytes);
        txbytes = 0;
        txdata[0] = NULL;
        *TXAttr = PAPER_BLACK;    
    }
}

void Draw_Menu(void)
{
  cprintf("\033[2J\033[0m");
  cprintf("       snapCterm SPECTRANET EDITION -- menu\n");
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


#endif