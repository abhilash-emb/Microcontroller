    // LCD module connections
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RB0_bit;
sbit LCD_D5 at RB1_bit;
sbit LCD_D6 at RB2_bit;
sbit LCD_D7 at RB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;
// End LCD module connections

unsigned char calc_even_parity(unsigned char rxtx_data);
void send_serial_data(unsigned char tx_data);
void send_start_str();
void msmt_str();
void Soft_UART_Send_Str(char *);

volatile unsigned char lv_data_rcvd;

unsigned char rx9d_data;
unsigned char rx_data;
unsigned char lv_err;
unsigned char resp_data[8];
unsigned char char_rcv_cnt;
unsigned char chck_recv_data;
unsigned int  wt_disp;


void interrupt()
{
     if(PIR1.RCIF) {
          //checking for error
          if(RCSTA.FERR) {
              rx_data        = RCREG;
          }
          else if(RCSTA.OERR) {
              RCSTA.CREN     = 0;
              RCSTA.CREN     = 1;
          }
          // data recieved
          else {
              rx9d_data                 = RCSTA.RX9D;
              resp_data[char_rcv_cnt++] = RCREG;
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

     TRISC.F6    = 1;
     TRISC.F7    = 1;
     SPBRG       = 51;          // 9600 baud for 8Mhz clock
     TXSTA       = 0x64;
     RCSTA       = 0xD0;
     INTCON.GIE  = 1;
     INTCON.PEIE = 1;
     PIE1.RCIE   = 1;


     Lcd_Init();                        // Initialize LCD
     Lcd_Cmd(_LCD_CLEAR);               // Clear display
     Lcd_Cmd(_LCD_CURSOR_OFF);          // Cursor off
     Lcd_Out(1,2,"START");
     Delay_ms(1000);
     
     error = Soft_UART_Init(&PORTD, 7, 6, 9600, 0);
     if(error ==0)
          Lcd_Out(2,2,"0");
     else if(error == 1)
          Lcd_Out(2,2,"1");
     else
         Lcd_Out(2,2,"2");
     
     Delay_ms(100);

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

     char_rcv_cnt = 0;
     //Delay_ms(1000);
     Lcd_Out(2,14,"Kg");

     while(1) {
         msmt_str();
         //Soft_UART_Init(&PORTD, 7, 6, 9600, 0);
         Delay_ms(100);
         //INTCON.GIE  = 0;
         Soft_UART_Write('A');
         Soft_UART_Write('B');
         Soft_UART_Write('C');
         //INTCON.GIE  = 1;
         //Soft_UART_Send_Str("got");
         Delay_ms(100);
         /*if(resp_data[0] == 0x00 && resp_data[4] == 0x0D && resp_data[5] == 0x0A) {

             wt_data |= resp_data[1];

             wt_data |= resp_data[2] << 8;
             wt_data |= resp_data[3] << 16;

             wt_data -= zero_val;
             //wt_disp /= 33.33;

             Lcd_Out(2,1,"            ");
             if(wt_data > 0) {

                 LongToStr(wt_data, txt);
                 Lcd_Out(2,1,txt);
                 Soft_UART_Send_Str("got");
                 if(wt_disp > 49) {
                     IntToStr(wt_disp, txt);
                     Lcd_Out(2,1,"          ");
                     Lcd_Out(2,1,txt);
                 }
                 wt_data=0;
             }
             else {
                 Lcd_Out(2,1,"0");
             }
         } */
     }
     /*while(char_rcv_cnt < 5) {
         if(lv_data_rcvd) {
             lv_data_rcvd = 0;
             if(calc_even_parity(resp_data[char_rcv_cnt]) != rx9d_data)
                 Lcd_Out(1,2,"Parity Error");
              resp_data[char_rcv_cnt++] = rx_data;
         }
     } */

     //Lcd_Chr(2,2,char_rcv_cnt+0x30);
     /*if(resp_data[0] == 0x00 && resp_data[4] == 0x0D && resp_data[5] == 0x0A) {
          if((resp_data[2] == 0x09))
               Lcd_Out(2,2," 0 kg");
          else
                Lcd_Out(2,2," 50 kg");
          if((resp_data[2] == 0x11))
               Lcd_Out(2,2," 50 kg");
          if((resp_data[2] == 0x13))
               Lcd_Out(2,2," 80 kg");
     }*/

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
     TXSTA.TX9D = calc_even_parity(tx_data);
     TXREG      = tx_data;
     while(!TXSTA.TRMT);
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

void Soft_UART_Send_Str(char *str)
{
     while(*str != '\0')
     {   
         Soft_UART_Write(*str);
         str++;
     }
     Soft_UART_Write(0x0A);
     Soft_UART_Write(0x0D);
}