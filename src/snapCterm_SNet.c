#ifdef __SNET__
#include <conio.h>
#include <input.h>  //#include <input/input_zx.h>
#include <spectrum.h>
#include <string.h>

//Spectranet headers
#include <sys/socket.h>
#include <sockpoll.h>
#include <netdb.h>

#include "./include/snapCterm_Common.h"


int sockfd, pfd, host_port, result;
struct sockaddr_in remoteaddr;
struct hostent *he;
char host_name[64], port_str[6];
/*  This stuff is setup in snapCterm_Common.h
unsigned char rxdata[4096], ;  //  RX buffer
uint16_t rxbytes, rxbyte_count, rxdata_Size;  //  RX, bytes, counter, size (buffer)
unsigned char txdata[20], txbytes, txbyte_count; //  TX, bytes, counter, size (buffer)
*/

int i; //menu input counter

void CommsInit(void)
{
    sockclose(sockfd);  // Shut that door

    //Resolve the input IP/Hostname, it handles IP addresses
    cprintf("\nResolving : %s",host_name);
    he=gethostbyname(host_name);

    //Create the socket (local port)
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if (sockfd == -1)
    {   cprintf("\nSocket create failed"); in_WaitForKey();  } 
    else 
    {
        cprintf("\n%u Socket",sockfd);
        
        // Bind the socket
        remoteaddr.sin_port=htons(host_port);
        remoteaddr.sin_addr.s_addr=he->h_addr;
        result = connect(sockfd,&remoteaddr,sizeof(struct sockaddr_in));    
        if(result == 0) 
            {io_init=1; cprintf("\nConnected! - Any key to continue"); in_WaitForKey();} 
        else
            {io_init=0; cprintf("\nConnection FAILED! - Any key to continue"); in_WaitForKey();}
    }

    clrscr();
    
}

void RX(void)
{
    *RXAttr = PAPER_RED;      //Indicate RX started    
    pfd=poll_fd(sockfd);
    if (pfd & POLLIN)         // Poll for data
    {
        *RXAttr = PAPER_RED + BRIGHT;
        rxbytes=recv(sockfd,rxdata,rxdata_Size,0);  //  Returns number of bytes moved from socket(sockfd), to rxdata buffer, upto rxdata_Size bytes (MAX), 0 because...
    }
    *KBAttr = PAPER_BLACK;
}

void TX(void)
{
    if (io_init==1)
    {    
        *TXAttr = PAPER_GREEN;  //Indicate TX started
        txbyte_count = 0;
        do
        {  //could get bold with this and change the size to a multiple of size uchar for the amount of characters to send, pushing in one go...
            send(sockfd,&txdata[txbyte_count],sizeof(unsigned char), 0);  // Pushes one Uchar at a time to socket(sockfd), & address of byte (txdata[] eliment), size of data to send, 0 because...
        }while(++txbyte_count<txbytes);
        txbytes = 0;
        txdata[0] = NULL;
        *TXAttr = PAPER_BLACK;    
    }
}

void Draw_Menu(void)
{
    if(strcmp(host_name,"")==0) 
    {
        strcpy(host_name,"127.0.0.1"); 
        host_port=23;
    }
    //       ----------==========----------==========
    cprintf("\033[2J\033[0m");
    cprintf("\n    = snapCterm = SPECTRANET = \n ");
    cprintf("\n1 - Address: \n   > %s:%d \n",host_name,host_port);
    cprintf("\n2 - Buffer size No / Small / Big \n   > "); if(rxdata_Size==no_buf){cprintf("No Buffer",rxdata_Size);} else if(rxdata_Size==small_buf){cprintf("small (%u bytes)",rxdata_Size);} else if(rxdata_Size==big_buf){cprintf("BIG (%u bytes)",rxdata_Size);}else{cprintf("-- (%u bytes)",rxdata_Size);}
    cprintf("\n3 - Clash correction  ON / OFF   \n   > "); if(KlashCorrectToggle == 1){cprintf("ON");}  else{cprintf("OFF");}
    cprintf("\n4 - Mono mode OFF 1 2 3 4 5 6 7  \n   > "); if(MonoFlag==0){cprintf("OFF");} else{cprintf("%d",MonoFlag);}
    cprintf("\n5 - HELP!");
    //cprintf("\n6 - Phonebook");
    //cprintf("\n\n\n");
    //cprintf("\n9 - Hardware detect              > ");
    cprintf("\n\n   Space bar - ! GO TERMINAL ! \n");
    cprintf("\nPress a Number to change settings");

    Clear_Keyboard_buffer();

}

