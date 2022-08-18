/* to be implemented
1. trailing zero removal
2. invalid or no entry display for empty eeprom memory locations        - done
3. incorporate bill no. and store it in memory 16-bit                   - tbd in print func
4. incorporate customer name and store it in memory                     - tbd in print func
5. replace goto with while                                              - not working will change later
6. read product and keep it in an array for printing                    - done */

/* Notes
1. Do not use RA4 as it it mux with Timer0 which is used to update RTC time
2. For this processor default project settings needs to be change for osc frequency   HS osc medium 4-16Mhz
3. EEPROM data details
   byte        function
   1-2         delimiter
   3           product location       *for verification purpose only*
   4-5         product code
   6-9         product price
   10-13       product consumption
   14-17       product net sale
   18-27       product name
   28-46       future use
   47-48       delimiter
End */

#include "built_in.h"                                   // for lo hi higher and highest

// bill no and customer info addresses
#define  BILL_NO_ADD               592
#define  CUST_INFO_L1              608
#define  CUST_INFO_L2              640

//EEPROM Addresses for stored items
#define ITEM_START_ADD             64
#define ITEM_MEM_CONST             48

// constant added for address resolution
#define LOC_PROD_LOC               2
#define LOC_PROD_CODE              3
#define LOC_PROD_RATE              5
#define LOC_PROD_CONS              9
#define LOC_PROD_NS                13
#define LOC_PROD_NAME              17
#define LOC_PROD_EDL               46

// ASCII character A address
#define ASCII_A                    65
#define PROD_NOT_SELECTED          33
#define PNS_LCD_DONE               55

// LCD module connections
sbit LCD_RS at RD1_bit;
sbit LCD_EN at RD0_bit;
sbit LCD_D4 at RD2_bit;
sbit LCD_D5 at RD3_bit;
sbit LCD_D6 at RD4_bit;
sbit LCD_D7 at RD5_bit;

sbit LCD_RS_Direction at TRISD1_bit;
sbit LCD_EN_Direction at TRISD0_bit;
sbit LCD_D4_Direction at TRISD2_bit;
sbit LCD_D5_Direction at TRISD3_bit;
sbit LCD_D6_Direction at TRISD4_bit;
sbit LCD_D7_Direction at TRISD5_bit;
// End LCD module connections

char  g_lv_intr_ind;
char  g_key_prs;                       // key press value
char  g_lv_kp;                         // key pressed or not
char  g_prod_sel_st;                   // product selected or not

char  g_lv_td_update;                  // for lcd update of every minute
char  g_t_sec;                         // 5 second update
char  g_td_cnt;                        // array index for time & date setting via LCD

char  g_st_spk_prs;                    // 0-default 1-set code 2-change price 4-record 8-date & time

char  g_lv_del_rec;                    // for deleting records

char  g_cp_cnt;                        // counter for number of change price pressed
char  g_cp_prod_loc;                   // change price product location
char  g_cp_prod_rate_idx;              // array index for new price product digits
char  g_cp_new_pr_arr[7] = {0};        // for new price entered

char  g_sc_prod_loc;                   // set code product location
char  g_sc_ch_inc_cnt;                 // counter for mobile keypad
char  g_sc_cnt;                        // counter for number of times set code pressed
char  g_sc_prod_ch_idx;                // array index where entered character for product name has to be stored
char  g_sc_code_ch_idx;                // array index where entered character for product code has to be stored
char  g_sc_curr_ch;                    // current character entered by user
char  g_sc_prod_code[2] = {0};               // array for set code prod code
char  g_sc_prod_name[11] = {0};              // set code product name

char  g_pr_prod_name[11] = {0} ;              // prod name for printing
char  g_pr_prod_qty[8] = {0};                // prod qty for printing
char  g_pr_prod_rate[9] = {0};               // prod rate for printing            Note : don't change sizeof g_pr_prod_rate, g_pr_prod_amt and g_wt_arr_pr keep it as 9
char  g_pr_prod_amt[9] = {0};                // prod amount for printing g_pr_prod_amount
char  g_pr_prod_total[12] = {0};              // prod total for printing
char  g_pr_bill_no[7] = {0};                 // bill number for printing
char  g_pr_flag;
char  g_rec_cons[9] = {0};                   // prod consumption for record printing
char  g_rec_net_total[9] = {0};              // prod net total for record printing

char  g_prod_loc;                            // product location

char  g_wt_arr_pr[9] = {0};           // weight array processed (with KG)
char  g_time_date[20] = {0};                 // time & date array for LCD display of time and date
char  g_prod_code[2] = {0};                  // make it a local variable if required
char  g_lcd_wt_ch_cnt;                       // for wt lcd clear
char  g_lcd_tot_ch_cnt;                      // for totoal lcd clear

float g_prod_rate;                           // product rate for current selected product
double g_prod_wt;
double g_prod_net_total;                      // net total
double g_prod_total;                          // product total for individual item
double g_total_sale;

const code char cc_customer_line_1[]           = "ABC Corp";
const code char cc_customer_line_2[]           = "Bangalore";
const code char cc_bill_no[]                   = "Bill No : ";
const code char cc_g_thank_msg[]               = "Thank You Visit Again";
const code char cc_space_arr[]                 = "                   ";
const code char cc_item_arr[]                  = "ITEM";
const code char cc_qty_prc_amt_arr[]           = "QTY  PRICE  AMOUNT";
const code char cc_kg_rs_arr[]                 = "Kg     Rs      Rs";
const code char cc_rec_kg_rs_arr[]             = "Kg         Rs";
const code char cc_rec_con_sale[]              = "CONSUMED   NET SALE";
const code char cc_total_rs[]                  = "TOTAL Rs ";
const code unsigned short  g_td_cur_pos[10] = {1,2,4,5,9,10,15,16,18,19};   // lcd positions

void init_g_var();
void func_zero();
void func_num_key(unsigned short index);
void func_setcode();
void func_changeprice();
void func_print();
void func_next();
void func_record();
void func_datetime();
void get_timedate();
void set_timedate();
void update_time();
void key_prs_run_mode(unsigned short index);
void key_prs_set_code(unsigned short index);
void key_prs_chg_prc(unsigned short index);
void key_prs_date_time(unsigned short index);
unsigned char BCD2UpperCh(unsigned char bcd);
unsigned char BCD2LowerCh(unsigned char bcd);
unsigned short read_ds1307(unsigned short address);
void write_ds1307(unsigned short address, unsigned short w_data);
//void error(char *str);
//void lcd_clear(unsigned short row);
void read_scale();
void running_mode();
void FloatToStringNew(char *str, float f, char size);
void itoa(long n, char s[]);
void reverse(char s[]);
void print_header();                                        // Printing header
void print_next();                                          // Printing next item
void print_footer();                                        // Printing footer
void print_record_header();                                        // Printing record
void print_record_item();
void print_record_footer();
void serial_print(char *);                                  // Sending string to serial port
void cancel_bold();
void set_bold();                                            // Set BOLD printing
void cancel_double();
void center_align();                                        // Set alignment to CENTER
void left_align();                                          // Set alignment to LEFT
void reset_printer();
void line_print();                                       // RESET printer settings
void print_to_memory();

