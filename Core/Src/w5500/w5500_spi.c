/*
 * w550_spi.c
 *
 *  Created on: Dec 29, 2023
 *      Author: chisc
 */


#include "stm32f0xx_hal.h"
#include "wizchip_conf.h"
#include "stdio.h"

extern SPI_HandleTypeDef hspi1;// handle to declare SPI 1

// main transfer function
uint8_t SPIReadWrite(uint8_t data)
{
	//wait untill FIFO has a free slot
	while((hspi1.Instance->SR & SPI_FLAG_TXE) != SPI_FLAG_TXE);

	*(__IO uint8_t *)&hspi1.Instance->DR = data;

	//now wait untill data arrives
	while((hspi1.Instance->SR & SPI_FLAG_RXNE) != SPI_FLAG_RXNE);

	return (*(__IO uint8_t *)&hspi1.Instance->DR);

}

void wizchip_select(void) // by pulling pin low
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

void wizchip_deselect(void)// by pulling pin high
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
}


uint8_t wizchip_read()
{
	uint8_t rb;
	rb = SPIReadWrite(0x00);
	return rb;
}


void wizchip_write(uint8_t wb)
{
	SPIReadWrite(wb);
}

//burst functions, they read and wri9te multiple bytes
void wizchip_readburst(uint8_t * pBuf, uint16_t len)
{
	for(uint16_t i = 0 ; i < len; i++)
	{
		*pBuf = SPIReadWrite(0x00);
		pBuf++;
	}
}

void wizchip_writeburst(uint8_t * pBuf, uint16_t len)
{
	for(uint16_t i = 0 ; i < len; i++)
	{
		SPIReadWrite(*pBuf);
		pBuf++;
	}
}

//private function, that's only called here
void W5500IOInit()
{
	/*
	 * Initialize the two GPIO pins
	 * Reset -> PC8
	 * and
	 * CS->PA1
	 */
	GPIO_InitTypeDef GPIO_InitStructA = {0};
	GPIO_InitTypeDef GPIO_InitStructC = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStructA.Pin = GPIO_PIN_1;
	GPIO_InitStructC.Pin = GPIO_PIN_8;

	GPIO_InitStructA.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructC.Mode = GPIO_MODE_OUTPUT_PP;

	GPIO_InitStructA.Pull = GPIO_NOPULL;
	GPIO_InitStructC.Pull = GPIO_NOPULL;

	GPIO_InitStructA.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructC.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStructA);
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructC);

}

void W5500Init()
{
	/* Reset -> PC8
		 * and
		 * CS->PA1
		 */
	uint8_t tmp;              // supporting 8 sockets simultaneously per w5500 DS
					// and deviding the memory ~32kb for all 8 note; you don't need all 8
					// both side have mmemeory for tx and rx
	uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };

	W5500IOInit();

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);//CS high by default

	//Send a pulse on reset pin
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
	tmp = 0xFF;
	while(tmp--);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);

	//Attach low level call back functions with the drivers
	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
	reg_wizchip_spiburst_cbfunc(wizchip_readburst, wizchip_writeburst);

	/* WIZChip Initialize*/
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1) {
		printf("WIZCHIP Initialization Failed.\r\n");
		while (1);
	}
	printf("WIZCHIP Initialization Success.\r\n");
}