void menu(void)
{
  i=0;
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
        case '31': // Address  

          //flush host_port and port_str, Defaults to port 23 when empty
          host_port=sizeof(port_str);
          do
          {
            port_str[host_port] = NULL;
          } while (--host_port>0);

          // CLEAR MENU
          gotoxy(5,4);
          cprintf("\033[K ");

          // GET HOSTNAME
          gotoxy(5,19);
          cprintf("ENTER IP/DNS ADDRESS");
          gotoxy(5,20);
          cprintf("> ");
          
          i=0;
          do  // Get keys a-z A-Z 0-9 . [Enter] for the DNS/IP address. Stop on enter or end of Array
          {
              do
              {
                  DrawCursor();
                  in_Pause(300);
                  chkey = getk();
              }while(chkey == NULL);
              
              if(chkey!= 12)  // process backspace delete manually
              {
                  if ((chkey >='a' && chkey <='z') || (chkey >='A' && chkey <='Z') || (chkey >='0' && chkey <='9') || (chkey == 10) || (chkey =='.') || (chkey == '-') || (chkey == '_'))
                  { //Valid characters add to address and display
                    ClearCursor();
                    host_name[i]=chkey;
                    cprintf("%c",host_name[i]);
                    i++;
                  }
              }
              else
              { 
                if (i>0)  // only backspace (delete) when forward of the start of the array
                {
                    ClearCursor();
                    i--;
                    gotoxy(wherex()-1,wherey());
                    cprintf(" ");
                    gotoxy(wherex()-1,wherey());
                }
              }
              keyboard_click();
          }while ((host_name[i-1]!=10) && (i < sizeof(host_name)-1));
          host_name[i-1]=NULL;

          // CLEAR input lines
          gotoxy(0,19);
          cprintf("\033[K ");
          gotoxy(0,20);
          cprintf("\033[K ");
          gotoxy(0,21);
          cprintf("\033[K ");
            
          // Next step GET PORT
          gotoxy(5,19);
          cprintf("ENTER PORT (Default 23)");
          gotoxy(5,20);
          cprintf("> ");
          
          i=0;
          do  //  Get keys 0-9 [Enter] for the port upto 5 digits or until enter pressed
          {
              do
              {
                  DrawCursor();
                  in_Pause(10);
                  chkey = getk();
              }while(chkey == NULL);
              
              if(chkey!= 12)  
              {
                  if((chkey >= '0' &&  chkey <= '9') || chkey ==10) 
                  {  // only put valid characters in the port string
                    ClearCursor();
                    port_str[i]=chkey;
                    cprintf("%c",port_str[i]);
                    i++;
                  }
              }
              else
              {
                if (i>0)  // process backspace delete manually
                {   //Valid characters add to address and display
                    ClearCursor();
                    i--;
                    gotoxy(wherex()-1,wherey());
                    cprintf(" ");
                    gotoxy(wherex()-1,wherey());
                }
              }
              keyboard_click();
          }while ((port_str[i-1]!=10) && (i < sizeof(port_str)-1));
          
          host_port = atoi(port_str);  // Convert the port "string" to an int, if 0 then set it to 23 default port
          if (host_port == 0) {host_port=23;}
            // CLEAR input lines
          gotoxy(5,19);
          cprintf("\033[K ");
          gotoxy(5,20);
          cprintf("\033[K ");

            // DONE update menu
          gotoxy(5,4);  // Possition for printing Address
          cprintf("%s:%d",host_name,host_port);
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
          
          gotoxy(4,7);
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
          gotoxy(4,9);
          cprintf("\033[K ");
          if(KlashCorrectToggle == 0) {KlashCorrectToggle=1; cprintf("ON");} else {KlashCorrectToggle=0; cprintf("OFF");}          
          break;
        case '34': // Mono mode
          ToggleMono();
          gotoxy(4,11);
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
          gotoxy(5,11);
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