// for 4x4 keypad
void interrupt(){
     // signal change interrupt
     g_lv_intr_ind = 1;
     if(RBIF_bit){


       INTCON.GIE = 0; // Global interrupt OFF
       Delay_ms(10); // 10 ms delay for key debouncing

       g_key_prs = 0; // clear variable g_key_prs

       if (RB7_bit == 0) {
          PORTB = 0b00001110;
          if (RB7_bit == 0) {
             g_key_prs = 15;
             goto STEP2;
          }
          PORTB = 0b00001101;
          if (RB7_bit == 0) {
             g_key_prs = 7;
             goto STEP2;
          }
          PORTB = 0b00001011;
          if (RB7_bit == 0) {
             g_key_prs = 4;
             goto STEP2;
          }
          PORTB = 0b00000111;
          if (RB7_bit == 0) {
             g_key_prs = 1;
             goto STEP2;
          }
       }

       if (RB6_bit == 0) {
          PORTB = 0b00001110;
          if (RB6_bit == 0) {
             g_key_prs = 10;
             goto STEP2;
          }
          PORTB = 0b00001101;
          if (RB6_bit == 0) {
             g_key_prs = 8;
             goto STEP2;
          }
          PORTB = 0b00001011;
          if (RB6_bit == 0) {
             g_key_prs = 5;
             goto STEP2;
          }
          PORTB = 0b00000111;
          if (RB6_bit == 0) {
             g_key_prs = 2;
             goto STEP2;
          }
       }

       if (RB5_bit == 0) {
          PORTB = 0b00001110;
          if (RB5_bit == 0) {
             g_key_prs = 14;
             goto STEP2;
          }
          PORTB = 0b00001101;
          if (RB5_bit == 0) {
             g_key_prs = 9;
             goto STEP2;
          }
          PORTB = 0b00001011;
          if (RB5_bit == 0) {
             g_key_prs = 6;
             goto STEP2;
          }
          PORTB = 0b00000111;
          if (RB5_bit == 0) {
             g_key_prs = 3;
             goto STEP2;
          }
       }

       if (RB4_bit == 0) {
          PORTB = 0b00001110;
          if (RB4_bit == 0) {
             g_key_prs = 13;
             goto STEP2;
          }
          PORTB = 0b00001101;
          if (RB4_bit == 0) {
             g_key_prs = 12;
             goto STEP2;
          }
          PORTB = 0b00001011;
          if (RB4_bit == 0) {
             g_key_prs = 11;
             goto STEP2;
          }
          PORTB = 0b00000111;
          if (RB4_bit == 0) {
             g_key_prs = 16;
             goto STEP2;
          }
       }

       STEP2:

             if ((RB7_bit == 1)&(RB6_bit == 1)&(RB5_bit==1)&(RB4_bit==1)) {
                Delay_ms(10);
             }
             else goto STEP2;
             g_lv_kp = 1;

       PORTB = 0;
       RBIF_bit = 0; // Reset the interrupt flag
       RBIE_bit = 1; // RB Change interrupt ON
       GIE_bit = 1; // Global interrupts ON

     }
     // 5 sec timer
     if (TMR0IF_bit){

        GIE_bit = 0;
        TMR0IF_bit = 0;
        TMR0H         = 0x67;
        TMR0L         = 0x6A;
        if(++g_t_sec > 14) {
            g_lv_td_update = 1;
            g_t_sec = 0;
        }
        GIE_bit = 1;
     }
}

// main program
void main()
{
    init_g_var();

    ANSELB        = 0x00;               // making port b, c and port d as digital
    ANSELD        = 0x00;
    ANSELC        = 0x00;
    UART1MD_bit   = 0;                  // enable uart communication
    MSSP1MD_bit   = 0;                  // enable I2C communication
    IOCB          = 0xF0;
    PORTB         = 0x00;
    TRISB         = 0xF0;               // RB 0 ~ 3 OUTPUTS, RB4 ~ 7 INPUTS
    T0CON         = 0x87;
    TMR0H         = 0x67;               // timer for 2 sec
    TMR0L         = 0x6A;
    RBIE_bit      = 1;                  // enable signal change interrupt
    TMR0IE_bit    = 1;                  // enable timer0 interrupt

    Lcd_Init();                         // Initialize LCD
    Lcd_Cmd(_LCD_CLEAR);                // Clear display
    Lcd_Cmd(_LCD_CURSOR_OFF);           // Cursor off
    Lcd_Out(1,6,"THOMSONS");            // Write text in first row
    Delay_ms(1000);

    UART1_Init(9600);                   //initialize Serial Port
    Delay_ms(100);

    I2C1_Init(100000);                  // for RTC
    Delay_ms(100);

    UART1_Write(0x1B);                  //initialization for printer
    UART1_Write(0x40);

    Lcd_Out(2,5,"Starting Up");
    Lcd_Out(3,3,"Please Wait. . .");
    Delay_ms(1000);

    GIE_bit       = 1;                  // enable global interrupt

    while(1) {

            if(g_lv_kp) {
                 switch (g_key_prs) {
                   case  1: func_num_key(1);    break;
                   case  2: func_num_key(2);    break;
                   case  3: func_num_key(3);    break;
                   case  4: func_num_key(4);    break;
                   case  5: func_num_key(5);    break;
                   case  6: func_num_key(6);    break;
                   case  7: func_num_key(7);    break;
                   case  8: func_num_key(8);    break;
                   case  9: func_num_key(9);    break;
                   case 10: func_zero();        break;
                   case 11: func_changeprice(); break;
                   case 12: func_next();        break;
                   case 13: func_print();       break;
                   case 14: func_record();      break;
                   case 15: func_datetime();    break;
                   case 16: func_setcode();     break;
                 }
                 g_lv_kp = 0;
            }

            if(!g_st_spk_prs)
                 running_mode();

            if(g_lv_td_update && !g_st_spk_prs) {
                 update_time();
            }
    }
}

//num key press
void func_num_key(unsigned short index)
{
     switch (g_st_spk_prs) {
            //default case
            case 0: {
                 key_prs_run_mode(index);
                 break;
            }
            // set code
            case 1: {
                 key_prs_set_code(index);
                 break;
            }
            // change price
            case 2: {
                 key_prs_chg_prc(index);
                 break;
            }
            // record
            case 4: {
                 //do nothing
                 break;
            }
            // date & time
            case 8: {
                 key_prs_date_time(index);
                 break;
            }
            //error
            default: {
                //error("error keypress");
                break;
            }
     }
}

