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
#include "yggdrasil/kernel/Processor.hpp"
#include "yggdrasil/interfaces/IStream.hpp"
#include "yggdrasil/kernel/Api.hpp"
#include "bsp/discovery/l475IotA/l475IotASystem.hpp"

namespace bsp
{
namespace discovery
{
namespace l475IotA
{
namespace drivers
{
class Usart1 : public interface::IStream<uint8_t>
{
public:
	static void init(uint32_t baudRate)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

		GPIOB->MODER = ((GPIOB->MODER & ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7)) | (GPIO_MODER_MODE7_1 | GPIO_MODER_MODE6_1));
		GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7);
		GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7);
		GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);
		GPIOB->AFR[0] = ((GPIOB->AFR[0] & ~(GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7)) | ((7u << GPIO_AFRL_AFSEL6_Pos) | (7u << GPIO_AFRL_AFSEL7_Pos)));


		USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;
		USART1->BRR = System::instance().getPeripheralClock2Frequency()/baudRate;
		USART1->ISR |= USART_ISR_IDLE;
		USART1->CR1 |= /*USART_CR1_IDLEIE |*/ USART_CR1_TE /*| USART_CR1_RE*/ | USART_CR1_UE;

		DMA1_Channel4->CCR |= DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_TCIE | DMA_CCR_DIR;
		DMA1_Channel4->CPAR = reinterpret_cast<uint32_t>(&USART1->TDR);

		//DMA1_Channel5->CCR |= DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_TCIE;
		//DMA1_Channel5->CPAR = reinterpret_cast<uint32_t>(&USART1->RDR);
		DMA1_CSELR->CSELR |= (2 << DMA_CSELR_C5S_Pos) | (2 << DMA_CSELR_C4S_Pos);

		kernel::Api::registerIrq(DMA1_Channel4_IRQn,dataTransmitted);
		kernel::Api::irqPriority(DMA1_Channel4_IRQn,2,1);
		kernel::Api::clearIrq(DMA1_Channel4_IRQn);
		kernel::Api::enableIrq(DMA1_Channel4_IRQn);
	}

	static void dataTransmitted()
	{
		DMA1->IFCR |= DMA_IFCR_CGIF4;
		DMA1_Channel4->CCR &= ~DMA_CCR_EN;
	}

	bool beginSend(const uint8_t* data, uint16_t size)
	{
		if((DMA1_Channel4->CCR & DMA_CCR_EN) == DMA_CCR_EN)
			return false;
		DMA1_Channel4->CMAR = reinterpret_cast<uint32_t>(data);
		DMA1_Channel4->CNDTR = size;
		DMA1_Channel4->CCR |= DMA_CCR_EN;
		return true;
	}
	bool endSend(int64_t timeout)
	{
		return false;
	}

	void receive(uint8_t* data, uint16_t size, bool eof)
	{

	}

	static Usart1& instance()
	{
		return s_instance;
	}

private:
	static Usart1 s_instance;
};
}	/* drivers*/
}	/* l475IotA*/
}	/* discovery */
}	/* bsp */



#endif /* STM32L475xx */
