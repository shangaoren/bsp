/*MIT License

Copyright (c) 2019 Florian GERARD

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


#ifdef STM32F746xx
#pragma once

#include <cstdint>
#include "../../../yggdrasil/interfaces/ISystem.hpp"
#include "../../../yggdrasil/kernel/Processor.hpp"

namespace bsp
{
namespace discovery
{
namespace f746g
{

class System : public interfaces::ISystem
{

public :
	//initialise clock of the board, maximum speed using internal oscillator
	bool initSystemClock(void) override
	{
		//prevent to initialise system a second time
		if(s_initialised)
			return false;
		else
			s_initialised = true;
		uint32_t timeout;


		//Set Flash Latency
		FLASH->ACR = ((FLASH->ACR & (~FLASH_ACR_LATENCY)) | (FLASH_ACR_LATENCY_7WS));
		if((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_7WS)
			return false;

		//Set Regulator voltage scaling
		PWR->CR1 = ((PWR->CR1 & ~(PWR_CR1_VOS)) | (PWR_CR1_VOS_0 | PWR_CR1_VOS_1));

		//Set over drive mode
		PWR->CR1 |= PWR_CR1_ODEN;

		//enable HSE clock
		RCC->CR |= RCC_CR_HSEON;

		timeout = 100;
		while((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}


		//enable LSI
		RCC->CSR |= RCC_CSR_LSION;
		timeout = 100;
		while((RCC->CSR & RCC_CSR_LSIRDY))
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		//enable access to backup domain
		PWR->CR1 |= PWR_CR1_DBP;
		//reset BackUpDomain
		RCC->BDCR |= RCC_BDCR_BDRST;
		//release BackupDomain
		RCC->BDCR &= ~RCC_BDCR_BDRST;

		RCC->PLLCFGR = ((RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP)) |
		             (RCC_PLLCFGR_PLLSRC_HSE | (RCC_PLLCFGR_PLLM_4 | RCC_PLLCFGR_PLLM_3 | RCC_PLLCFGR_PLLM_0) | 432 << RCC_PLLCFGR_PLLN_Pos ));


		//Enable PLL
		RCC->CR |= RCC_CR_PLLON;
		timeout = 100;
		while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		//Setup AHB Prescaler
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_HPRE)) | (RCC_CFGR_HPRE_DIV1));

		//SetupAPB1 Prescaler
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_PPRE1)) | (RCC_CFGR_PPRE1_DIV4));

		//Setup APB2 Prescaler
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_PPRE2)) | (RCC_CFGR_PPRE2_DIV2));

		//Configure system Clock Source
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_SW)) | (RCC_CFGR_SW_PLL));

		//wait for System Clock
		timeout = 100;
		while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		//Configure 48M domain
		//RCC->DCKCFGR2 = ((RCC->DCKCFGR2 &~(RCC_DCKCFGR2_CK48MSEL)) | (RCC_DCKCFGR2_CK48MSEL));

		s_systemCoreClock = 216000000;
		s_peripheralClock1 = s_systemCoreClock/4;
		s_peripheralClock2 = s_systemCoreClock/2;
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
} /* namespace f746g */
} /* namespace discovery */
} /* namespace boards */
#endif /*  STM32F746xx */