void key_prs_run_mode(unsigned short index)
{
     unsigned short i;
     unsigned int kprm_item_add;
     char kprm_arr_rate[7];

     g_prod_sel_st         = 0;
     g_st_spk_prs          = 0;
     Lcd_Out(1,1,"                    ");
     Lcd_Out(2,1,"                    ");
     Lcd_Out(3,1,"                    ");
     g_prod_loc            =  index;
     g_prod_code[0]        =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_CODE);
     g_prod_code[1]        =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_CODE+1);

     if((g_prod_code[0] < 65 || g_prod_code[0] > 90) || (g_prod_code[1] < 65 || g_prod_code[1] > 90)) {
           Lcd_Out(1,1,"Invalid Product !");
           Delay_ms(1000);
           g_prod_sel_st = PROD_NOT_SELECTED;
           return;
     }

     Lo(g_prod_rate)       =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE);
     Hi(g_prod_rate)       =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE+1);
     Higher(g_prod_rate)   =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE+2);
     Highest(g_prod_rate)  =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE+3);

     kprm_item_add = ITEM_START_ADD + ((index)*ITEM_MEM_CONST) + LOC_PROD_NAME;
     for(i = 0; i < 11; i++)
         if((g_pr_prod_name[i] = EEPROM_Read(kprm_item_add+i)) == '\0')
             break;

     FloatToStringNew(kprm_arr_rate, g_prod_rate, 2);

     Lcd_Chr(1,1,0x30+index);

     Lcd_Chr(1,5,g_prod_code[0]);
     Lcd_Chr(1,6,g_prod_code[1]);
     Lcd_Out(2,1,"Rate(Kg)");
     Lcd_Out(2,11,kprm_arr_rate);

     strcpy(g_pr_prod_rate,kprm_arr_rate);
     Lcd_Out(3,1,"Total(Rs)");
     Lcd_Out(1,18,"Kg");

}

void key_prs_set_code(unsigned short index)
{
     if(g_sc_cnt == 0) {
          Lcd_Chr(3,1,0x30+index);
          Delay_ms(1000);
          Lcd_Cmd(_LCD_CLEAR);
          /*
          Lcd_Out(1,1,"                    ");
          Lcd_Out(2,1,"                    ");
          Lcd_Out(3,1,"                    ");
          */

          Lcd_Out(1,1,"Enter Product Name");
          Lcd_Cmd(_LCD_SECOND_ROW);
          g_sc_prod_loc = index;
          g_sc_cnt++;
     }
     else {//if(g_sc_cnt == 1 || g_sc_cnt == 2) {

          if((index == 9) && (g_sc_ch_inc_cnt == 2))
               g_sc_ch_inc_cnt = 0;
          else if(g_sc_ch_inc_cnt == 3)
               g_sc_ch_inc_cnt = 0;

          g_sc_curr_ch = ASCII_A +(index-1)*3 + g_sc_ch_inc_cnt;
          Lcd_Chr_Cp(g_sc_curr_ch);
          Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
          g_sc_ch_inc_cnt++;
     }
}

void key_prs_chg_prc(unsigned short index)
{
     unsigned short i;
     float kpcp_cp_prod_rate;
     char  kpcp_arr_rate[7];

     if(g_cp_cnt == 0) {
         Lcd_Out(1,1,"                    ");
         Lcd_Out(2,1,"                    ");
         Lcd_Out(3,1,"                    ");
         Lcd_Chr(1,5,EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_CODE));
         Lcd_Chr(1,6,EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_CODE+1));
         Lo(kpcp_cp_prod_rate)       =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE);
         Hi(kpcp_cp_prod_rate)       =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE+1);
         Higher(kpcp_cp_prod_rate)   =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE+2);
         Highest(kpcp_cp_prod_rate)  =  EEPROM_Read(ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_RATE+3);

         FloatToStringNew(kpcp_arr_rate, kpcp_cp_prod_rate, 2);

         Lcd_Chr(1,1,0x30+index);

         Lcd_Out(2,1,"Current Rate ");
         Lcd_Out(2,14,kpcp_arr_rate);
         Lcd_Out(3,1,"New Rate ");
         Lcd_Out(3,11,"000.00");
         for(i=0;i<6;i++)
              Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
         g_cp_prod_loc = index;
         g_cp_cnt++;

     } else if (g_cp_cnt == 1) {
         if(g_cp_prod_rate_idx < 6)
             Lcd_Chr_Cp(0x30+index);
             g_cp_new_pr_arr[g_cp_prod_rate_idx++] = index+'0';
             if(g_cp_prod_rate_idx == 3) {
                  Lcd_Chr_Cp('.');
                  g_cp_new_pr_arr[g_cp_prod_rate_idx++] = '.';
             }
     }

}

void key_prs_date_time(unsigned short index)
{
     g_time_date[g_td_cnt] = index;
     Lcd_Chr(4, g_td_cur_pos[g_td_cnt], 0x30+index);
     g_td_cnt++;
}

//reading and processing scale output
void read_scale()
{
     char i,rx_data,wt_true = 1;
     
     while(1) {
         while(!UART1_Data_Ready());
         rx_data  = UART1_Read();
         if(rx_data == 0x0D) {
             for(i = 0; rx_data != 0x0A; i++) {
                 while(!UART1_Data_Ready());
                 rx_data  = UART1_Read();
                 if((rx_data >= '0' || rx_data <= '9') || rx_data == '.' || rx_data == ' ' || rx_data == '+' || rx_data == '-')
                     g_wt_arr_pr[i] = rx_data;
                 else {
                     wt_true = 0;
                     break;
                 }
             }
             if(wt_true) {
                 g_wt_arr_pr[--i] = '\0';
                 break;
             }
         }
     }

     if(wt_true) {
         while((g_wt_arr_pr[0] == '0' && g_wt_arr_pr[1] != '.') || (g_wt_arr_pr[0] == ' ')||(g_wt_arr_pr[0] == '-'))
                for(i = 0; g_wt_arr_pr[i] != '\0'; i++)
                      g_wt_arr_pr[i] = g_wt_arr_pr[i+1];
         g_wt_arr_pr[i] = '\0';
     }
}

