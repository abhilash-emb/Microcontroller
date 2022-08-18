#include "built_in.h" 

#define INACTIVE       0
#define ACTIVE         1

#define SET            1
#define CLEAR          0

#define ENABLE         1
#define DISABLE        0

#define SUCCESS        1
#define FAIL           0

#define DLC_COUNT      0x18
#define DLC_OFFSET     0x20

unsigned char calc_even_parity(unsigned char rxtx_data);
void send_serial_data(unsigned char tx_data);
void send_start_str();
void msmt_str(char);
char Init_system();
char Init_DLC(char );
void Do_command(char *);
char Register_DLC(char);
long Read_DLC(char); 
void Test_DLC();

volatile unsigned char lv_data_rcvd;

unsigned char rx9d_data;
unsigned char rx_data;
unsigned char lv_err;
unsigned char resp_data[8];//={0x00,0xFF,0xFE,0x00,0x0D,0x0A,0,0};
unsigned char char_rcv_cnt,char_cmd_cnt;
unsigned char chck_recv_data;
unsigned int  wt_disp; 
unsigned int rx1_flag,end_command = INACTIVE;
unsigned char DLC_count;
char txt[12];
char command[3];

void interrupt()
{
     if(PIR1.RC1IF) 
     {
         command[char_cmd_cnt++] = RCREG1;
         if(char_cmd_cnt == 2)
         {    
               //command[2] = '\0';
               //PIE1.RC1IE  = DISABLE;
               rx1_flag = SET;
         }
     }
     else if(PIR3.RC2IF) 
     {
          //checking for error
          if(RCSTA2.FERR) {
              rx_data = RCREG2;
          }
          else if(RCSTA2.OERR) {
              RCSTA2.CREN     = 0;
              RCSTA2.CREN     = 1;
          }
          // data recieved
          else {
              rx9d_data                 = RCSTA2.RX9D;
              resp_data[char_rcv_cnt++] = RCREG2;
              //lv_data_rcvd              = 1;
          }
     }
}

void main()
{
     char j = 0 ,response = 0;
     long reading = 0;

     //lv_data_rcvd = 0;
     char_rcv_cnt = 0;
     char_cmd_cnt = 0;
     
     //ANSELB        = 0x00;               // making port b, c and port d as digital
     ANSELD        = 0x00;
     ANSELC        = 0x00;
     UART1MD_bit   = 0;                  // enable uart1 communication
     UART2MD_bit   = 0;                  // enable uart2 communication
     
     TRISD.F6    = 1;
     TRISD.F7    = 1;
     
     SPBRG2       = 103;          // 9600 baud for 16Mhz clock BRGH = 1; 51 for 8 MHz BRGH = 1
     TXSTA2       = 0x64;
     RCSTA2       = 0xD0;
     INTCON.GIE  = SET;
     INTCON.PEIE = SET;    
     PIE3.RC2IE   = ENABLE;
     PIE1.RC1IE   = ENABLE;

     UART1_Init(9600);

     Delay_ms(100);
     DLC_count = EEPROM_Read(DLC_COUNT);

     /*while(char_rcv_cnt < 4) {
         if(lv_data_rcvd) {
             lv_data_rcvd = 0;
             if(calc_even_parity(resp_data[char_rcv_cnt]) != rx9d_data)
                  Lcd_Out(1,2,"Parity Error");

         }
     }*/
     
     Delay_ms(1000);
     
       /*UART1_Write_Text("START");
     UART1_Write(0x0D);
     UART1_Write(0x0A);*/
     
     Delay_ms(500);
     
     char_rcv_cnt = 0;
     
     while(1) 
     {
         while(1)
         {
              if(rx1_flag == SET)
              {
                  char_cmd_cnt = 0;
                  
                  if((command[0] == 'I') && (command[1] == 'N'))
                  {
                       rx1_flag = CLEAR;
                       //PIE1.RC1IE  = ENABLE;
                       //PIE3.RC2IE  = ENABLE;
                       break;
                  }
                  else if((command[0] == 'T') && (command[1] == 'S'))
                  {
                       Test_DLC();
                  }
                  else if((command[0] == 'R'))
                  {
                       Do_command(command);
                  } 
                  else if((command[0] == 'D'))
                  {
                       Do_command(command);
                  }
                  rx1_flag = CLEAR;
                  //PIE1.RC1IE  = ENABLE;
              }
         }
         response = Init_system();
         if(response == SUCCESS)                             // If all DLCs are OK then proceed with reading DLCs, else again wait for INIT
         {
              UART1_Write(response+48);
              while(1)
              {

                      if(rx1_flag == SET)
                      {
                           char_cmd_cnt = 0;
                            
                           Do_command(command);
                           rx1_flag = CLEAR;
                           PIE1.RC1IE  = ENABLE;
                      }
                      else if(end_command == INACTIVE)
                      {
                             reading = 0;
                             for(j=1;j<=DLC_count;j++)
                             {
                                 reading+=Read_DLC(j);
                             }
                             if(reading >=0)
                             {
                                 LongToStr(reading, txt);
                                 UART1_Write_Text(txt);
                             }
                             else
                                 UART1_Write_Text("NO DLC");
                      }
              }
         }
         else
             UART1_Write(response+48);
     }

}

