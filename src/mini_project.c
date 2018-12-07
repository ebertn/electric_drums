#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "sinewave.h"

int count = 0;
int sineCount = 0;

void dac_init(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //GPIO Clock
	RCC->APB1ENR |= RCC_APB1ENR_DACEN; //DAC clock
	GPIOA->MODER |= 3<<(2*4);
	DAC->CR &= ~DAC_CR_EN1; //Disable DAC

	DAC->CR |= DAC_CR_TEN1;
	DAC->CR |= DAC_CR_TSEL1; // Timer 6 TRGO event
    DAC->CR &= ~DAC_CR_BOFF1;
	DAC->CR |= DAC_CR_EN1;
	//DAC->CR |= (1<<12);
	//DMA1_Channel1->CCR |= 1;
	//DAC->CR |= 1;
}

void adc_init() {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER |= 0x3;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    RCC->CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY));

    ADC1->CR |= ADC_CR_ADEN;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    while((ADC1->CR & ADC_CR_ADSTART));
}
void adc_init_2() {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER |= (0x3<<1);
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    RCC->CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY));

    ADC1->CR |= ADC_CR_ADEN;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    while((ADC1->CR & ADC_CR_ADSTART));
}


#define NUM_AVERAGE 10
int prev[NUM_AVERAGE] = {0};

int adc_average(){
    int sum = 0;
    for (int i = 0; i < NUM_AVERAGE - 1; i++){
        prev[i] = prev[i+1];
        sum += prev[i];
    }
    //adc_read();
    prev[NUM_AVERAGE - 1] = ADC1->DR;
    sum += prev[NUM_AVERAGE - 1];

    return sum / NUM_AVERAGE;
}

void adc_read(void){
    ADC1->CHSELR = 0;
    ADC1->CHSELR |= 1 << 8;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CR |= ADC_CR_ADSTART;

    char line[21];
    sprintf(line, "Count: %d", count);
    display2_spi(line);

    //while(!(ADC1->ISR & ADC_ISR_EOC));
}
void adc_read_2(void){
    ADC1->CHSELR = 0;
    ADC1->CHSELR |= 1 << 9;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CR |= ADC_CR_ADSTART;

    //char line[21];
    //printf(line, "Count: %d", count);
    //display2_spi(line);

    //while(!(ADC1->ISR & ADC_ISR_EOC));
}
void TIM6_DAC_IRQHandler(void){
   //int ave = adc_average();

    if(count > 0){
        DAC->DHR12R1 = sine_table[sineCount];
        sineCount += 1;
        count--;
    } else if(ADC1->DR > 500){
        DAC->DHR12R1 = sine_table[sineCount];
        sineCount += 1;
        count = 2000;
    } else if(count <= 0) {
        DAC->DHR12R1 = 0;
    }


    //unsigned int val = 20 * (sine_table[sineCount] * (ave / 4095.0));

//   if(val > 0xfff){
//       val = 0xfff;
//   }

    DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

    //nano_wait(100000);

    if(sineCount == 360){
        sineCount = 0;
    }

    TIM6->SR &= ~TIM_SR_UIF;
}

// Value to put in ARR for each note
int note_B4 = 67; // 493.88
int note_A4 = 75; // 440
int note_G4 = 84; // 392
int note_F4 = 95; // 349
int note_E4 = 101; // 329
int note_D4 = 113; // 293
int note_C4 = 127; // 261
int note_B3 = 135; // 246


void change_pitch(){

//	int note_table[] = {
//		note_B4, note_A4, note_G4,
//		note_F4, note_E4, note_D4,
//		note_C4, note_B3
//	};

    TIM6->ARR = note_D4;
//	for(int i = 0; i < 8; i++){
//		TIM6->ARR = note_table[i];
//		nano_wait(1000000000);
//
//		if (i == 7){
//			i = 0;
//		}
//	}
}



void tim6_init(void) {
//	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
//	TIM6->CR2 |= (2<<4);
//	TIM6->CR1 |= TIM_CR1_ARPE;
//
//	TIM6->PSC = 3;
//	TIM6->ARR = 67;
//	TIM6->CR1 |= TIM_CR1_CEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->DIER |= TIM_DIER_UIE;
    //TIM6->CR1 &= ~TIM_CR1_DIR;
    TIM6->CR2 &= ~TIM_CR2_MMS;
    TIM6->CR2 |= TIM_CR2_MMS_1;
    //TIM6->DIER |= TIM_DIER_CC1IE;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->PSC = 3;
    TIM6->ARR= 67;

    NVIC->ISER[0] = 1<<TIM6_DAC_IRQn;
    TIM6->CR1 |= TIM_CR1_CEN;
}