// default running state
void running_mode()
{
     char  rm_arr_total[8];

     if(!g_prod_sel_st) {
      //READ_SCALE:
          RBIE_bit = 0;
          TMR0IE_bit = 0;
          read_scale();
          g_prod_wt = atof(g_wt_arr_pr);
          g_prod_total = g_prod_wt * g_prod_rate;
          FloatToStringNew(rm_arr_total, g_prod_total, 2);
          /*Lcd_Out(1,11,"       Kg ");
          Lcd_Out(3,11,"          "); */
          if(g_lcd_wt_ch_cnt != strlen(g_wt_arr_pr))
              Lcd_Out(1,11,"       Kg ");
          g_lcd_wt_ch_cnt = strlen(g_wt_arr_pr);
          
          if(g_lcd_tot_ch_cnt != strlen(rm_arr_total))
              Lcd_Out(3,11,"          ");
          g_lcd_tot_ch_cnt = strlen(rm_arr_total);
          //if(g_lv_intr_ind) { g_lv_intr_ind = 0; goto READ_SCALE;}
          Lcd_Out(1,11,g_wt_arr_pr);
          Lcd_Out(3,11,rm_arr_total);
          strcpy(g_pr_prod_amt,rm_arr_total);
          RBIE_bit = 1;
          TMR0IE_bit = 1;

     }
     else if(g_prod_sel_st == PROD_NOT_SELECTED) {
          Lcd_Out(1,1,"                    ");
          Lcd_Out(2,1,"                    ");
          Lcd_Out(3,1,"                    ");
          Lcd_Cmd(_LCD_CURSOR_OFF);
          Lcd_Out(1,1,"Please select item");
          update_time();
          g_prod_sel_st = PNS_LCD_DONE;
     }
     else if(g_prod_sel_st == PNS_LCD_DONE) {
          // do nothing
     }
}

void func_zero()
{
      switch (g_st_spk_prs) {
            //default case
            case 0: {
                key_prs_run_mode(0);
                break;
            }
            // set code
            // special function
            case 1: {
                 if(g_sc_cnt) {
                     g_sc_ch_inc_cnt = 0;
                     if(g_sc_cnt == 1)
                          g_sc_prod_name[g_sc_prod_ch_idx++] = g_sc_curr_ch;
                     else if (g_sc_cnt == 2)
                          g_sc_prod_code[g_sc_code_ch_idx++] = g_sc_curr_ch;
                     Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                 }
                 else
                     key_prs_set_code(0);
                 break;
            }
            // change price
            case 2: {
                 key_prs_chg_prc(0);
                 break;
            }
            //record
            //special function
            case 4: {
                 if(!g_lv_del_rec) {
                     Lcd_Out(1,1,"                    ");
                     Lcd_Out(2,1,"                    ");
                     Lcd_Out(1,2,"Delete Records ?");
                     g_lv_del_rec = 1;
                 }
                 else {
                     running_mode();
                     g_lv_del_rec = 0;
                 }
                 break;
            }
            //date & time
            case 8: {
                 key_prs_date_time(0);
                 break;
            }
            default:
                 break;
      }

}

void func_setcode()
{
     unsigned short i;
     unsigned int sc_prod_add;

     if(g_prod_sel_st == PNS_LCD_DONE)
          return;

     switch (g_st_spk_prs) {
            //default case
            case 0: {
                Lcd_Cmd(_LCD_CLEAR);                // Clear display
                Lcd_Out(1,1,"Enter Product");
                Lcd_Out(2,1,"Location");
                Lcd_Cmd(_LCD_THIRD_ROW);
                Lcd_Cmd(_LCD_BLINK_CURSOR_ON);
                g_st_spk_prs = 1;
                g_sc_prod_ch_idx = 0;
                break;
            }
            // set code
            case 1: {
                /*1 - for prod name 2 - for prod code*/
                if(g_sc_cnt == 1) {
                     g_sc_prod_name[g_sc_prod_ch_idx++] = g_sc_curr_ch;
                     g_sc_prod_name[g_sc_prod_ch_idx] = '\0';
                     g_sc_cnt++;
                     Lcd_Out(1,1,"                    ");
                     Lcd_Out(2,1,"                    ");
                     Lcd_Out(1,1,"Enter Product Code");
                     Lcd_Cmd(_LCD_SECOND_ROW);
                     g_sc_prod_ch_idx = 0;
                     g_sc_ch_inc_cnt  = 0;
                }
                else if(g_sc_cnt == 2) {
                     g_sc_prod_code[g_sc_code_ch_idx++] = g_sc_curr_ch;
                     Lcd_Cmd(_LCD_CURSOR_OFF);
                     sc_prod_add = ITEM_START_ADD + (g_sc_prod_loc * ITEM_MEM_CONST);
                     EEPROM_Write(sc_prod_add+LOC_PROD_LOC,g_sc_prod_loc);
                     EEPROM_Write(sc_prod_add+LOC_PROD_CODE,g_sc_prod_code[0]);
                     EEPROM_Write(sc_prod_add+LOC_PROD_CODE+1,g_sc_prod_code[1]);
                     for(i = 0; i <= 10; i++) {
                          if(g_sc_prod_name[i] == '\0')
                                break;
                          EEPROM_Write(sc_prod_add+LOC_PROD_NAME+i,g_sc_prod_name[i]);
                     }
                     for(; i < 10; i++) {
                          EEPROM_Write(sc_prod_add+LOC_PROD_NAME+i,' ');
                     }
                     EEPROM_Write(sc_prod_add+LOC_PROD_NAME+i,'\0');

                     Lcd_Out(1,1,"                    ");
                     Lcd_Out(2,1,"                    ");
                     Lcd_Out(3,1,"                    ");
                     Lcd_Out(1,3,"Product Saved");
                     Delay_ms(1000);
                     g_sc_code_ch_idx = 0;
                     g_sc_ch_inc_cnt = 0;
                     g_st_spk_prs = 0;
                     g_sc_cnt = 0;
                     g_prod_sel_st = PROD_NOT_SELECTED;
                }
                break;
            }
            case 2:
            case 4:
            case 8:
            default:
                 break;
   }

}

void func_changeprice()
{
     unsigned int cp_prod_rate_loc;
     float        cp_new_price;

     if(g_prod_sel_st == PNS_LCD_DONE)
          return;

     switch (g_st_spk_prs) {
            //default case
            case 0: {
                Lcd_Cmd(_LCD_CLEAR);                // Clear display
                Lcd_Out(1,1,"Enter Product");
                Lcd_Out(2,1,"Location");
                Lcd_Cmd(_LCD_THIRD_ROW);
                Lcd_Cmd(_LCD_BLINK_CURSOR_ON);
                g_st_spk_prs = 2;
                break;
            }
            // change price
            case 2: {
                Lcd_Cmd(_LCD_CURSOR_OFF);
                g_cp_new_pr_arr[6] = '\0';
                cp_new_price = atof(g_cp_new_pr_arr);
                cp_prod_rate_loc = ITEM_START_ADD + (g_cp_prod_loc * ITEM_MEM_CONST) + LOC_PROD_RATE;
                EEPROM_Write(cp_prod_rate_loc,   Lo(cp_new_price));
                EEPROM_Write(cp_prod_rate_loc+1, Hi(cp_new_price));
                EEPROM_Write(cp_prod_rate_loc+2, Higher(cp_new_price));
                EEPROM_Write(cp_prod_rate_loc+3, Highest(cp_new_price));
                Lcd_Out(1,1,"                    ");
                Lcd_Out(2,1,"                    ");
                Lcd_Out(3,1,"                    ");
                Lcd_Out(1,3,"Price Saved");
                g_st_spk_prs          = 0;
                g_cp_prod_loc         = 0;
                g_cp_prod_rate_idx    = 0;
                g_cp_cnt              = 0;
                g_prod_sel_st = PROD_NOT_SELECTED;
                Delay_ms(1000);
                break;
            }
            case 1:
            case 4:
            case 8:  break;

            default:
                 break;
   }
}

