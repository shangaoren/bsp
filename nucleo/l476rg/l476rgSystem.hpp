/*
 * l476rgSystem.hpp
 *
 *  Created on: 26 sept. 2018
 *      Author: Flo
 */

#pragma once
#ifdef STM32L476xx
#include "yggdrasil/kernel/Processor.hpp"
#include "yggdrasil/interfaces/ISystem.hpp"
#include <stdint.h>

namespace bsp
{
namespace nucleo
{
namespace l476rg
{

class System : public interface::ISystem
{
public:
	//initialise clock of the board, maximum speed using internal MSI Clock
	bool initSystemClock(void) override
	{
		//prevent to initialise system a second time
		if(s_initialised)
			return false;
		else
			s_initialised = true;

		uint32_t timeout = 100;
		FLASH->ACR = ((FLASH->ACR & ~(FLASH_ACR_LATENCY)) | (FLASH_ACR_LATENCY_4WS));
		if((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS)
			return false;
		PWR->CR1 = ((PWR->CR1 & ~(PWR_CR1_VOS)) | (PWR_CR1_VOS_0));
		RCC->CR |= RCC_CR_HSION;
		while((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		RCC->ICSCR = ((RCC->ICSCR & ~(RCC_ICSCR_HSITRIM)) | (16<<RCC_ICSCR_HSITRIM_Pos));
		RCC->PLLCFGR = ((RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLR)) | (RCC_PLLCFGR_PLLSRC_HSI | 0U << RCC_PLLCFGR_PLLM_Pos | 10U << RCC_PLLCFGR_PLLN_Pos | 0U << RCC_PLLCFGR_PLLR_Pos));
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
		RCC->CR |= RCC_CR_PLLON;
		timeout = 100;
		while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_SW)) | (RCC_CFGR_SW_PLL));

		timeout = 100;
		while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		{
			timeout--;
			if(timeout == 0)
				return false;
		}

		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_HPRE))  | (RCC_CFGR_HPRE_DIV1));
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_PPRE1)) | (RCC_CFGR_PPRE1_DIV1));
		RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_PPRE2)) | (RCC_CFGR_PPRE2_DIV1));


		s_systemCoreClock = 80000000;
		s_peripheralClock1 = 80000000;
		s_peripheralClock2 = 80000000;
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

} /* namespace l476rg */
} /* namespace nucleo */
} /* namespace bsp */
#endif

