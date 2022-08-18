#include "stm8s.h"
#include "stm8s_clk.h"
#include "stm8s_exti.h"
#include "stm8s_gpio.h"
#include "stm8s_itc.h"
#include "stm8s_uart1.h"

#define ITC_IRQ_UART2_TX      (uint8_t)20  /*!< USART2 TX interrupt */
#define ITC_IRQ_UART2_RX      (uint8_t)21  /*!< USART2 RX interrupt */

uint8_t *itoa(int32_t value, uint8_t *s, uint8_t radix);
void uart_int_str(int32_t value, uint8_t *s);
void delay(uint16_t nCount);
void send_udata(uint8_t data);
void set_clk_HSI_16M(void);

uint8_t uart_buffer[33];
uint8_t test_var;
uint32_t disp_adcrc;
int32_t adc_raw_count;

int main()
{
   //uint8_t ret_val;
   test_var = 0;
   adc_raw_count = 0;
   disp_adcrc = 0;

   set_clk_HSI_16M();

   UART2_DeInit();
   UART2_Init(9600,UART2_WORDLENGTH_8D,UART2_STOPBITS_1,UART2_PARITY_NO,UART2_SYNCMODE_CLOCK_DISABLE,UART2_MODE_TXRX_ENABLE);
   UART2_Cmd(ENABLE);
   ITC_SetSoftwarePriority(ITC_IRQ_UART2_RX, ITC_PRIORITYLEVEL_3);
   UART2_ITConfig(UART2_IT_RXNE_OR,ENABLE);

  // Reset GPIO port D to a default state
  /*GPIO_DeInit(GPIOD);
  GPIO_DeInit(GPIOB);*/
  // Set operation mode for port D / pin 0
  // (connected to the onboard LED)
  GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(GPIOE, GPIO_PIN_0, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(GPIOB, GPIO_PIN_6, GPIO_MODE_IN_FL_IT);
  GPIO_WriteHigh(GPIOD, GPIO_PIN_0);
  delay(10000);
  send_udata('s');
  send_udata('\r');
  send_udata('\n');

  /* Initialize the Interrupt sensitivity */
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_FALL_ONLY);
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);

  enableInterrupts();

  //GPIO_WriteLow(GPIOD, GPIO_PIN_0);
    //__asm__("rim");
  /*__asm;
  rim
  __endasm;*/
  //enableInterrupts();
  //send_udata(0x77);
  /*ret_val = GPIO_ReadInputPin(GPIOB,GPIO_PIN_6);
  send_udata(ret_val);*/

  while (1)
  {
    /*if(adc_hl > 2000)
       GPIO_WriteLow(GPIOD, GPIO_PIN_0);*/
    if(test_var) {
        uart_int_str(disp_adcrc,uart_buffer);
        send_udata('\r');
        send_udata('\n');
        test_var = 0;
    }
    /*GPIO_WriteReverse(GPIOD, GPIO_PIN_0);
    delay(60000);*/
    //GPIO_WriteLow(GPIOD, GPIO_PIN_0);
    /*if(GPIO_ReadInputPin(GPIOB,GPIO_PIN_6) == 0x40) {
        GPIO_WriteLow(GPIOD, GPIO_PIN_0);
        ret_val = GPIO_ReadInputPin(GPIOB,GPIO_PIN_6);
        send_udata(ret_val);
    }
    else if (GPIO_ReadInputPin(GPIOB,GPIO_PIN_6) == 0x00) {
        GPIO_WriteHigh(GPIOD, GPIO_PIN_0);
        ret_val = GPIO_ReadInputPin(GPIOB,GPIO_PIN_6);
        send_udata(ret_val);
    }
    else
       send_udata(0x77);*/

     /*ret_val = GPIO_ReadInputPin(GPIOB,GPIO_PIN_6);
     send_udata(ret_val);
     delay(60000);*/
    /* The LED toggles in the interrupt routines */
  }

}

void EXTI_PORTB_IRQHandler() __interrupt (4)
{
    uint8_t i;

    adc_raw_count = 0;
    disp_adcrc++;
    //disp_adcrc = 0;
    /*GPIO_WriteLow(GPIOE, GPIO_PIN_0);

    for (i=0;i<24;i++){
        GPIO_WriteHigh(GPIOE, GPIO_PIN_0);
        adc_raw_count = adc_raw_count << 1;
        GPIO_WriteLow(GPIOE, GPIO_PIN_0);
        if(GPIO_ReadInputPin(GPIOB, GPIO_PIN_6)) {
            adc_raw_count++;
            //disp_adcrc++;
        }
        //delay(1000);
    }

    GPIO_WriteHigh(GPIOE, GPIO_PIN_0);
    adc_raw_count = adc_raw_count ^ 0x800000;
    GPIO_WriteLow(GPIOE, GPIO_PIN_0);

    GPIO_WriteHigh(GPIOE, GPIO_PIN_0);
    nop();
    GPIO_WriteLow(GPIOE, GPIO_PIN_0);

    GPIO_WriteHigh(GPIOE, GPIO_PIN_0);
    nop();
    GPIO_WriteLow(GPIOE, GPIO_PIN_0);*/

    test_var = 1;
    /*if(test_var) {
        GPIO_WriteLow(GPIOD, GPIO_PIN_0);
        send_udata('h');
    }
    else {
        GPIO_WriteHigh(GPIOD, GPIO_PIN_0);
        send_udata('l');
    }
    test_var = !test_var;*/
}
/*INTERRUPT_HANDLER(EXTI_PORTB_IRQHandler, 4)
{

}*/

void uart_int_str(int32_t data,uint8_t *s)
{
    itoa(data,uart_buffer,10);

    while(*s)
        send_udata(*s++);
}

void send_udata(uint8_t data)
{
    while(!(UART2->SR & UART2_SR_TXE));
    UART2->DR = data;
}

uint8_t *itoa(int32_t value, uint8_t *s, uint8_t radix)
{
    const uint8_t *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    uint32_t ulvalue = value;
    uint8_t *p = s, *q = s;
    uint8_t temp;

    if (radix == 10 && value < 0)
    {
        *p++ = '-';
        q = p;
        ulvalue = -value;
    }

    do
    {
        *p++ = digits[ulvalue % radix];
        ulvalue /= radix;
    }
    while (ulvalue > 0);

    *p-- = '\0';

    while (q < p)
    {
        temp = *q;
        *q++ = *p;
        *p-- = temp;
    }

    return s;
}

void delay(uint16_t nCount)
{
    /* Decrement nCount value */
    while (nCount != 0)
        nCount--;
}

void set_clk_HSI_16M(void)
{
    // go to default state
    CLK_DeInit();

    /* Configure the Fcpu to DIV1*/
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);

    /* Configure the HSI prescaler to the optimal value */
    CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);

    /* Output Fcpu on CLK_CCO pin */
    CLK_CCOConfig(CLK_OUTPUT_CPU);

    /* 16Mhz internal clock */
    CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSI, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);

    /*wait for some clk cycles*/
    delay(1000);
}


