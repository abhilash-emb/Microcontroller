#include "Header.h"

unsigned short read_ds1307(unsigned short address)
{
    I2C1_Start();
    I2C1_Wr(0xd0);
    I2C1_Wr(address);
    I2C1_Repeated_Start();
    I2C1_Wr(0xd1);
    datas=I2C1_Rd(0);
    I2C1_Stop();
    return(datas);
}

unsigned char BCD2UpperCh(unsigned char bcd)
{
    return ((bcd >> 4) + '0');
}

unsigned char BCD2LowerCh(unsigned char bcd)
{
    return ((bcd & 0x0F) + '0');
}