//EEPROM Addresses for stored items
#define ITEM_START_ADD             64
#define ITEM_MEM_CONST             48

// constant added for address resolution
#define LOC_PROD_CODE              3
#define LOC_PROD_RATE              5
#define LOC_PROD_NAME              17

#include "built_in.h"
void main() {

     char i=0,j=0;
     double price[10]={255.0,113.0,131.0,876.0,95.0,505.0,750.0,45.0,200.0,120.0};
     char *ccode[10]  ={"FX","SB","MS","OL","CN","CO","CS","PM","PT","SM"};
     char *item[10]   ={"FLAXSEED  ",
                        "SOYABEAN  ",
                        "MUSTARD   ",
                        "OLIVE     ",
                        "COCONUT   ",
                        "CORN      ",
                        "COTTONSEED",
                        "PALM      ",
                        "PEANUT    ",
                        "SESAME    "};
                        
                        //ITEM_START_ADD + (index*ITEM_MEM_CONST) + LOC_PROD_CODE
     for(i=0;i<10;i++)
     {
         EEPROM_Write(ITEM_START_ADD + (i*ITEM_MEM_CONST)+LOC_PROD_CODE,*(ccode[i]+0));
         EEPROM_Write(ITEM_START_ADD + (i*ITEM_MEM_CONST)+LOC_PROD_CODE+1,*(ccode[i]+1));
         
         for(j = 0; j <= 10; j++)
         {
             EEPROM_Write(ITEM_START_ADD + (i*ITEM_MEM_CONST) + LOC_PROD_NAME +j,*(item[i]+j));
             Delay_Ms(20);
         }


        EEPROM_Write(ITEM_START_ADD + (i*ITEM_MEM_CONST)+LOC_PROD_RATE,   Lo(price[i]));
        EEPROM_Write(ITEM_START_ADD + (i*ITEM_MEM_CONST)+LOC_PROD_RATE+1, Hi(price[i]));
        EEPROM_Write(ITEM_START_ADD + (i*ITEM_MEM_CONST)+LOC_PROD_RATE+2, Higher(price[i]));
        EEPROM_Write(ITEM_START_ADD + (i*ITEM_MEM_CONST)+LOC_PROD_RATE+3, Highest(price[i]));
        
        Delay_Ms(50);


     }


}
