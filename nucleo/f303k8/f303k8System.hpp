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


#ifdef STM32F303x8
#pragma once

#include <cstdint>
#include "yggdrasil/kernel/Processor.hpp"
#include "yggdrasil/interfaces/ISystem.hpp"

namespace bsp
{
namespace nucleo
{
namespace f303k8
{

class System : public interface::ISystem
{


public:
	//initialise clock of the board, maximum speed using 8Mhz external crystal
	bool initSystemClock(void) override
	{
		//prevent to initialise system a second time
		if(s_initialised)
			return true;

		uint32_t timeout = 100;

		FLASH->ACR = ((FLASH->ACR & ~(FLASH_ACR_LATENCY)) | (FLASH_ACR_LATENCY_1));

		if((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_1)
			return false;

		RCC->CR |= RCC_CR_HSEBYP;
		RCC->CR |= RCC_CR_HSEON;

		while((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY)
		{
			timeout --;
			if(timeout == 0)
				return false;
		}
		

		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL)) | (RCC_CFGR_PLLSRC_HSE_PREDIV | RCC_CFGR_PLLMUL9));
		RCC->CFGR2 = ((RCC->CFGR2 & ~(RCC_CFGR2_PREDIV)) | (RCC_CFGR2_PREDIV_DIV1));

		RCC->CR |= RCC_CR_PLLON;

		timeout = 100;
		while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_HPRE)) | (RCC_CFGR_HPRE_DIV1));
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_PPRE1)) | (RCC_CFGR_PPRE1_DIV2));
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_PPRE2)) | (RCC_CFGR_PPRE2_DIV1));

		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_SW)) | (RCC_CFGR_SW_PLL));

		timeout = 100;
		while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		s_systemCoreClock = 72000000;
		s_peripheralClock1 = 36000000;
		s_peripheralClock2 = 72000000;
		s_initialised = true;
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
	static bool s_initialised;
};

}
}
}

#endif /* STM32F3 */

