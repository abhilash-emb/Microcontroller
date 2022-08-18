unsigned char calc_even_parity(unsigned char rxtx_data);
void send_serial_data(unsigned char tx_data);
void send_start_str();
void msmt_str();
void Soft_UART_Send_Str(char *);

volatile unsigned char lv_data_rcvd;

unsigned char rx9d_data;
unsigned char rx_data;
unsigned char lv_err;
unsigned char resp_data[8];//={0x00,0xFF,0xFE,0x00,0x0D,0x0A,0,0};
unsigned char char_rcv_cnt;
unsigned char chck_recv_data;
unsigned int  wt_disp;


void interrupt()
{
     if(PIR1.RC1IF) {
          //checking for error
          if(RCSTA1.FERR) {
              rx_data        = RCREG1;
          }
          else if(RCSTA.OERR) {
              RCSTA1.CREN     = 0;
              RCSTA1.CREN     = 1;
          }
          // data recieved
          else {
              rx9d_data                 = RCSTA1.RX9D;
              resp_data[char_rcv_cnt++] = RCREG1;
              //lv_data_rcvd              = 1;
          }
     }
}

void main()
{
     long wt_data = 0;
     long zero_val = 2400;
     char txt[12],error = 0;

     //lv_data_rcvd = 0;
     char_rcv_cnt = 0;
     
     ANSELB        = 0x00;               // making port b, c and port d as digital
     ANSELD        = 0x00;
     ANSELC        = 0x00;
     UART1MD_bit   = 0;                  // enable uart1 communication
     UART2MD_bit   = 0;                  // enable uart2 communication
     
     TRISC.F6    = 1;
     TRISC.F7    = 1;
     SPBRG1       = 103;          // 9600 baud for 16Mhz clock BRGH = 1; 51 for 8 MHz BRGH = 1
     TXSTA1       = 0x64;
     RCSTA1       = 0xD0;
     INTCON.GIE  = 1;
     INTCON.PEIE = 1;
     PIE1.RC1IE   = 1;

     UART2_Init(9600);

     Delay_ms(1000);

     send_start_str();
     Delay_ms(1000);
     /*while(char_rcv_cnt < 4) {
         if(lv_data_rcvd) {
             lv_data_rcvd = 0;
             if(calc_even_parity(resp_data[char_rcv_cnt]) != rx9d_data)
                  Lcd_Out(1,2,"Parity Error");

         }
     }*/

     /*Lcd_Cmd(_LCD_CLEAR);
     Lcd_Out(1,2,resp_data);*/ 
     Delay_ms(1000);  
                 UART2_Write_Text("START");

     char_rcv_cnt = 0;
     
     while(1) 
     {

         char_rcv_cnt = 0;
         msmt_str();
         Delay_ms(100);
         if(resp_data[0] == 0x00 && resp_data[4] == 0x0D && resp_data[5] == 0x0A) 
         {
             wt_data |= resp_data[1];
             wt_data |= resp_data[2] << 8;
             wt_data |= resp_data[3] << 16;
             //wt_data -= 0x960;
             if(wt_data >= 0) {
                 wt_data *= 0x03;
                 wt_data /= 0x64;
                 LongToStr(wt_data, txt);
                 UART2_Write_Text(txt);
             }
         }
     }

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
     TXSTA1.TX9D = calc_even_parity(tx_data);
     TXREG1      = tx_data;
     while(!TXSTA1.TRMT);
}

void send_start_str()
{
     //S01;S01;ADR?;
     send_serial_data('S');
     send_serial_data('0');
     send_serial_data('1');
     send_serial_data(';');
     send_serial_data('S');
     send_serial_data('0');
     send_serial_data('1');
     send_serial_data(';');
     send_serial_data('A');
     send_serial_data('D');
     send_serial_data('R');
     send_serial_data('?');
     send_serial_data(';');
     Delay_ms(100);
}

void msmt_str()
{
      //MSV?:S01;
     send_serial_data('M');
     send_serial_data('S');
     send_serial_data('V');
     send_serial_data('?');
     send_serial_data(';');
     send_serial_data('S');
     send_serial_data('0');
     send_serial_data('1');
     send_serial_data(';');
}