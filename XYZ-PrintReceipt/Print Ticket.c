#include "Header.h"
void print_ticket()
{
    get_timedate();

     UART1_Write(0x0A);
     UART1_Write(0x0A);
     
     UART1_Write(0x1B);
     UART1_Write(0x61);
     UART1_Write(0x01);

     UART1_Write(0x1B);
     UART1_Write(0x21);
     UART1_Write(0x00);

     serial_print(header); //print string XYZ Ltd
     UART1_Write(0x0A);

     serial_print(place);  //print strin Bangalore
     UART1_Write(0x0A);

     serial_print(line);   // print -----------------------
     UART1_Write(0x0A);

     UART1_Write(0x1B);
     UART1_Write(0x61);
     UART1_Write(0x00);

     UART1_Write(0x0A);
     serial_print("Weight : ");
     serial_print(prt_arr);
     serial_print(" KG");
     UART1_Write(0x0A);

     UART1_Write(0x1B);
     UART1_Write(0x61);
     UART1_Write(0x01);

     UART1_Write(0x0A);
     serial_print("DT: ");
     serial_print(ddate);
     serial_print(" ");
     serial_print(time);
     UART1_Write(0x0A);


     serial_print(line);     //print -------------------------------
     UART1_Write(0x0A);

     serial_print(thanks);   // print thanks
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);
     UART1_Write(0x0A);

}

void serial_print(char *str)
{
   for( ;*str!='\0';str++)
   {
        UART1_Write(*str);
   }
}