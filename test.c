//-----------------------------------------------------------------------------------------------------------------------	
// Titel : Grundgerüst für Digitale Filter
// Datei : test.c
// Erstellt : 07.10.19 / haa
// Funktion : alle 50us timer-interrupt -> startet ADC -> per ADC-Interrupt AD Wert auf LED
//-----------------------------------------------------------------------------------------------------------------------	

#include <stm32f10x.h> 																		//Mikrocontrollertyp

int main(void) 																						//Hauptprogramm
{
//-----------------------------------------------------------------------------------------------------------------------	
//Initialisierung LED-Port/DAC Port A Pin 4
//-----------------------------------------------------------------------------------------------------------------------	
	RCC->APB2ENR |= 1<<6;																		//GPIOE Clock
	RCC->APB1ENR |= 1<<1;																		//Timer 3 clock enable
	GPIOE->CRH   = 	0x11111111;															//LED Port (PE15..8) output
	GPIOA->CRL   |= 0x00010000;															//DAC Port A Bit 4 output
	GPIOC->CRL 	 &= ~0x000F0000;														//PIN4 analog-input, wohl nicht nötig -> per default

//-----------------------------------------------------------------------------------------------------------------------	
//Initialisierung Timer 3
//-----------------------------------------------------------------------------------------------------------------------	
	TIM3->PSC = 23;																					//set prescaler to 23->24 (PSC+1 (1/72000000*24=333.3333ns) PSC ist 16Bit
	TIM3->ARR = 149;																				//auto reload value 149 (->150 -> 150*333.33ns=50us
	TIM3->DIER |= 1<<0;																			//enable update interupt (timer level)
	TIM3->CR1 |= 1<<0;																			//enable timer

//-------------------------------------------------------------------------------------------------------------------------------	
//Initialisierung Alternate Functions/peripheral clock/ADC/DAC
//-------------------------------------------------------------------------------------------------------------------------------	
	RCC->APB2ENR = RCC->APB2ENR | 0x00000200;								//set bit 9, clock ADC1
	RCC->APB1ENR 	|= 	(1<<29);															//set bit29, DAC Clock enable
	RCC->CFGR 		|= 	(2<<14);															//divide by 6 (default divide by 2)
	ADC1->SQR3		 = 	(14<<0);															//channel 14 als einzigen Kanal->kein shiften nötig
	ADC1->CR2 		|= (7<<17);																//start conversion by sw
	ADC1->CR2 		|= (1<<20);																//enable extern trigger (per sw=extern)
	ADC1->CR1   	|= (1<<5);																//EOCIE
  NVIC->ISER[0] |= (1<<18);																//interrupt set enable register for adc
	ADC1->CR2 		|= (1<<0);																//ADC ON

//-----------------------------------------------------------------------------------------------------------------------	
//Initialisierung Interrupt NVIC Timer 3
//-----------------------------------------------------------------------------------------------------------------------	
  NVIC->ISER[0] |= (1<<29);																//enable interrupt from TIM3 (NVIC level)

//-----------------------------------------------------------------------------------------------------------------------	
//  Calibration ADC
//-----------------------------------------------------------------------------------------------------------------------	
	ADC1->CR2 		|= (1<<3);  															// Reset calibration  
  while (ADC1->CR2 & (1<<3));  														// wait until reset finished  
	ADC1->CR2 |= (1<<2);  																	// start calibration  
  while (ADC1->CR2 & (1<<2));  														// wait until calibration finished 

//-----------------------------------------------------------------------------------------------------------------------	
//  DAC-Setup 
//-----------------------------------------------------------------------------------------------------------------------	
	DAC->CR				|=	0x00000039;														//DAC initialisieren (SW-Trigger, Enable)

//-----------------------------------------------------------------------------------------------------------------------	
//forever
//-----------------------------------------------------------------------------------------------------------------------	
	while(1)																								//forever
	{
		;																											//nothing to do 
	}
}

//-----------------------------------------------------------------------------------------------------------------------	
//Timer 3 Interrupt Handler
//-----------------------------------------------------------------------------------------------------------------------	
void TIM3_IRQHandler(void)
{
	ADC1->CR2 		|= 	(1<<0);																//start conversion
	TIM3->SR &= ~(1<<0);                    								// clear UIF flag
}

//-----------------------------------------------------------------------------------------------------------------------	
//ADC Interrupt Handler
//-----------------------------------------------------------------------------------------------------------------------	
void ADC1_2_IRQHandler(void)
{
//	GPIOE->ODR = (ADC1->DR)<<4;															//set LED aus 12Bit->16Bit->higher Byte anzeigen auf LED
	GPIOE->ODR ^= 0xFFFF;																			//toggle LED -> messen sampling rate
//ADC1->SR &= ~(1<<1); 																			//wird automatisch durch auslesen des Registers gelöscht
	DAC->DHR12R1	=		(ADC1->DR);															//12-Bit Wert DAC=ADC											
	DAC->SWTRIGR	|=	0x00000001;
}

