#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h> 

uint16_t ADC_VALUE[3]={0,0,0};
char BUFFER[100];
int j=0;
int i=0;

void hse_clk(void) {
    RCC->CR |= RCC_CR_HSEON; 
    while (!(RCC->CR & RCC_CR_HSERDY)); 
    RCC->CFGR = RCC_CFGR_SW_0; 
    while (!(RCC->CFGR & RCC_CFGR_SWS_0)); 
}

void config_EXTI0(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //enable clock for system_configuration
    SYSCFG->EXTICR[0] = 0x0; //Select PA0 as External Interrupt Source
    EXTI->RTSR |= EXTI_RTSR_TR0; //Rising trigger enabled
    EXTI->IMR |= EXTI_IMR_MR0; // Enable interrupt
    NVIC_EnableIRQ(EXTI0_IRQn); // Enable request in NVIC
}

void config_TIMER_2 (void)
{
	RCC->APB1ENR |=(1 << 0);
	TIM2->PSC =7999; //1ms
	TIM2->ARR=999;
	TIM2->CR2 |= (2<<4); 
	TIM2->CR1 |=(1 << 0); // Enable TIM2
}
void config_ADC (void)
{
	// PA4  PC4  PC5
	RCC->APB2ENR |= (1 << 8) ; //Enable ADC1 clock
	RCC->AHB1ENR |= (1 << 0); //Enable GPIOA clock  
	RCC->AHB1ENR |=(1 << 2); //Enable GPIOC clock
	
	GPIOA->MODER |= (3 << 8); 
	GPIOC->MODER |= (3 << 8);  
  GPIOC->MODER |= (3 << 10); 
	
  ADC1->SQR1 |= (2 << 20);

  ADC1->SQR3 |= (4 << 0);   // CH4  1st conversion
  ADC1->SQR3 |= (14 << 5);  // CH14  2nd conversion
  ADC1->SQR3 |= (15 << 10); // CH15 3rd conversion

  ADC1->CR1 |= (1 << 8);  // Enable mode scan
  ADC1->CR2 |= (1 << 8);  // Enable DMA
  ADC1->CR2 |= (1 << 10); // EOCS = 1
  ADC1->CR2 |= (1 << 9);  // Enable DMA continous mode  (DDS = 1)
  ADC1->CR2 |= (1 << 28); // rising edge
  ADC1->CR2 |= (6 << 24); // TIM2 TRGO 

  ADC1->CR2 |= (1 << 0); // Enable ADC

}
void config_DMA2 (void)
{
	// Stream 0
	RCC->AHB1ENR |= (1 << 22);  // DMA2 clock enable
	DMA2_Stream0->CR &= ~(7 << 25); // Select channel 0
	DMA2_Stream0->CR|= (1 <<10);
	DMA2_Stream0->CR |= (1 << 8); //	 Mode circulaire 
	
	DMA2_Stream0->CR|= (0b01 <<11);
	DMA2_Stream0->CR|= (0b01 << 13);
	
	DMA2_Stream0->PAR =(int) &ADC1->DR;  // les données de DATA REGISTRE

	DMA2_Stream0->M0AR= (int)ADC_VALUE; // sauvgarde dans ADC_VALUE dans SRAM
	
	DMA2_Stream0->NDTR=3;// 3 conversions
  
	DMA2_Stream0->FCR&= ~(1 << 2); // EN direct mode (Desactiver le FIFO )
  
	DMA2_Stream0->CR|= (1 << 4);
	DMA2_Stream0->CR |=(1 << 0);  // Enable DMA2 Stream 0
		
}



void DMA2_Stream0_IRQHandler(void)
{
 if( DMA2->LISR & (DMA_LISR_TCIF0)) //transfer complete flag
 {
	 i=1; 
	 
   DMA2->LIFCR |= DMA_LIFCR_CTCIF0; //clear transfer complete flag
 }
}


void EXTI0_IRQHandler(void) {
    j++;
    if (j == 1) { 
        TIM2->CR1 = 0x1; // Enable TIM2 counter
    }
    if (j == 2) { 
        TIM2->CR1 = 0x0;//Disable TIM2 counter
        j = 0;
    }
    EXTI->PR = 1; //clear flag
}
int main(void) 
{
	  hse_clk();
    config_EXTI0();
    config_TIMER_2();
    config_ADC();
    config_DMA2();
	while(1)
		{
     if (i==1)
		 {
			
      sprintf(BUFFER,"CH1=%d CH2=%d CH3=%d",ADC_VALUE[0],ADC_VALUE[1],ADC_VALUE[2]); 
			
			 i=0;
		 }			 
	}
}
