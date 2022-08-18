void serial_print(char *);
void print_ticket();
void get_timedate();

unsigned char BCD2UpperCh(unsigned char bcd);
unsigned char BCD2LowerCh(unsigned char bcd);
unsigned short read_ds1307(unsigned short address );
unsigned short sec;
unsigned short minute;
unsigned short hour;
unsigned short day;
unsigned short date;
unsigned short month;
unsigned short year;
unsigned short datas;

char wt_arr[9];
char prt_arr[9];
char header[] = "XYZ LTD";
char place[] = "Bangalore";
char line[] = "-----------------------";
char thanks[] = "Thank You Visit Again";

int z_flag = 0;

char ddate[9];
char time[9];
void main()
{
 char i,j,rxdata,l,k,m;
      TRISB = 0x01;
      UART1_Init(9600);
      I2C1_Init(100000);
      
      UART1_Write(0x1B);
      UART1_Write(0x40);
      m=0;
      while(1)
       {
          if(PORTB.F0 == 1)
          {
           while( PORTB.F0 == 1);
            for( k = 0+m ; k < 2 ; k++ ) // Recieve from scale twice only for 1st time
            {
               while(1)
               {
                 while(!UART1_Data_Ready());
                 rxdata  = UART1_Read();
                   if( ( rxdata >= '0' && rxdata <= '9') || rxdata == '.')
                   {
                       z_flag = 0;
                       i = 0;
                       do
                       {
                            wt_arr[i++] = rxdata;
                            while(!UART1_Data_Ready());
                            rxdata =  UART1_Read();

                       }while(rxdata != 0x0A);

                       wt_arr[i] = '\0';

                       for(i=0;wt_arr[i]!='\0'; i++)
                           {
                               if((wt_arr[i] != '0') && (wt_arr[i] != '.') )
                               {
                                    for(j=0;wt_arr[i]!='\0';j++)
                                    {
                                        prt_arr[j] = wt_arr[i++];
                                    }
                                    z_flag = 1;
                                    prt_arr[j] = '\0';
                                    break;
                               }
                               else if( wt_arr[i] == '.')
                               {
                                    for(l=i+1;wt_arr[l]!='\0';l++)
                                    {
                                         if( wt_arr[l]!='0')
                                             z_flag = 1;
                                    }
                                    if( z_flag == 1 )
                                    {
                                        prt_arr[0] = '0';
                                        for(j=1;wt_arr[i]!='\0';j++)
                                        {
                                            prt_arr[j] = wt_arr[i++];
                                        }
                                        prt_arr[j] = '\0';
                                    }

                                    break;
                               }
                           }

                         if(z_flag == 0)
                         {
                             prt_arr[0] = '0';
                             prt_arr[1] = '\0';
                         }

                       break;
                   }
               }
            }
            print_ticket();
            if(m == 0) // Check whether program is running for 1st Time
                 m = 1;
                 
          }// end of switch check
       }
}