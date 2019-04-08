#ifdef __RS232__
#include <conio.h>
#include <spectrum.h>
#include <rs232.h>
#include "snapCterm_Common.h"

void TX_RS232(void)
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
    txdata[0] = NULL;
}


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

#endif