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


class Spi3
{
public :

	static void init()
	{
		/**
		 * PC10 used as SPI3 SCK
		 * PC11 used as SPI3 MISO
		 * PC12 used as SPI3 MOSI
		 * PB5 used as chip select
		 * PB15 used as SHDN
		 *
		 * PE5 used as EXTI 5
		 *
		 * DMA2 Channel 1 as RX
		 * DMA2 Channel 2 as TX
		 */

		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIOEEN | RCC_AHB2ENR_GPIOBEN;
		RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN;


		GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR10 | GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12);
		GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12);

		GPIOC->AFR[1] = ((GPIOC->AFR[1] & ~(GPIO_AFRH_AFSEL10 | GPIO_AFRH_AFSEL11 | GPIO_AFRH_AFSEL12))
				|((6u << GPIO_AFRH_AFSEL10_Pos) | (6u << GPIO_AFRH_AFSEL11_Pos) | (6u << GPIO_AFRH_AFSEL12_Pos)));
		GPIOC->MODER = ((GPIOB->MODER &~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12))
				|(GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1 | GPIO_MODER_MODE12_1));

		GPIOB->MODER = ((GPIOB->MODER & ~(GPIO_MODER_MODE15 | GPIO_MODER_MODE5 )) | (GPIO_MODER_MODE15_0 | GPIO_MODER_MODE5_0));
		GPIOB->BSRR = GPIO_BSRR_BR15;

		SPI3->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR | SPI_CR1_MSTR;
		SPI3->CR2 |= SPI_CR2_FRXTH | (0b111 << SPI_CR2_DS_Pos) | SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;

		//DMA RX
		DMA2_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_TCIE;
		DMA2_Channel1->CPAR = reinterpret_cast<uint32_t>(&SPI3->DR);

		DMA2_Channel2->CCR |= DMA_CCR_MINC | DMA_CCR_DIR;
		DMA2_Channel2->CPAR = reinterpret_cast<uint32_t>(&SPI3->DR);
		DMA2_CSELR->CSELR |= (3U << DMA_CSELR_C1S_Pos) | (3U << DMA_CSELR_C2S_Pos);

		kernel::Api::registerIrq(DMA2_Channel1_IRQn,dma2Interrupt);
		kernel::Api::irqPriority(DMA2_Channel1_IRQn,10);
		kernel::Api::enableIrq(DMA2_Channel1_IRQn);

		SPI3->CR1 |= SPI_CR1_SPE;


	}

	bool transfert( const uint8_t* sendingBuffer, uint8_t* receivingBuffer, uint16_t transfertSize)
	{
		if((DMA2_Channel1->CCR & DMA_CCR_EN) == DMA_CCR_EN || (DMA2_Channel2->CCR & DMA_CCR_EN) == DMA_CCR_EN)
			return false;

		GPIOB->BSRR = GPIO_BSRR_BR5;

		DMA2_Channel1->CMAR = reinterpret_cast<uint32_t>(receivingBuffer);
		DMA2_Channel1->CNDTR = transfertSize;
		DMA2_Channel1->CCR |= DMA_CCR_EN;

		DMA2_Channel2->CMAR = reinterpret_cast<uint32_t>(sendingBuffer);
		DMA2_Channel2->CNDTR = transfertSize;
		DMA2_Channel2->CCR |= DMA_CCR_EN;
		return true;
	}

	static Spi3& instance()
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
		GPIOB->BSRR = GPIO_BSRR_BS5;
		DMA2->IFCR |= DMA_IFCR_CGIF1 | DMA_IFCR_CGIF2;
		DMA2_Channel1->CCR &= ~DMA_CCR_EN;
		DMA2_Channel2->CCR &= ~DMA_CCR_EN;
		s_transfertComplete();
	}

	static void extiInterrupt()
	{

	}


	static Spi3 s_instance;
};



}	//drivers
}	//l475IotA
}	//discovery
}	//bsp
#endif
