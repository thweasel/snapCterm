#ifndef snapCterm_Common
#define snapCterm_Common

//GLOBALS

//Transmission TX & RX
extern unsigned char chkey, inbyte;  // deleted bytecount lastbyte -- To delete
extern unsigned char rxdata[4096], ; //  RXDATA -- 10[/] 20[/] 40[-] 80[x]  @9600 ~18 @19200 ~50/60
extern unsigned char txdata[20], txbytes, txbyte_count; //  TX DATA -- 20
extern unsigned char kbdata[20], kbbytes, kbbyte_count; //  Keyboard buffer and counters
extern uint16_t rxbytes, rxbyte_count, rxdata_Size;  //  Bytes in RXbuffer array | Counter for RX processing | size of the RX array buffer
extern uint BaudRate;
extern uint_fast8_t BaudOption; 


//ESC Code registers & variables -- Protocol()
extern uint_fast8_t   ESC_Num_Int_Size, ESC_Num_String_Size;
extern uint_fast8_t   ESC_Code, CSI_Code, Custom_Code;
extern unsigned char  ESC_Num_String[8];                      //  ESC code number string 4[X] 8[-]
extern uint_fast8_t   ESC_Num_String_Index;                   //  Index for the ESC_Num_String
extern uint8_t        ESC_Num_Int[8];                         //  ESC code number strings as ints
extern uint_fast8_t   ESC_Num_Int_Index,ESC_Num_Int_Counter;  //  Index for the ESC_Num_Int, Counter for processing the ESC_Num_Int

//To Sort
extern unsigned char *CursorAddr;
extern uint_fast8_t ExtendKeyFlag, CursorFlag, CursorMask, MonoFlag, KlashCorrectToggle;  // deleted - ESCFlag -
extern int cursorX, cursorY;
extern uint_fast8_t RunFlag;  



//Scroll fix & Attribute painting -- newline_attr() and mono()
extern unsigned char row_attr, *attr; 
extern unsigned char *RXAttr, *TXAttr, *KBAttr;

//SGR registers
extern uint_fast8_t ClashCorrection, Bold, Underline, Inverse, BlinkSlow, BlinkFast, ForegroundColour, BackgroundColour;

void Reset(void);
void DrawCursor(void);
void ClearCursor(void);
void mono(void);
void ToggleMono(void);
void newline_attr(void);
void Push_inbyte2screen(void);
void Protocol_Reset_All(void);
void ESC_Num_Str2Int(void);
void Native_Support(void);
void keyboard_click(void);
void Clear_Keyboard_buffer(void);
void Hardware_Detect(void);
void Help(void);
void demotitle(void);
//void title(void);
//void Protocol(void);  // Main
//void Process_RXdata(void);  // Main

#endif