void func_print()
{
     if(g_prod_sel_st == PNS_LCD_DONE)
          return;

     switch (g_st_spk_prs) {
            //default case
            case 0: {
                if(g_pr_flag == 0)
                     print_header();
                print_next();
                print_to_memory();
                print_footer();
                g_pr_flag = 0;
                break;
            }
            // set code
            case 1:
            case 2:
            case 4:
            case 8: break;

            default:
                 break;
   }
}

void func_next()
{
     if(g_prod_sel_st == PNS_LCD_DONE)
          return;

     switch (g_st_spk_prs) {
            //default case
            case 0: {
                // printing the receipt
                 if(g_pr_flag == 0) {
                     g_pr_flag = 1;
                     print_header();
                 }
                 print_next();

                 // writing to memory
                 print_to_memory();
                 break;
            }
            // set code
            case 1:
            case 2:
            case 4:
            case 8: {
                 init_g_var();
                 running_mode();
                 break;
            }
            default:
                 break;
   }
}

void func_record()
{
      unsigned int rec_add_items;
      double rec_prod_ns;
      double rec_prod_cons;
      unsigned short i,j;
      char rec_eeprom_data = 0;
      g_total_sale = 0.0;
     if(g_prod_sel_st == PNS_LCD_DONE)
          return;

      switch (g_st_spk_prs) {
            // default case
            case 0: {
                 if(!g_pr_flag) {
                   Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,3,"Print Records ?");
                   g_st_spk_prs = 4;
                 }
                 break;
            }
            // records
            case 4: {
                 if(g_lv_del_rec) {
                     Lcd_Out(1,1,"                    ");
                     Lcd_Out(2,1,"                    ");
                     Lcd_Out(1,2,"Deleting Records");
                     Lcd_Out(2,2,"Please Wait. . .");
                     g_lv_del_rec = 0;
                     rec_add_items = ITEM_START_ADD;

                     for(i = 0; i < 10; i++) {
                           EEPROM_Write(rec_add_items+LOC_PROD_CONS,0x00);
                           EEPROM_Write(rec_add_items+LOC_PROD_CONS+1,0x00);
                           EEPROM_Write(rec_add_items+LOC_PROD_CONS+2,0x00);
                           EEPROM_Write(rec_add_items+LOC_PROD_CONS+3,0x00);
                           EEPROM_Write(rec_add_items+LOC_PROD_NS,0x00);
                           EEPROM_Write(rec_add_items+LOC_PROD_NS+1,0x00);
                           EEPROM_Write(rec_add_items+LOC_PROD_NS+2,0x00);
                           EEPROM_Write(rec_add_items+LOC_PROD_NS+3,0x00);
                           rec_add_items += ITEM_MEM_CONST;
                     }
                     Delay_ms(1000);
                 }
                 else {

                     Lcd_Out(1,1,"                    ");
                     Lcd_Out(2,1,"                    ");
                     Lcd_Out(1,2,"Printing Records");
                     Lcd_Out(2,2,"Please Wait. . .");
                     //call print_records function here
                     print_record_header();
                     rec_add_items = ITEM_START_ADD;

                     for(i = 0; i < 10; i++) {

                           g_prod_code[0]        =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_CODE);
                           g_prod_code[1]        =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_CODE+1);

                           if((g_prod_code[0] < 65 || g_prod_code[0] > 90) || (g_prod_code[1] < 65 || g_prod_code[1] > 90))
                              continue;

                           for(j = 0; j < 11; j++)
                             if((g_pr_prod_name[j] = EEPROM_Read(ITEM_START_ADD + ((i)*ITEM_MEM_CONST) + LOC_PROD_NAME + j)) == '\0')
                                  break;

                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_NS)) != 0xFF)
                               Lo(rec_prod_ns)         = rec_eeprom_data;
                           else
                               Lo(rec_prod_ns)         = 0x00;

                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_NS+1)) != 0xFF)
                               Hi(rec_prod_ns)         = rec_eeprom_data;
                           else
                               Hi(rec_prod_ns)         = 0x00;

                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_NS+2)) != 0xFF)
                               Higher(rec_prod_ns)         = rec_eeprom_data;
                           else
                               Higher(rec_prod_ns)         = 0x00;

                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_NS+3)) != 0xFF)
                               Highest(rec_prod_ns)         = rec_eeprom_data;
                           else
                               Highest(rec_prod_ns)         = 0x00;

                           // Consumption read
                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_CONS)) != 0xFF)
                               Lo(rec_prod_cons)         = rec_eeprom_data;
                           else
                               Lo(rec_prod_cons)         = 0x00;

                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_CONS+1)) != 0xFF)
                               Hi(rec_prod_cons)         = rec_eeprom_data;
                           else
                               Hi(rec_prod_cons)         = 0x00;

                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_CONS+2)) != 0xFF)
                               Higher(rec_prod_cons)         = rec_eeprom_data;
                           else
                               Higher(rec_prod_cons)         = 0x00;

                           if((rec_eeprom_data =  EEPROM_Read(ITEM_START_ADD + (i * ITEM_MEM_CONST) + LOC_PROD_CONS+3)) != 0xFF)
                               Highest(rec_prod_cons)         = rec_eeprom_data;
                           else
                               Highest(rec_prod_cons)         = 0x00;

                           FloatToStringNew(g_rec_cons, rec_prod_cons, 3);
                           FloatToStringNew(g_rec_net_total,rec_prod_ns,2);

                           print_record_item();

                           rec_add_items += ITEM_MEM_CONST;
                           Delay_Ms(20);
                           g_total_sale += rec_prod_ns ;
                           
                     }
                     print_record_footer();
                     Delay_ms(1000);
                 }
                 g_pr_flag = 0;
                 g_st_spk_prs = 0;
                 g_prod_sel_st = PROD_NOT_SELECTED;
                 break;
            }

            //all other cases
            case 1:
            case 2:
            case 8: break;

            default:
                    break;
      }
}

