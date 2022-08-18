void fun_print_header();                                        // Printing header
void fun_print_next();                                          // Printing next item
void fun_print_footer();                                        // Printing footer
void fun_print_record();                                        // Printing record

void fun_serial_print(char *);                                  // Sending string to serial port
void fun_cancel_bold();
void fun_bold();                                                // Set BOLD printing
void fun_cancel_double();
void fun_center_align();                                        // Set alignment to CENTER
void fun_left_align();                                          // Set alignment to LEFT
void fun_reset_printer();                                       // RESET printer settings

char g_pr_prod_name[]="SOYABEAN  ";//[11];
char g_pr_prod_qty[]=" 1.45";//[6];
char g_pr_prod_rate[]=" 50.00";//[7];
char g_pr_prod_amount[]="  72.50";//[8];
char g_pr_prod_total[]="22250";//[9];
char g_pr_bill[]="  123";//[6];
char g_rec_cons[]="200.500";//[8];
char g_rec_ntotal[]="10025.00";//[9];

char g_time_date[]="25/07/2013   08:49 AM";

char g_thank_msg[]="Thank You Visit Again";
char g_line_print[]="--------------------------------";

void main() {
     TRISB = 0xff;
     UART1_Init(9600);

     UART1_Write(0x1B);                                         // Initialize printer
     UART1_Write(0x40);
     while(1)
     {
             if(PORTB.F0 == 1)
             {
              fun_print_header();
              while(PORTB.F0 == 1);
             }
             else if(PORTB.F1 == 1)
             {
              fun_print_next();
              while(PORTB.F1 == 1);
             }
             else if(PORTB.F2 == 1)
             {
              fun_print_footer();
              while(PORTB.F2 == 1);
             }
             else if(PORTB.F3 == 1)
             {
              fun_print_record();
              while(PORTB.F3 == 1);
             }
     }
}
void fun_print_header()
{
     fun_cancel_double();
     fun_bold();
     fun_serial_print("OSWAL OIL");
     UART1_Write(0x0A);

     fun_cancel_bold();
     fun_left_align();
     
     fun_serial_print("Indore");
     UART1_Write(0x0A);

     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     fun_serial_print(g_time_date);
     UART1_Write(0x0A);
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     fun_serial_print("ITEM         QTY  PRICE   AMOUNT");
     UART1_Write(0x0A);
     fun_serial_print("              Kg     Rs       Rs");
     UART1_Write(0x0A);
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
}
void fun_print_next()
{
     fun_serial_print(g_pr_prod_name);
     fun_serial_print(" ");
     fun_serial_print(g_pr_prod_qty);
     fun_serial_print(" ");
     fun_serial_print(g_pr_prod_rate);
     fun_serial_print("  ");
     fun_serial_print(g_pr_prod_amount);
     UART1_Write(0x0A);
}
void fun_print_footer()
{
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     
     fun_bold();
     
     fun_serial_print("TOTAL   Rs ");
     fun_serial_print(g_pr_prod_total);
     UART1_Write(0x0A);
     
     fun_cancel_bold();
     
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     
     fun_serial_print("Bill No : ");
     fun_serial_print(g_pr_bill);
     UART1_Write(0x0A);
     
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);

     fun_serial_print(g_thank_msg);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
}
void fun_print_record()
{
     fun_bold();
     fun_serial_print("ABC Corp");
     UART1_Write(0x0A);
     
     fun_cancel_bold();
     fun_left_align();
     fun_serial_print("Bangalore");
     UART1_Write(0x0A);

     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     fun_serial_print(g_time_date);
     UART1_Write(0x0A);
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     fun_left_align();
     fun_serial_print("ITEM            CONS   NET PRICE");
     UART1_Write(0x0A);
     fun_serial_print("                  Kg          Rs");
     UART1_Write(0x0A);
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     fun_serial_print(g_pr_prod_name);
     fun_serial_print("   ");
     fun_serial_print(g_rec_cons);
     fun_serial_print("    ");
     fun_serial_print(g_rec_ntotal);
     UART1_Write(0x0A);
     fun_serial_print(g_line_print);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
}
void fun_serial_print(char *str)
{
     for( ;*str!='\0';str++)
     {
          UART1_Write(*str);
     }
}
void fun_cancel_double()
{
     UART1_Write(0x1B); // Double width
     UART1_Write(0x14);
}
void fun_bold()
{
     UART1_Write(0x1B);
     UART1_Write(0x0E);
}
void fun_cancel_bold()
{
     UART1_Write(0x1B); // Double width
     UART1_Write(0x14);
}
void fun_center_align()
{
     UART1_Write(0x1B); // center
     UART1_Write(0x61);
     UART1_Write(0x01);
}
void fun_left_align()
{
     UART1_Write(0x1B); // left
     UART1_Write(0x61);
     UART1_Write(0x00);
}
void fun_reset_printer()
{
     UART1_Write(0x1B); // Reset print mode
     UART1_Write(0x21);
     UART1_Write(0x00);
}