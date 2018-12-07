/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>

void init_lcd_spi(void);
void display1_spi(const char *s);
void display2_spi(const char *s);
void adc_init(void);
void push_button_config();
void dma_init(short int *);
void adc_read(void);


int main(void){
	init_lcd_spi();
	dac_init();
	tim6_init();
	adc_init();

	//change_pitch();

	char line[21];

	while(1){
	    adc_read();
	    nano_wait(1000);
	    sprintf(line, "Integer: %d", DAC->DHR12R1);
	    display1_spi(line);

//	    sprintf(line, "Integer: 0x%x", ADC1->DR);
//        display1_spi(line);
//        sprintf(line, "Voltage: %2.2f", ADC1->DR * 3 / 4095.0);
//        display2_spi(line);
	}

//        while(!(ADC1->ISR & ADC_ISR_EOC));
//        sprintf(line, "Integer: 0x%x", ADC1->DR);
//        display1_spi(line);
//        sprintf(line, "Voltage: %2.2f", ADC1->DR * 3 / 4095.0);
//        display2_spi(line);

	return 0;
}

/*
 * char line[21];
	init_lcd_spi();
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	GPIOB->MODER |= 3;
	RCC->CR2 |= 1;
	while(!(RCC->CR2 & 0x2));
	ADC1->CR |= 1;
	while(!(ADC1->ISR & 1));
	while((ADC1->CR & ADC_CR_ADSTART));

	while(1)
	{
		ADC1->CHSELR = 0;
		ADC1->CHSELR |= 1<<8;
		while(!(ADC1->ISR & 1));
		ADC1->CR |= (1<<2);
		while(!(ADC1->ISR |= (1<<2)));
		sprintf(line, "%2.2f", ADC1->DR *3 / 4095.0);
		display1_spi(line);
 *
 */
