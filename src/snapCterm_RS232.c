#ifdef __RS232__
#include <conio.h>
#include <spectrum.h>
#include <rs232.h>
#include "snapCterm_Common.h"

void SetPort(void)
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


void RX_RS232(void)
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

void TX_RS232(void)
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




#endif