void Test_DLC()
{     
     Delay_ms(50);
     UART1_Write(Init_DLC(1)+48);
          
     if((DLC_count == 2)||(DLC_count == 4)||(DLC_count == 6)||(DLC_count == 8))
     {
         Delay_ms(50);
         UART1_Write(Init_DLC(2)+48);
     }
     if((DLC_count == 4)||(DLC_count == 6)||(DLC_count == 8))
     {
         Delay_ms(50);
         UART1_Write(Init_DLC(3)+48);
             
         Delay_ms(50);
         UART1_Write(Init_DLC(4)+48);
     }
     if((DLC_count == 6)||(DLC_count == 8))
     {
         Delay_ms(50);
         UART1_Write(Init_DLC(5)+48);
             
         Delay_ms(50);
         UART1_Write(Init_DLC(6)+48);
     }
     if(DLC_count == 8)
     {
         Delay_ms(50);
         UART1_Write(Init_DLC(7)+48);
             
         Delay_ms(50);
         UART1_Write(Init_DLC(8)+48);
     }
     
}

long Read_DLC(char ID)
{
    long wt_data = 0,zero_cnt = 0;
    char_rcv_cnt = 0;
    
    msmt_str(ID);
    Delay_ms(100);
    if(resp_data[0] == 0x00 && resp_data[4] == 0x0D && resp_data[5] == 0x0A)
    {
          wt_data |= resp_data[1];
          wt_data |= resp_data[2] << 8;
          wt_data |= resp_data[3] << 16;
          
          zero_cnt |= EEPROM_Read(DLC_OFFSET+(8*(ID-1))); 
          zero_cnt |= EEPROM_Read(DLC_OFFSET+(8*(ID-1))+1)<< 8;
          zero_cnt |= EEPROM_Read(DLC_OFFSET+(8*(ID-1))+2)<< 16;
          
          wt_data -= zero_cnt;
          if(wt_data >= 0)
          {
               wt_data *= 0x03;
               wt_data /= 0x64;
               return wt_data;
          }
          else
               return 0;
    }

}

char Init_system()
{
     char DLC1 = 1,DLC2 = 1,DLC3 = 1,DLC4 = 1,DLC5 = 1,DLC6 = 1,DLC7 = 1,DLC8 = 1;
     //char result1 = 1,result2 = 1,result3 = 1,result4 = 1,result5 = 1;
     
     DLC1 = Init_DLC(1);
      
     if((DLC_count == 2)||(DLC_count == 4)||(DLC_count == 6) || (DLC_count == 8))
     {
         DLC2 = Init_DLC(2);
     }
     if((DLC_count == 4)||(DLC_count == 6) || (DLC_count == 8))
     {
         DLC3 = Init_DLC(3);
         DLC4 = Init_DLC(4);
         /*if(!DLC3 || !DLC4)
             result3 = 0;*/
     }     
     if((DLC_count == 6) || (DLC_count == 8))
     {
         DLC5 = Init_DLC(5);
         DLC6 = Init_DLC(6);
         /*if(!DLC5 || !DLC6)
             result4 = 0;*/
     }
     if(DLC_count == 8)
     {
         DLC7 = Init_DLC(7);
         DLC8 = Init_DLC(8);
         /*if(!DLC7 || !DLC8)
             result5 = 0;*/
     }
     
     if(DLC1 && DLC2 && DLC3 && DLC4 && DLC5 && DLC6 && DLC7 && DLC8)   // All DLCs are working properly
         return SUCCESS;
     else
         return FAIL;
}
char Init_DLC(char ID)
{
     char_rcv_cnt = 0;
     send_serial_data('S');
     send_serial_data('0');
     send_serial_data(ID+48);
     send_serial_data(';');
     send_serial_data('A');
     send_serial_data('D');
     send_serial_data('R');
     send_serial_data('?');
     send_serial_data(';');
     Delay_ms(100);
     if((resp_data[0] == '0') && (resp_data[1] == (ID+48)) )
           return SUCCESS;
     else
           return FAIL;
}

