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

#pragma once
#ifdef STM32L432xx
#include <cstdint>
#include "yggdrasil/interfaces/ISystem.hpp"
#include "yggdrasil/processor/st/l4/stm32l432xx.h"

namespace bsp
{
namespace nucleo
{
namespace l432kc
{

class System : public interfaces::ISystem
{
public:

	bool initSystemClock() override
	{
		//test if was already initialized
		if(s_initialized != false)
			return true;

		
		uint32_t timeout;
		FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_4WS;
		if ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS)
			return false;;
		PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | PWR_CR1_VOS_0;
		RCC->CR |= RCC_CR_HSION;

		timeout = 100;
		while ((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY)
		{
			timeout -= 1;
			if(timeout == 0)
				return false;
		}

		RCC->ICSCR = (RCC->ICSCR & ~RCC_ICSCR_HSITRIM) | (16 << RCC_ICSCR_HSITRIM_Pos);
		RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLR)) | (RCC_PLLCFGR_PLLSRC_HSI | 0 | 10 << RCC_PLLCFGR_PLLN_Pos | 0);
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
		RCC->CR |= RCC_CR_PLLON;

		timeout = 100;
		while ((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY)
		{
			timeout -= 1;
			if(timeout == 0)
				return false;
		}

		RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2 | RCC_CFGR_SW)) | (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1 |  RCC_CFGR_SW_PLL);

		timeout = 100;
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		{
			timeout -= 1;
			if(timeout == 0)
				return false;
		}

		s_systemCoreClock = 80000000;
		s_peripheralClock1 = 80000000;
		s_peripheralClock2 = 80000000;
		s_initialized = true; //initialisation finished without errors
		return true;
	}


	uint32_t getSystemCoreFrequency() override
	{
		return s_systemCoreClock;
	}

	uint32_t getPeripheralClock1Frequency() override
	{
		return s_peripheralClock1;
	}

	uint32_t getPeripheralClock2Frequency() override
	{
		return s_peripheralClock2;
	}

	static System& instance()
	{
		return s_instance;
	}

private:
	static uint32_t s_systemCoreClock;
	static uint32_t s_peripheralClock1;
	static uint32_t s_peripheralClock2;

	static System s_instance;
	static bool s_initialized;
};


} /* namespace l432kc */
} /* namespace nucleo */
} /* namespace boards */

#endif
