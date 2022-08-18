#include "Header.h"

void get_timedate()
{
    sec=read_ds1307(0);
    minute=read_ds1307(1);
    hour=read_ds1307(2);
    date=read_ds1307(4);
    month=read_ds1307(5);
    year=read_ds1307(6);

    time[0] = BCD2UpperCh(hour);
    time[1] = BCD2LowerCh(hour);
    time[2] = ':';
    time[3] = BCD2UpperCh(minute);
    time[4] = BCD2LowerCh(minute);
    time[5] = ' ';
    time[6] = 'A';
    time[7] = 'M';
    time[8] = '\0';

    if( time[0] == '0' && time[1] == '0')
    {
        time[0] = '1';
        time[1] = '2';
        time[6] = 'A';
        time[7] = 'M';

    }
    else if(time[0] == '1' && time[1] == '2')
    {
        time[6] = 'P';
        time[7] = 'M';
    }
    else if((time[0] == '1' && time[1] > '2') || time[0] == '2' )
    {
        time[0] = time[0] - 1;
        time[1] = time[1] - 2;
        time[6] = 'P';
        time[7] = 'M';

    }

    ddate[0] = BCD2UpperCh(date);
    ddate[1] = BCD2LowerCh(date);
    ddate[2] ='/';
    ddate[3] = BCD2UpperCh(month);
    ddate[4] = BCD2LowerCh(month);
    ddate[5] ='/';
    ddate[6] = BCD2UpperCh(year);
    ddate[7] = BCD2LowerCh(year);
    ddate[8] = '\0';
}