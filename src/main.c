#include "main.h"

static volatile unsigned int TimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

/* Private Function prototypes -----------------------------------------------*/
void  RCC_Configuration(void);
void  RTC_Configuration(void);
void  Init_GPIOs (void);
void  Init_Board(void);
/*******************************************************************************/

const uint32_t track[1] = { 0x01234567,
                            0x89ABCDEF };

uint32_t getTrackAt(uint32_t pos) {
  uint8_t trackIndex = pos >> 5;
  uint32_t leftSeg = track[trackIndex];
  uint32_t rightSeg = track[trackIndex + 1];
  uint8_t posWithinTrack = pos & 0b0001 1111;

  uint32_t trackSection = (leftSeg << posWithinTrack) | (rightSeg >> (32 - posWithinTrack));
  return trackSection;
}

int main(void) {
  Init_Board();

  LCD_GLASS_DisplayString("055555");

  while(TRUE) {
    GPIO_ResetBits(GPIOB, GPIO_Pin_6); // Turn on Blue
    GPIO_SetBits(  GPIOB, GPIO_Pin_7); // Turn off Green
    Delay(2000); 

    GPIO_SetBits(  GPIOB, GPIO_Pin_6);
    GPIO_ResetBits(GPIOB, GPIO_Pin_7);
    Delay(2000); 
  }
}
           
void UserButtonDown(void)
{
  LCD_GLASS_DisplayString("1");
}

void UserButtonUp(void)
{
  LCD_GLASS_DisplayString("0");
}

void Delay(unsigned int nTime)
{
  TimingDelay = nTime;
  while(TimingDelay != 0);
  
}

void Decrement_TimingDelay(void)
{
  if (TimingDelay != 0x00) { 
    TimingDelay--;
  }
}                                  

void Init_Board(void)
{ 
  /* Configure RTC Clocks */
  RTC_Configuration();

  /* Set internal voltage regulator to 1.8V */
  PWR_VoltageScalingConfig(PWR_VoltageScaling_Range1);

  /* Wait Until the Voltage Regulator is ready */
  while (PWR_GetFlagStatus(PWR_FLAG_VOS) != RESET) ;

  /* Enable debug features in low power modes (Sleep, STOP and STANDBY) */
#ifdef  DEBUG_SWD_PIN
  DBGMCU_Config(DBGMCU_SLEEP | DBGMCU_STOP | DBGMCU_STANDBY, ENABLE);
#endif
  
  /* Configure SysTick IRQ and SysTick Timer to generate interrupts */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
  
  /* Initializes the LCD glass */
  LCD_GLASS_Configure_GPIO();
  LCD_GLASS_Init();

  /* Init I/O ports */
  Init_GPIOs();
}
		
//Configures the different system clocks.
void RCC_Configuration(void)
{  
  
  /* Enable HSI Clock */
  RCC_HSICmd(ENABLE);
  
  /*!< Wait till HSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
  {}

  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  
  RCC_MSIRangeConfig(RCC_MSIRange_6);

  RCC_HSEConfig(RCC_HSE_OFF);  
  if(RCC_GetFlagStatus(RCC_FLAG_HSERDY) != RESET )
  {
    while(1);
  }
 
  /* Enable  comparator clock LCD and PWR mngt */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_LCD | RCC_APB1Periph_PWR, ENABLE);
    
  /* Enable ADC clock & SYSCFG */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_SYSCFG, ENABLE);

}


void RTC_Configuration(void)
{
  
/* Allow access to the RTC */
  PWR_RTCAccessCmd(ENABLE);

  /* Reset Backup Domain */
  RCC_RTCResetCmd(ENABLE);
  RCC_RTCResetCmd(DISABLE);

  /* LSE Enable */
  RCC_LSEConfig(RCC_LSE_ON);

  /* Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {}
  
  RCC_RTCCLKCmd(ENABLE);
   
  /* LCD Clock Source Selection */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable comparator clock LCD */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_LCD, ENABLE);

}

//To initialize the I/O ports
void conf_analog_all_GPIOS(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOs clock */ 	
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | 
                        RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD | 
                        RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH, ENABLE);

  /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  
  /* Disable GPIOs clock */ 	
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
                        RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD | 
                        RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH, DISABLE);
}

void  Init_GPIOs (void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable GPIOs clock */ 	
  RCC_AHBPeriphClockCmd(LD_GPIO_PORT_CLK | USERBUTTON_GPIO_CLK, ENABLE);
 
  /* Configure User Button pin as input */
  GPIO_InitStructure.GPIO_Pin = USERBUTTON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(USERBUTTON_GPIO_PORT, &GPIO_InitStructure);

  /* Connect Button EXTI Line to Button GPIO Pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);

  /* Configure User Button and IDD_WakeUP EXTI line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0 ;  // PA0 for User button AND IDD_WakeUP
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set User Button and IDD_WakeUP EXTI Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure); 

/* Configure the GPIO_LED pins  LD3 & LD4*/
  GPIO_InitStructure.GPIO_Pin = LD_GREEN_GPIO_PIN | LD_BLUE_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(LD_GPIO_PORT, &GPIO_InitStructure);
  GPIO_LOW(LD_GPIO_PORT, LD_GREEN_GPIO_PIN);	
  GPIO_LOW(LD_GPIO_PORT, LD_BLUE_GPIO_PIN);

/* Disable all GPIOs clock */ 	
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | 
                        RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD | 
                        RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH, DISABLE);

  RCC_AHBPeriphClockCmd(LD_GPIO_PORT_CLK | USERBUTTON_GPIO_CLK, ENABLE);
}  

