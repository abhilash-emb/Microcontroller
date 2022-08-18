#ifndef HEADER_H
#define HEADER_H
extern void serial_print(char *);
extern void print_ticket();
extern void get_timedate();

extern unsigned char BCD2UpperCh(unsigned char bcd);
extern unsigned char BCD2LowerCh(unsigned char bcd);
extern unsigned short read_ds1307(unsigned short address );
extern unsigned short sec;
extern unsigned short minute;
extern unsigned short hour;
extern unsigned short day;
extern unsigned short date;
extern unsigned short month;
extern unsigned short year;
extern unsigned short datas;

extern char wt_arr[9];
extern char prt_arr[9];
extern char header[16];
extern char place[7];
extern char thanks[22];
extern char line[24];
extern int z_flag;

extern char ddate[9];
extern char time[9];
#endif