/*MIT License

Copyright (c) 2018 Florian GERARD

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Except as contained in this notice, the name of Florian GERARD shall not be used 
in advertising or otherwise to promote the sale, use or other dealings in this 
Software without prior written authorization from Florian GERARD

*/


#ifdef STM32L475xx
#pragma once

#include <cstdint>
#include <functional>
#include "yggdrasil/kernel/Processor.hpp"
#include "yggdrasil/kernel/Api.hpp"

namespace bsp
{
namespace discovery
{
namespace l475IotA
{
namespace drivers
{


class Spi1
{
public :

	static void init()
	{
		/**
		 * PA5 used as SPI1 SCK
		 * PA6 used as SPI1 MISO
		 * PA7 used as SPI1 MOSI
		 * PA2 used as chip select
		 * PD14 used as SHDN
		 *
		 *
		 *
		 * DMA2 Channel 3 as RX
		 * DMA2 Channel 4 as TX
		 */

		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIODEN;
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;


		GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR5 | GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7);
		GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);

		GPIOA->AFR[0] = ((GPIOA->AFR[0] & ~(GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7))
				|((5u << GPIO_AFRL_AFSEL5_Pos) | (5u << GPIO_AFRL_AFSEL6_Pos) | (5u << GPIO_AFRL_AFSEL7_Pos)));
		GPIOA->MODER = ((GPIOA->MODER &~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7))
				|(GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1));

		GPIOD->MODER = ((GPIOD->MODER & ~(GPIO_MODER_MODE14)) | (GPIO_MODER_MODE14_0));
		GPIOD->BSRR = GPIO_BSRR_BR14;

		SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR | SPI_CR1_MSTR;
		SPI1->CR2 |= SPI_CR2_FRXTH | (0b111 << SPI_CR2_DS_Pos) | SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;

		//DMA RX
		DMA2_Channel3->CCR |= DMA_CCR_MINC | DMA_CCR_TCIE;
		DMA2_Channel3->CPAR = reinterpret_cast<uint32_t>(&SPI1->DR);

		DMA2_Channel4->CCR |= DMA_CCR_MINC | DMA_CCR_DIR;
		DMA2_Channel4->CPAR = reinterpret_cast<uint32_t>(&SPI1->DR);
		DMA2_CSELR->CSELR |= (4U << DMA_CSELR_C3S_Pos) | (4U << DMA_CSELR_C4S_Pos);

		kernel::Api::registerIrq(DMA2_Channel3_IRQn,dma2Interrupt);
		kernel::Api::irqPriority(DMA2_Channel3_IRQn,10);
		kernel::Api::enableIrq(DMA2_Channel3_IRQn);

		SPI1->CR1 |= SPI_CR1_SPE;


	}

	bool transfert( const uint8_t* sendingBuffer, uint8_t* receivingBuffer, uint16_t transfertSize)
	{
		if((DMA2_Channel3->CCR & DMA_CCR_EN) == DMA_CCR_EN || (DMA2_Channel4->CCR & DMA_CCR_EN) == DMA_CCR_EN)
			return false;

		GPIOA->BSRR = GPIO_BSRR_BR2;

		DMA2_Channel3->CMAR = reinterpret_cast<uint32_t>(receivingBuffer);
		DMA2_Channel3->CNDTR = transfertSize;
		DMA2_Channel3->CCR |= DMA_CCR_EN;

		DMA2_Channel4->CMAR = reinterpret_cast<uint32_t>(sendingBuffer);
		DMA2_Channel4->CNDTR = transfertSize;
		DMA2_Channel4->CCR |= DMA_CCR_EN;
		return true;
	}

	static Spi1& instance()
	{
		return s_instance;
	}

	static void onTransfertComplete(std::function<void(void)> callback)
	{
		s_transfertComplete = callback;
	}

	static void onReceiveExti(std::function<void(void)> callback)
	{
		s_receiveInterrupt = callback;
	}
private :

	static std::function<void(void)> s_transfertComplete;
	static std::function<void(void)> s_receiveInterrupt;

	static void dma2Interrupt()
	{
		GPIOA->BSRR = GPIO_BSRR_BS2;
		DMA2->IFCR |= DMA_IFCR_CGIF3 | DMA_IFCR_CGIF4;
		DMA2_Channel3->CCR &= ~DMA_CCR_EN;
		DMA2_Channel4->CCR &= ~DMA_CCR_EN;
		s_transfertComplete();
	}

	static void extiInterrupt()
	{

	}


	static Spi1 s_instance;
};



}	//drivers
}	//l475IotA
}	//discovery
}	//bsp
#endif