void func_datetime()
{
     if(g_prod_sel_st == PNS_LCD_DONE)
          return;

      switch (g_st_spk_prs) {
            //default case
            case 0: {
                 Lcd_Cmd(_LCD_CLEAR);                // Clear display
                 Lcd_Out(1,3,"Set Date & Time");
                 Lcd_Out(2,1,"(DD/MM/20YY)");
                 Lcd_Out(2,14,"H/Min");
                 Lcd_Out(3,13,"(24hr)");
                 Lcd_Chr(4,3,'/');
                 Lcd_Chr(4,6,'/');
                 Lcd_Chr(4,7,'2');
                 Lcd_Chr(4,8,'0');
                 Lcd_Chr(4,17,':');
                 Lcd_Cmd(_LCD_FOURTH_ROW);
                 g_st_spk_prs = 8;
                 break;
            }
            case 8: {
                 Lcd_Cmd(_LCD_CURSOR_OFF);
                 set_timedate();
                 g_st_spk_prs = 0;
                 g_prod_sel_st = PROD_NOT_SELECTED;
                 Lcd_Cmd(_LCD_CLEAR);
                 Lcd_Out(1,1,"Date & Time Saved");
                 Delay_ms(1000);
                 break;
            }
            case 1:
            case 2:
            case 4: break;

            default:
                    break;
      }

}

/*void error(char *str)
{
      Lcd_Cmd(_LCD_CLEAR);                // Clear display
      Lcd_Out(1,1,str);                  // Write error in first row
} */

/*void lcd_clear(unsigned short row)
{
      Lcd_Out(row,1,"                    ");
} */

 // convert float to string one decimal digit at a time
 // assumes float is < 65536 and ARRAYSIZE is big enough
 // problem: it truncates numbers at size without rounding
 // str is a char array to hold the result, float is the number to convert
 // size is the number of decimal digits you want
 void FloatToStringNew(char *str, float f, char size)
 {
        char pos;  // position in string
        char len;  // length of decimal part of result
        long value;  // decimal digit(s) to convert
        pos = 0;  // initialize pos, just to be sure

        value = (long)f;  // truncate the floating point number
        itoa(value,str);  // this is kinda dangerous depending on the length of str
        // now str array has the digits before the decimal

        len = strlen(str);  // find out how big the integer part was
        pos = len;  // position the pointer to the end of the integer part
        str[pos++] = '.';  // add decimal point to string

        while( pos < (size + len + 1) )  // process remaining digits
        {
                f = f - (float)value;  // hack off the whole part of the number
                f *= 10;  // move next digit over
                value = (long)f;  // get next digit
                str[pos++] = value + '0';
        }
        str[pos] = '\0';
 }

 /* itoa:  convert n to characters in s */
 void itoa(long n, char s[])
 {
     int i=0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     s[i] = '\0';
     reverse(s);
 }

/* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 unsigned short read_ds1307(unsigned short address)
{
    unsigned short rtc_data;
    I2C1_Start();
    I2C1_Wr(0xd0);
    I2C1_Wr(address);
    I2C1_Repeated_Start();
    I2C1_Wr(0xd1);
    rtc_data = I2C1_Rd(0);
    I2C1_Stop();
    return(rtc_data);
}

void write_ds1307(unsigned short address,unsigned short w_data)
{
    I2C1_Start();
    I2C1_Wr(0xD0);
    I2C1_Wr(address);
    I2C1_Wr(w_data);
    I2C1_Stop();
}

unsigned char BCD2UpperCh(unsigned char bcd)
{
    return ((bcd >> 4) + '0');
}

unsigned char BCD2LowerCh(unsigned char bcd)
{
    return ((bcd & 0x0F) + '0');
}

// setting time and date to RTC
void set_timedate()
{
    unsigned short min;
    unsigned short hour;
    unsigned short date;
    unsigned short month;
    unsigned short year;


    g_time_date[0]&=0x0F;
    g_time_date[1]&=0x0F;
    date   = (g_time_date[0]*0x10) + g_time_date[1];

    g_time_date[2]&=0x0F;
    g_time_date[3]&=0x0F;
    month  = (g_time_date[2]*0x10) + g_time_date[3];

    g_time_date[4]&=0x0F;
    g_time_date[5]&=0x0F;
    year   = (g_time_date[4]*0x10) + g_time_date[5];

    g_time_date[6]&=0x0F;
    g_time_date[7]&=0x0F;
    hour   = (g_time_date[6]*0x10) + g_time_date[7];

    g_time_date[8]&=0x0F;
    g_time_date[9]&=0x0F;
    min    = (g_time_date[8]*0x10) + g_time_date[9];

    write_ds1307(0,0x80);                                            //Reset second to 0 sec. and stop Oscillator
    write_ds1307(1,min);                                             // write min
    write_ds1307(2,hour);                                            // write hour
    write_ds1307(4,date);                                            // write date
    write_ds1307(5,month);                                           // write month
    write_ds1307(6,year);                                            // write year
    write_ds1307(0,0x00);                                            //Reset second to 0 sec. and start Oscillator

    g_td_cnt = 0;                                                    // reset the counter
}

// get time and date to time & date array
void get_timedate()
{
    unsigned short minute;
    unsigned short hour;
    unsigned short date;
    unsigned short month;
    unsigned short year;

    minute=read_ds1307(1);                                            // read minute
    hour=read_ds1307(2);                                              // read hour
    date=read_ds1307(4);                                              // read date
    month=read_ds1307(5);                                             // read month
    year=read_ds1307(6);                                              // read year

    g_time_date[0] = BCD2UpperCh(date);
    g_time_date[1] = BCD2LowerCh(date);
    g_time_date[2] = '/';
    g_time_date[3] = BCD2UpperCh(month);
    g_time_date[4] = BCD2LowerCh(month);
    g_time_date[5] = '/';
    g_time_date[6] = '2';
    g_time_date[7] = '0';
    g_time_date[8] = BCD2UpperCh(year);
    g_time_date[9] = BCD2LowerCh(year);
    g_time_date[10] = ' ';
    g_time_date[11] = ' ';
    g_time_date[12] = BCD2UpperCh(hour);
    g_time_date[13] = BCD2LowerCh(hour);
    g_time_date[14] = ':';
    g_time_date[15] = BCD2UpperCh(minute);
    g_time_date[16] = BCD2LowerCh(minute);
    g_time_date[17] = 'A';
    g_time_date[18] = 'M';
    g_time_date[19] = '\0';

    if( g_time_date[12] == '0' && g_time_date[13] == '0') {
        g_time_date[12] = '1';
        g_time_date[13] = '2';
        g_time_date[17] = 'A';
        g_time_date[18] = 'M';
    } else if(g_time_date[12] == '1' && g_time_date[13] == '2') {
        g_time_date[17] = 'P';
        g_time_date[18] = 'M';
    } else if(g_time_date[12] == '2' && (g_time_date[13] == '0' || g_time_date[13] == '1')) {
        g_time_date[12] = g_time_date[12] - 2;
        g_time_date[13] = g_time_date[13] + 8;
        g_time_date[17] = 'P';
        g_time_date[18] = 'M';
    } else if((g_time_date[12] == '1' && g_time_date[13] > '2') || g_time_date[12] == '2' ) {
        g_time_date[12] = g_time_date[12] - 1;
        g_time_date[13] = g_time_date[13] - 2;
        g_time_date[17] = 'P';
        g_time_date[18] = 'M';
    }
}

//display current time
void update_time()
{
     g_lv_td_update = 0;
     get_timedate();
     Lcd_Out(4,1,"                    ");
     Lcd_Out(4,1,g_time_date);   // Write time in fourth row
}

// creating print header
void print_header()
{
     int pf_bill_no,i;

     cancel_double();
     set_bold();

     for(i=0;cc_customer_line_1[i] != '\0';i++)
         UART1_Write(cc_customer_line_1[i]);
     //serial_print("OSWAL BROTHERS");
     UART1_Write(0x0A);

     cancel_bold();
     left_align();

     for(i=0;cc_customer_line_2[i] != '\0';i++)
         UART1_Write(cc_customer_line_2[i]);
     //serial_print("Usha Ganj,Indore");
     UART1_Write(0x0A);

     line_print();
     //serial_print(g_line_print);
     UART1_Write(0x0A);

     Lo(pf_bill_no)  = EEPROM_Read(BILL_NO_ADD);
     Hi(pf_bill_no)  = EEPROM_Read(BILL_NO_ADD+1);

     if(pf_bill_no == 0xFFFF)
         pf_bill_no = 100;
     IntToStr(pf_bill_no, g_pr_bill_no);

     for(i=0;cc_bill_no[i] != '\0';i++)
         UART1_Write(cc_bill_no[i]);

     serial_print(g_pr_bill_no);
     UART1_Write(0x0A);

     pf_bill_no++;
     EEPROM_Write(BILL_NO_ADD ,Lo(pf_bill_no));
     EEPROM_Write(BILL_NO_ADD+1 ,Hi(pf_bill_no));

     line_print();
     UART1_Write(0x0A);

     // Item print
     for(i=0;i<5;i++)
         UART1_Write(cc_item_arr[i]);
         
     // Space print
     for(i=0;i<10;i++)
         UART1_Write(cc_space_arr[i]);
         
     // Qty Rate Amount Print
     for(i=0;cc_qty_prc_amt_arr[i] != '\0';i++)
         UART1_Write(cc_qty_prc_amt_arr[i]);
     UART1_Write(0x0A);
     
     // Space print
     for(i=0;i<15;i++)
         UART1_Write(cc_space_arr[i]);
     // Kg Rs Rs Print
     for(i=0;cc_kg_rs_arr[i] != '\0';i++)
         UART1_Write(cc_kg_rs_arr[i]);
     UART1_Write(0x0A);
     
     line_print();
     UART1_Write(0x0A);
}

// printing next item
void print_next()
{
     char  n_arr_total[9],i,j;

     serial_print(g_pr_prod_name);
     serial_print(" ");

     i=strlen(g_wt_arr_pr);
     if(i<6)
     {
        for(j=0;j<(6-i);j++)
            n_arr_total[j]= ' ';
        for(i=0;j<7;j++,i++)
            n_arr_total[j]= g_wt_arr_pr[i];
     }
     else
         strcpy(n_arr_total,g_wt_arr_pr);

     serial_print(n_arr_total);
     serial_print(" ");

     i=strlen(g_pr_prod_rate);
     if(i<6)
     {
        for(j=0;j<(6-i);j++)
            n_arr_total[j]= ' ';
        for(i=0;j<7;j++,i++)
            n_arr_total[j]= g_pr_prod_rate[i];
     }
     else
         strcpy(n_arr_total,g_pr_prod_rate);

     serial_print(n_arr_total);
     serial_print(" ");

     //strcpy(g_pr_prod_amt,"99999.99");
     i=strlen(g_pr_prod_amt);
     if(i>=8)
     {
         strncpy(n_arr_total, g_pr_prod_amt, 7);
         n_arr_total[8] = '\0';
     }
     else
     {
         if(i<7)
         {
            for(j=0;j<(7-i);j++)
                n_arr_total[j]= ' ';
            for(i=0;j<8;j++,i++)
                n_arr_total[j]= g_pr_prod_amt[i];
         }
         else
             strcpy(n_arr_total,g_pr_prod_amt);
     }
     serial_print(n_arr_total);
     UART1_Write(0x0A);

     g_prod_net_total += g_prod_total;                // Calulating total bill amount
     g_wt_arr_pr[0] = '\0';
}

// footer of the print bill
void print_footer()
{
     int i;
     double pr_roundoff = 0.0;
     long pr_total = 0;

     line_print();
     UART1_Write(0x0A);

     set_bold();
     pr_roundoff = g_prod_net_total;
     pr_total = (long)pr_roundoff;
     pr_roundoff = pr_roundoff - (long)pr_roundoff;
     if(pr_roundoff > 0.5)
           pr_total+=1;

     LongToStr(pr_total,g_pr_prod_total);

     for(i=0;cc_total_rs[i] != '\0';i++)
          UART1_Write(cc_total_rs[i]);
          
     //serial_print("TOTAL Rs ");
     serial_print(g_pr_prod_total+4);
     UART1_Write(0x0A);

     cancel_bold();

     line_print();
     UART1_Write(0x0A);

     serial_print(g_time_date);
     UART1_Write(0x0A);
     line_print();
     UART1_Write(0x0A);

     for(i=0;cc_g_thank_msg[i] != '\0';i++)
          UART1_Write(cc_g_thank_msg[i]);
     //serial_print(g_thank_msg);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);

     g_prod_net_total = 0;
}

// printing records
void print_record_header()
{
     int i;
     set_bold();
     
     for(i=0;cc_customer_line_1[i] != '\0';i++)
         UART1_Write(cc_customer_line_1[i]);
     //serial_print("OSWAL BROTHERS");
     UART1_Write(0x0A);

     cancel_bold();
     left_align();

     for(i=0;cc_customer_line_2[i] != '\0';i++)
         UART1_Write(cc_customer_line_2[i]);
     //serial_print("Usha Ganj,Indore");
     UART1_Write(0x0A);

     line_print();
     //serial_print(g_line_print);
     UART1_Write(0x0A);
     
     left_align();
     
     for(i=0;i<5;i++)
          UART1_Write(cc_item_arr[i]);
     // Space print
     for(i=0;i<9;i++)
         UART1_Write(cc_space_arr[i]);
         
     for(i=0;cc_rec_con_sale[i] != '\0';i++)
         UART1_Write(cc_rec_con_sale[i]);
     UART1_Write(0x0A);
     
     for(i=0;cc_space_arr[i] !='\0';i++)
         UART1_Write(cc_space_arr[i]);
         
     for(i=0;cc_rec_kg_rs_arr[i] !='\0';i++)
         UART1_Write(cc_rec_kg_rs_arr[i]);
     UART1_Write(0x0A);
     
     //serial_print("                   ");

     line_print();
     UART1_Write(0x0A);
}

void print_record_item()
{
     char  n_arr_total[9],i,j;

     serial_print(g_pr_prod_name);
     serial_print("   ");
     
     i=strlen(g_rec_cons);
     if(i<8)
     {
        for(j=0;j<(8-i);j++)
            n_arr_total[j]= ' ';
        for(i=0;j<9;j++,i++)
            n_arr_total[j]= g_rec_cons[i];
     }
     else
         strcpy(n_arr_total,g_rec_cons);
     serial_print(n_arr_total);
     
     //serial_print(g_rec_cons);
     
     serial_print("   ");
     i=strlen(g_rec_net_total);
     if(i<8)
     {
        for(j=0;j<(8-i);j++)
            n_arr_total[j]= ' ';
        for(i=0;j<9;j++,i++)
            n_arr_total[j]= g_rec_net_total[i];
     }
     else
         strcpy(n_arr_total,g_rec_net_total);
     serial_print(n_arr_total);
     
     //serial_print(g_rec_net_total);
     UART1_Write(0x0A);

}
void print_record_footer()
{
     char pr_total_arr[12];
     double pr_roundoff = 0.0;
     long pr_total = 0;
     int i;
     
     line_print();
     UART1_Write(0x0A);

     set_bold();
     pr_roundoff = g_total_sale;
     pr_total = (long)pr_roundoff;
     pr_roundoff = pr_roundoff - (long)pr_roundoff;
     if(pr_roundoff > 0.5)
           pr_total+=1;

     LongToStr(pr_total,pr_total_arr);
     for(i=0;cc_total_rs[i] != '\0';i++)
          UART1_Write(cc_total_rs[i]);

     //serial_print("TOTAL Rs ");
     serial_print(pr_total_arr+4);
     UART1_Write(0x0A);

     cancel_bold();

     line_print();
     UART1_Write(0x0A);
     serial_print(g_time_date);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
}

// print string
void serial_print(char *str)
{
     for( ;*str!='\0';str++)
          UART1_Write(*str);
}

// font Double width
void cancel_double()
{
     UART1_Write(0x1B);
     UART1_Write(0x14);
}

// set font bold
void set_bold()
{
     UART1_Write(0x1B);
     UART1_Write(0x0E);
}

// cancel font bold
void cancel_bold()
{
     UART1_Write(0x1B);
     UART1_Write(0x14);
}

// align print line to center
void center_align()
{
     UART1_Write(0x1B);
     UART1_Write(0x61);
     UART1_Write(0x01);
}

// aling print line to left
void left_align()
{
     UART1_Write(0x1B);
     UART1_Write(0x61);
     UART1_Write(0x00);
}

// default settings
void reset_printer()
{
     UART1_Write(0x1B);
     UART1_Write(0x21);
     UART1_Write(0x00);
}

// init global variables to zero
void init_g_var()
{
     g_key_prs             = 0;
     g_lv_kp               = 0;
     g_prod_sel_st         = PROD_NOT_SELECTED;
     g_lv_td_update        = 0;
     g_t_sec               = 0;
     g_td_cnt              = 0;
     g_st_spk_prs          = 0;
     g_lv_del_rec          = 0;
     g_cp_prod_loc         = 0;
     g_cp_prod_rate_idx    = 0;
     g_cp_cnt              = 0;
     g_sc_prod_loc         = 0;
     g_sc_ch_inc_cnt       = 0;
     g_sc_cnt              = 0;
     g_sc_prod_ch_idx      = 0;
     g_sc_code_ch_idx      = 0;
     g_sc_curr_ch          = 0;
     g_prod_loc            = 0;
     g_pr_flag             = 0;
     g_prod_rate           = 0.0;
     g_prod_total          = 0;
     g_prod_net_total      = 0;
     g_lcd_tot_ch_cnt      = 0;
     g_lcd_wt_ch_cnt       = 0;
     g_total_sale          = 0.0;

}
void line_print()
{
  unsigned short i;
  for(i=0;i<32;i++)
      UART1_Write('-');
}

void print_to_memory()
{
     unsigned int nxt_prod_ns_loc;
     unsigned int nxt_prod_cons_loc;
     double nxt_prod_ns;
     double nxt_prod_cons;
     char nxt_eeprom_data = 0;
     
                 nxt_prod_ns_loc         =  ITEM_START_ADD + (g_prod_loc * ITEM_MEM_CONST) + LOC_PROD_NS;
                 nxt_prod_cons_loc       =  ITEM_START_ADD + (g_prod_loc * ITEM_MEM_CONST) + LOC_PROD_CONS;

                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_ns_loc)) != 0xFF)
                     Lo(nxt_prod_ns)         = nxt_eeprom_data;
                 else
                     Lo(nxt_prod_ns)         = 0x00;

                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_ns_loc+1)) != 0xFF)
                     Hi(nxt_prod_ns)         = nxt_eeprom_data;
                 else
                     Hi(nxt_prod_ns)         = 0x00;

                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_ns_loc+2)) != 0xFF)
                     Higher(nxt_prod_ns)         = nxt_eeprom_data;
                 else
                     Higher(nxt_prod_ns)         = 0x00;

                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_ns_loc+3)) != 0xFF)
                     Highest(nxt_prod_ns)         = nxt_eeprom_data;
                 else
                     Highest(nxt_prod_ns)         = 0x00;

                 // Consumption read
                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_cons_loc)) != 0xFF)
                     Lo(nxt_prod_cons)         = nxt_eeprom_data;
                 else
                     Lo(nxt_prod_cons)         = 0x00;

                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_cons_loc+1)) != 0xFF)
                     Hi(nxt_prod_cons)         = nxt_eeprom_data;
                 else
                     Hi(nxt_prod_cons)         = 0x00;

                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_cons_loc+2)) != 0xFF)
                     Higher(nxt_prod_cons)         = nxt_eeprom_data;
                 else
                     Higher(nxt_prod_cons)         = 0x00;

                 if((nxt_eeprom_data =  EEPROM_Read(nxt_prod_cons_loc+3)) != 0xFF)
                     Highest(nxt_prod_cons)         = nxt_eeprom_data;
                 else
                     Highest(nxt_prod_cons)         = 0x00;

                 nxt_prod_ns += g_prod_total;
                 nxt_prod_cons += g_prod_wt;
                 Delay_Ms(50);

                 EEPROM_Write(nxt_prod_ns_loc,   Lo(nxt_prod_ns));
                 EEPROM_Write(nxt_prod_ns_loc+1, Hi(nxt_prod_ns));
                 EEPROM_Write(nxt_prod_ns_loc+2, Higher(nxt_prod_ns));
                 EEPROM_Write(nxt_prod_ns_loc+3, Highest(nxt_prod_ns));

                 EEPROM_Write(nxt_prod_cons_loc,   Lo(nxt_prod_cons));
                 EEPROM_Write(nxt_prod_cons_loc+1, Hi(nxt_prod_cons));
                 EEPROM_Write(nxt_prod_cons_loc+2, Higher(nxt_prod_cons));
                 EEPROM_Write(nxt_prod_cons_loc+3, Highest(nxt_prod_cons));
}