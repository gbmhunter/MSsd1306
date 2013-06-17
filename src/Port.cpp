//!
//! @file 		Port.c
//! @author 	Geoffrey Hunter <gbmhunter@gmail.com>
//! @edited 	n/a
//! @date 		2013/06/11
//! @brief 		Contains port specific functions for the SSD1306 library.
//! @details
//!				See README.rst

//===============================================================================================//
//========================================== INCLUDES ===========================================//
//===============================================================================================//

#include "Config.h"
#if(configINCLUDE_CAP_SENSE == 1)

#ifdef __cplusplus
extern "C" {
#endif
	// PSoC includes
	#include <device.h>
#ifdef __cplusplus
}
#endif

// User includes
#include "PublicObjects.h"
#include "./CapSense/include/CapSense.h"
#include "./SSD1306/include/SSD1306.h"
#include "./UartDebug/include/UartDebug.h"


//===============================================================================================//
//===================================== PRE-COMPILER CHECKS =====================================//
//===============================================================================================//

	
//===============================================================================================//
//========================================== DEFINES ============================================//
//===============================================================================================//

namespace SSD1306Ns
{

	//===============================================================================================//
	//===================================== GLOBAL FUNCTIONS ========================================//
	//===============================================================================================//

	// See Doxygen documentation or function declarations in Port.h for more info.

	Port::Port()
	{
		// Constructor
		
		#if(MCU_PLATFORM == PSOC)
			i2cPort = i2cLeft;
		#else
			#warning No port-specific code for Port::Port() 
		#endif
	}
	

	// I2C Wrapper Functions

	//! @private
	void Port::I2cStart()
	{
		#if(MCU_PLATFORM == PSOC)
			I2cCpLeft_Start();
			I2cCpRight_Start();
		#else
			#warning No port-specific code for Port::I2cStart() 
		#endif
	}

	//! @private
	void Port::I2cEnableInt()
	{
		#if(MCU_PLATFORM == PSOC)
			I2cCpLeft_EnableInt();
			I2cCpRight_EnableInt();
		#else
			#warning No port-specific code for Port::I2cEnableInt() 
		#endif
	}

	//! @details	Supports two I2C ports
	//! @private
	uint8 Port::I2cMasterSendStart(uint8 slaveAddress, uint8 readWrite)
	{
		#if(MCU_PLATFORM == PSOC)
			if(i2cPort == i2cLeft)
				return I2cCpLeft_MasterSendStart(slaveAddress, readWrite);
			else if(i2cPort == i2cRight)
				return I2cCpRight_MasterSendStart(slaveAddress, readWrite);
			else
				return 0;
		#else
			#warning No port-specific code for Port::I2cMasterSendStart() 
		#endif
	}

	//! @details	Supports two I2C ports
	//! @private
	uint8 Port::I2cMasterWriteByte(uint8 byteToWrite)
	{
		#if(MCU_PLATFORM == PSOC)
			if(i2cPort == i2cLeft)
				return I2cCpLeft_MasterWriteByte(byteToWrite);
			else if(i2cPort == i2cRight)
				return I2cCpRight_MasterWriteByte(byteToWrite);
			else
				return 0;
		#else
			#warning No port-specific code for Port::I2cMasterWriteByte() 
		#endif
	}

	//! @details	Supports two I2C ports
	//! @private
	uint8 Port::I2cMasterSendStop()
	{
		#if(MCU_PLATFORM == PSOC)
			if(i2cPort == i2cLeft)
				return I2cCpLeft_MasterSendStop();
			else if(i2cPort == i2cRight)
				return I2cCpRight_MasterSendStop();
			else
				return 0;
		#else
			#warning No port-specific code for Port::I2cMasterSendStop() 
		#endif
	}


	//! @brief		I/O wrapper function. Pulls the reset pin low, putting SSD1306 into reset
	//! @details	
	//! @public
	void Port::PullResetLow()
	{
		#if(MCU_PLATFORM == PSOC)
			if(i2cPort == i2cLeft)
			{
				CapSense_SetCapDevice(LEFT_LIN);
				CapSense_SetGpio(LOW);
			}
			else if(i2cPort == i2cRight)
			{
				CapSense_SetCapDevice(RIGHT_LIN);
				CapSense_SetGpio(LOW);
			}
		#else
			#warning No port-specific code for Port::PullResetLow() 
		#endif
	}

	void Port::PullResetHigh()
	{
		#if(MCU_PLATFORM == PSOC)
			if(i2cPort == i2cLeft)
			{
				CapSense_SetCapDevice(LEFT_LIN);
				CapSense_SetGpio(HIGH);
			}
			else if(i2cPort == i2cRight)
			{
				CapSense_SetCapDevice(RIGHT_LIN);
				CapSense_SetGpio(HIGH);
			}
		#else
			#warning No port-specific code for Port::PullResetHigh() 
		#endif
	}

	//! @brief		Wrapper function for micro-second delay
	//! @private
	void Port::DelayUs(uint16 delayTimeUs)
	{
		#if(MCU_PLATFORM == PSOC)
			// Call Cypress API function
			CyDelayUs(delayTimeUs);
		#else
			#warning No port-specific code for Port::DelayUs() 
		#endif
	}


	//! @brief		Wrapper function for milli-second delay
	//! @private
	void Port::DelayMs(uint16 delayTimeMs)
	{
		#if(MCU_PLATFORM == PSOC)
			// Call milli-second delay Cypress API function
			CyDelay(delayTimeMs);
		#else
			#warning No port-specific code for Port::DelayMs() 
		#endif
	}
	
	void Port::PrintDebug(const char* msg)
	{
		#if(MCU_PLATFORM == PSOC)
			UartDebug_PutString(msg);
		#else
			#warning No port-specific code for Port::PrintDebug() 
		#endif
	}


	//===============================================================================================//
	//============================================ GRAVEYARD ========================================//
	//===============================================================================================//


} // namespace SSD1306Ns


#endif	// #if(configINCLUDE_CAP_SENSE == 1)

// EOF
