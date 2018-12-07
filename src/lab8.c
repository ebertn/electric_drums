#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void nano_wait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

static short dispmem[] = {
        0x002,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x0c0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};

//===========================================================================
// Send a command to the LCD.
void cmd(char b) {
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = b;
}

//===========================================================================
// Send a character to the LCD.
void data(char b) {
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = 0x200 | b;
}

//===========================================================================
// Initialize the LCD.
void init_lcd_spi(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER |= 0X8A000000;

	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	SPI2->CR1 |= SPI_CR1_MSTR;
	SPI2->CR1 |= 0x8000;
	SPI2->CR1 |= 0x4000;
	SPI2->CR1 |= 0x7<<3;
	SPI2->CR1 &= 0xFFFD;
	SPI2->CR1 &= 0xFFFE;
	SPI2->CR2 = (0x9)<< 8;
	SPI2->CR2 |= 0x4;
	SPI2->CR2 |= 0x8;
	SPI2->CR1 |= SPI_CR1_SPE;


	nano_wait(100000000); // Give it 100ms to initialize
	cmd(0x38); // 0011 NF00 N=1, F=0: two lines
	cmd(0x0c); // 0000 1DCB: display on, no cursor, no blink
	cmd(0x01); // clear entire display
	nano_wait(6200000); // clear takes 6.2ms to complete
	cmd(0x02); // put the cursor in the home position
	cmd(0x06); // 0000 01IS: set display to increment

}

//===========================================================================
// Initialize the LCD to use circular DMA to write to the SPI channel.
void init_lcd(void) {
	init_lcd_spi();

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;
	DMA1_Channel5->CMAR = (uint32_t) dispmem;
	DMA1_Channel5->CPAR = (uint32_t) &SPI2->DR;
	DMA1_Channel5->CNDTR = 34;
	DMA1_Channel5->CCR |= 0x100;
	DMA1_Channel5->CCR |= 0x400;
	DMA1_Channel5->CCR |= DMA_CCR_DIR;
	DMA1_Channel5->CCR &= ~DMA_CCR_HTIE;
	DMA1_Channel5->CCR |= DMA_CCR_MINC;
	DMA1_Channel5->CCR &= ~DMA_CCR_PL;
	DMA1_Channel5->CCR |= DMA_CCR_CIRC;
	SPI2->CR2 |= 0x80;
	SPI2->CR2 |= 0x2;
	DMA1_Channel5->CCR |= DMA_CCR_EN;
}

//===========================================================================
// Display a string on line 1 by writing to SPI directly.
void display1_spi(const char *s) {
    cmd(0x02); // put the cursor on the beginning of the first line.
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//===========================================================================
// Display a string on line 2 by writing to SPI directly.
void display2_spi(const char *s) {
    cmd(0xc0); // put the cursor on the beginning of the second line.
    int x;
    for(x=0; x<16; x+=1)
        if (s[x] != '\0')
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//===========================================================================
// Display a string on line 1 by using DMA to write to SPI.
void display1_dma(const char *s) {
	 cmd(0x02); // put the cursor on the beginning of the first line.
	 int x;
	 for(x=0; x<16; x+=1)
		 if (s[x])
	     {
	    	 dispmem[x+1]= s[x] | 0x200;
	     }

	     else
	         break;
	 for(   ; x<16; x+=1)
	 {
	     dispmem[x+1]=0x220;
	 }

	 RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	 DMA1_Channel5->CCR &= ~DMA_CCR_EN;
	 DMA1_Channel5->CMAR = (uint32_t) dispmem;
	 DMA1_Channel5->CPAR = (uint32_t) &SPI2->DR;
	 DMA1_Channel5->CNDTR = 17;
	 DMA1_Channel5->CCR |= 0x100;
	 DMA1_Channel5->CCR |= 0x400;
	 DMA1_Channel5->CCR |= DMA_CCR_DIR;
	 DMA1_Channel5->CCR &= ~DMA_CCR_HTIE;
	 DMA1_Channel5->CCR |= DMA_CCR_MINC;
	 DMA1_Channel5->CCR &= ~DMA_CCR_PL;
	 SPI2->CR2 |= 0x80;
	 SPI2->CR2 |= 0x2;
	 DMA1_Channel5->CCR |= DMA_CCR_EN;

}


//===========================================================================
// Display a string on line 2 by using DMA to write to SPI.
void display2_dma(const char *s) {
	cmd(0x02); // put the cursor on the beginning of the first line.
		 int x;
		 for(x=0; x<16; x+=1)
			 if (s[x])
		     {
		    	 dispmem[x+18]= s[x] | 0x200;
		     }

		     else
		         break;
		 for(   ; x<16; x+=1)
		 {
		     dispmem[x+1]=0x220;
		 }

		 RCC->AHBENR |= RCC_AHBENR_DMA1EN;
		 DMA1_Channel5->CCR &= ~DMA_CCR_EN;
		 DMA1_Channel5->CMAR = (uint32_t) dispmem;
		 DMA1_Channel5->CPAR = (uint32_t) &SPI2->DR;
		 DMA1_Channel5->CNDTR = 17;
		 DMA1_Channel5->CCR |= 0x100;
		 DMA1_Channel5->CCR |= 0x400;
		 DMA1_Channel5->CCR |= DMA_CCR_DIR;
		 DMA1_Channel5->CCR &= ~DMA_CCR_HTIE;
		 DMA1_Channel5->CCR |= DMA_CCR_MINC;
		 DMA1_Channel5->CCR &= ~DMA_CCR_PL;
		 SPI2->CR2 |= 0x80;
		 SPI2->CR2 |= 0x2;
		 DMA1_Channel5->CCR |= DMA_CCR_EN;

}

//===========================================================================
// Display a string on line 1 by writing to the DMA sorrce.
void display1(const char *s) {
	cmd(0x02); // put the cursor on the beginning of the first line.
		 int x;
		 for(x=0; x<16; x+=1)
			 if (s[x])
		     {
		    	 dispmem[x+1]= s[x] | 0x200;
		     }

		     else
		         break;
		 for(   ; x<16; x+=1)
		 {
		     dispmem[x+1]=0x220;
		 }
}

//===========================================================================
// Display a string on line 2 by writing to the DMA source.
void display2(const char *s) {
	cmd(0x02); // put the cursor on the beginning of the first line.
			 int x;
			 for(x=0; x<16; x+=1)
				 if (s[x])
			     {
			    	 dispmem[x+18]= s[x] | 0x200;
			     }

			     else
			         break;
			 for(   ; x<16; x+=1)
			 {
			     dispmem[x+18]=0x220;
			 }
}
void TIM2_IRQHandler(void) {
	static int seconds = 0;
	static int minutes = 0;
	static int hours = 0;
	if ((TIM2->SR & TIM_SR_UIF) != 0) {
			TIM2->SR &= ~TIM_SR_UIF;
			//return;
	}
	seconds += 1;

	if (seconds > 59) {
			seconds = 0;
			minutes += 1;
	}
	if (minutes > 59) {
			minutes = 0;
			hours += 1;
	}
	char line[20];
	sprintf(line, "Waiting %02d:%02d:%02d", hours, minutes, seconds);
	display1(line);
}
void init_tim2(void) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->CCER |= TIM_CCER_CC2E;
	TIM2->PSC = 48000;
	TIM2->ARR= 1000 ;
	NVIC->ISER[0] = 1<<TIM2_IRQn;
	TIM2->CR1 |= TIM_CR1_CEN;

}
