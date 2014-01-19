//!
//! @file 				Port.hpp
//! @author 			Geoffrey Hunter <gbmhunter@gmail.com> (www.cladlab.com)
//! @edited 			n/a
//! @created			2012/10/04
//! @last-modified 	2014/01/14
//! @brief 				Contains port specific functions for the SSD1306 library.
//! @details
//!						See the README in the repo root dir for more info.

//===============================================================================================//
//======================================= HEADER GAURD ==========================================//
//===============================================================================================//

// Header guard 
#ifndef PORT_H
#define PORT_H

namespace SSD1306Ns
{

	//===============================================================================================//
	//======================================= PUBLIC DEFINES ========================================//
	//===============================================================================================//
	
	//! @brief		Determines what code is used for the port-specific functions.
	//! @details	Supported platforms:
	//!					PSOC	All PSoC families (PSoC 3, PSoC 4, PSoC 5, PSoC 5LP)
	#define MCU_PLATFORM		PSOC

	//! @brief 		Set to 1 if SA0 is pulled low
	#define SSD1306_I2C_SA0_0 	1		
	
	//! @brief 		Set to 1 is SA0 is pulled high
	#define SSD1306_I2C_SA0_1 	0		
	
	//===============================================================================================//
	//=========================================== CLASS =============================================//
	//===============================================================================================//

	//! @brief 		Contains port specific functions for the SSD1306 library.
	class Port
	{
		public:
		
			typedef enum
			{
				i2cLeft,
				i2cRight
			} i2cPort_t;
			
			i2cPort_t i2cPort;
		
			//! @brief		Constructor.
			Port();
		
			//! @brief		Initialises the I2C interface.
			void I2cStart();
			
			//! @brief		Enables I2C interrupts. May not be needed on some
			//!				platforms.
			void I2cEnableInt();
			
			//! @brief		Sends a start signal on the I2C interface.
			uint8 I2cMasterSendStart(uint8 slaveAddress, uint8 readWrite);
			
			//! @brief		Writes a byte across the I2C interface.
			uint8 I2cMasterWriteByte(uint8 byteToWrite);
			
			//! @brief		Sends stop signal on I2C interface.
			uint8 I2cMasterSendStop();
			
			//! @brief		Delays processing for a certain amount of time (in micro-seconds).
			//! @details	This can either be a hard wait (stalls the processor), or a soft wait
			//!				(performs a context-switch, only applicable if using an OS).
			//!				Used by SSD1306::Reset().
			void DelayUs(uint16 delayTimeUs);
			
			//! @brief		Delays processing for a certain amount of time (in milli-seconds).
			//! @details	This can either be a hard wait (stalls the processor), or a soft wait
			//!				(performs a context-switch, only applicable if using an OS).
			void DelayMs(uint16 delayTimeMs);
			
			//! @brief		Pulls the reset pin low, putting SSD1306 into reset
			//! @details	
			void PullResetLow();
			
			//! @brief		Pulls the reset pin high, taking SSD1306 out of reset
			void PullResetHigh();
			
			//! @brief		Prints debug information to an output (typically a UART).
			void PrintDebug(const char* msg);
	
	};
	
} // namespace SSD1306Ns

#endif // #ifndef PORT_H

// EOF