void Do_command(char *cmd)
{
     char status = 1;
     long wt_data = 0;
     if(strlen(cmd) > 1)
     {
         if(cmd[0] == 'R')
         {
              status = Register_DLC(cmd[1]);
              if(status == SUCCESS)
              {   
                  char_rcv_cnt = 0;
                  msmt_str(cmd[1]-48);   // Subtract 48 only when you are calling msmt_str() from Do_command
                  Delay_ms(100);
                  if(resp_data[0] == 0x00 && resp_data[4] == 0x0D && resp_data[5] == 0x0A)
                  {
                        
                        EEPROM_Write(DLC_OFFSET+(8*(cmd[1]-48-1)),   resp_data[1]);
                        EEPROM_Write(DLC_OFFSET+(8*(cmd[1]-48-1))+1, resp_data[2]);
                        EEPROM_Write(DLC_OFFSET+(8*(cmd[1]-48-1))+2, resp_data[3]);
                                            
                        wt_data |= resp_data[1];
                        wt_data |= resp_data[2] << 8;
                        wt_data |= resp_data[3] << 16;
                        //wt_data -= 0x960;
                        if(wt_data >= 0)
                        {
                             LongToStr(wt_data, txt);
                             UART1_Write_Text(txt);
                        } 
                        else
                             UART1_Write_Text("----");     // When count is neagtive
                   }

              }
              else
                  UART1_Write_Text("FAILED");
         }
         else if(cmd[0] == 'D')
         {
             EEPROM_Write(0x18 ,cmd[1]-48);
             Delay_ms(30);
         }
         else if((cmd[0] == 'E') && (cmd[1] == 'D'))
         {
              end_command = ACTIVE;
         }
         else if((cmd[0] == 'B') && (cmd[1] == 'G'))
         {
              end_command = INACTIVE;
         }
         else if((cmd[0] == 'T') && (cmd[1] == 'S'))
         {
              Test_DLC();
         }
     }
}

char Register_DLC(char ID)
{
     //S98;ADR01;TDD1; 
     char_rcv_cnt = 0;
     send_serial_data('S');
     send_serial_data('9');
     send_serial_data('8');
     send_serial_data(';');
     send_serial_data('A');
     send_serial_data('D');
     send_serial_data('R');
     send_serial_data('0');
     send_serial_data(ID);
     send_serial_data(';'); 
     send_serial_data('S');
     send_serial_data('0');
     send_serial_data(ID);
     send_serial_data(';');
     send_serial_data('T');
     send_serial_data('D');
     send_serial_data('D');
     send_serial_data('1');
     send_serial_data(';');
     Delay_ms(100);
     if((resp_data[0] == '0'))
           return SUCCESS;
     else
           return FAIL;
}

unsigned char calc_even_parity(unsigned char rxtx_data)
{
     rxtx_data ^= rxtx_data >> 4;
     rxtx_data ^= rxtx_data >> 2;
     rxtx_data ^= rxtx_data >> 1;
     return rxtx_data & 1;
}

void send_serial_data(unsigned char tx_data)
{
     TXSTA2.TX9D = calc_even_parity(tx_data);
     TXREG2      = tx_data;
     while(!TXSTA2.TRMT);
}

void msmt_str(char ID)
{
      //MSV?:S01;
     send_serial_data('S');
     send_serial_data('0');
     send_serial_data(ID+48); //('1');
     send_serial_data(';');
     send_serial_data('M');
     send_serial_data('S');
     send_serial_data('V');
     send_serial_data('?');
     send_serial_data(';');
}