//!
//! @file 				Ssd1306.h
//! @author 			Geoffrey Hunter <gbmhunter@gmail.com> (www.cladlab.com)
//! @edited 			n/a
//! @created			2012/10/04
//! @last-modified 	2014/01/14
//! @brief 				Driver for the SDD1306 OLED segment/common controller.
//! @details
//!						See the README in the repo root dir for more info.

//===============================================================================================//
//======================================= HEADER GAURD ==========================================//
//===============================================================================================//


// Header guard 
#ifndef SSD1306_H
#define SSD1306_H

#include "./SSD1306/include/Port.h"

namespace SSD1306Ns
{

	//===============================================================================================//
	//======================================= PUBLIC DEFINES ========================================//
	//===============================================================================================//
	
	//! @brief		Number of columns of display device connected to SSD1306.
	#define ssd1306NUM_COLS				128
	
	//! @brief		Number of rows of display device connected to SSD1306.
	//! @note		Currenlty not used in any code.
	#define ssd1306NUM_ROWS				32
	
	//! @brief		Determines the max brightness used by the FadeIn() and FadeOut() functions.
	#define	ssd1306BRIGHTNESS			0x8F
	
	//! @brief		Enables/disables fonts. 
	//! @details	As fonts a decent amount of flash memory, there is
	//!				the option to disables fonts. If fonts are disabled, you can't use any
	//! 			font-related functions.
	#define ssd1306ENABLE_FONTS			1

	//! @brief		Sent over I2C to indiciate to the SSD1306 IC that a command follows.
	//! @details	This is the "control" byte.
	#define ssd1306CMD_FOLLOWS_BYTE 	0x00
	
	//! @brief		Sent over I2C to indiciate to the SSD1306 IC that data follows.
	//! @details	This is the "control" byte.
	#define ssd1306DATA_FOLLOWS_BYTE 	0x40


	//! @brief		Main class for the SSD1306 driver.
	class SSD1306
	{
		public:
		
			//===============================================================================================//
			//======================================== PUBLIC TYPEDEFS ======================================//
			//===============================================================================================//
		
			//! @brief		Used as an input to SetAddressingMode().
			typedef enum
			{
				HORIZONTAL_ADDRESSING_MODE 	= 0x00,
				VERTICAL_ADDRESSING_MODE 	= 0x01,
				PAGE_ADDRESSING_MODE 		= 0x02
			} addressingMode_t;
		
			//===============================================================================================//
			//================================== PUBLIC METHOD DECLARATIONS =================================//
			//===============================================================================================//
		
			//! @brief		Initialises OLED screen with common settings when Vcc supplied internally.
			void OledInitVccInt();
		
			//! @brief		Initialises OLED screen with common settings when Vcc supplied externally.
			void OledInitVccExt();
		
			//! @public
			void EnableI2c();

			//! @brief		Sets the I2C port to use for communication
			//! @details	Module supports multiple SSD1306 IC communication
			//!				on one or more I2C port(s)
			//! @public
			void SetI2cPort(Port::i2cPort_t i2cPort);

			//! @brief		Enables power to the Vddb line.
			//! @details	P-ch MOSFET controls power to pin.
			//! @sa			DisableVddb()
			//! @public
			void EnableVddb();

			//! @brief		Disables power to the Vddb line
			//! @details	P-ch MOSFET controls power to pin.
			//! @sa			void EnableVddb()
			//! @public
			void DisableVddb();

			//! @brief		Resets the SSD1306 IC by pulling the reset line low.
			//! @details	Includes a processor stalling delay before pulling
			//!				the chip back out of reset.
			//! @public
			void Reset();

			//! @brief		Enables the internal charge pump
			//! @details	0x10 => default, 0x10 => disable, 0x14 => enable
			//! @public
			void SetChargePumpOn();

			//! @brief		Disables the internal charge pump
			//! @details	0x10 => default, 0x10 => disable, 0x14 => enable
			//! @public
			void SetChargePumpOff();

			//! @brief		Turns the display on. Used for sleep purposes.
			//! @details	Set Display On/Off
			//!				Default => 0xAE
			//!				0xAE => Display Off
			//! 			0xAF => Display On
			//! @public
			void ActivateDisplay();

			//! @brief		Turns the display off. Used for sleep purposes.
			//! @details	Set Display On/Off
			//!				Default => 0xAE
			//!				0xAE => Display Off
			//! 			0xAF => Display On
			//! @public
			void DeactivateDisplay();

			//! @brief		Turns every pixel on, ignoring the contents of RAM
			//! @public
			void TurnEveryPixelOn();

			//! @brief		Makes the screen follow the contents of RAM again
			//! @public
			void TurnEveryPixelOff();

			//! @brief		Sets the display clock (DCLK) divider and oscillator frequency
			//! @details	
			//! @param		regVal Config byte
			//!				regVal[3:0]: Display clock divide ratio. 0x00->0xFF maps to divide ratio
			//!					of 1-16.
			//!				regVal[7:4]: Oscillator frequency for CLK if CLS pin pulled high. 0x00->0xFF
			//!					maps from approx 270-540kHz
			//! @public
			void SetDisplayClock(uint8 regVal);

			//! @brief		Sets multiplex ratio
			//! @details	Influences screen brightness. The output pads COM0-COM63 will
			//!				be switched to the corresponding COM signal.
			//! @param		d Multiplex ratio. Valid range 16-63. Default = 63
			//! @public
			void SetMultiplexRatio(uint8 regVal);

			//! @public
			void SetDisplayOffset(uint8 regVal);

			//! @public
			void SetStartLine(uint8 regVal);

			//! @brief		Sets the memory addressing mode
			//! @details	Use the enumeration #addressingMode_t to set the addressing
			//!				mode. Options are horizontal, vertical, or page addressing.
			//! @public
			void SetAddressingMode(addressingMode_t addressingMode);

			//! @public
			void SetSegmentRemap(uint8 regVal);

			//! @public
			void SetCommonRemap(uint8 regVal);

			//! @details	Used for smaller screens (e.g. 128x32 pixel) to remap RAM contents
			//!				so that it is displayed properly.
			//! @param		d Config byte. 0b00xx0010
			//!					d[4] = 0b: Sequential COM pin config.
			//!					d[4] = 1b: Alternative COM pin config (default)
			//!					d[5] = 0b: Disable COM left/right remap (default)
			//!					d[5] = 1b: Enable COM left/right remap
			//! @public
			void SetComPinConfig(uint8 regVal);

			//! @brief		Sets the control contrast setting for the display
			//! @details	There are 256 contrast steps from 0x00 (min) to 0xFF (max).
			//!				The segment output current increases as the contrast increases.
			void SetContrastControl(uint8 regVal);

			//! @brief		Sets the duration of the pre-charge period.
			//! @details	Interval is counted in the number of DCLKs, default is 2 DCLKs
			//! @param		regVal Config settings
			//!				A[3:0] - Phase 1 period (pre-charge) of up to 15DCLK. 0 is invalid. Default = 0x2
			//!				A[7:4] - Phase 2 period (discharge) of up to 15DCLK. 0 is invalid. Default = 0x2
			void SetPrechargePeriod(uint8 regVal);

			//! @brief		Selects the VOMH deselect (logic 0) level
			//! @details	Command 0xDB
			//! @param		regVal Config byte.
			//!				regVal = 0x00 - 0.65Vcc
			//!				regVal = 0x20 - 0.77Vcc (default)
			//!				regVal = 0x30 - 0.83Vcc 
			//! @public
			void SetVCOMH(uint8 regVal);

			//! @brief		Sets the display to either be normal or inverted.
			//! @details	In normal mode, '1' in RAM indicates pixel on, in inverse mode
			//!				this indicates pixel off.
			//! @param		isInverse
			//!				isInverse = TRUE - Display pixels are inverted
			//!				isInverse = FLASE - Display pixels are normal (default)
			void SetInverseDisplay(bool isInverse);

			//! @brief		Full-screen fade in.
			void FadeIn();
			
			//! @brief		Full-screen fade out.
			void FadeOut();

			//! @brief		Shows a pattern (either partial or full screen)
			//! @details	Requires pixel array to already be formulated and passed into the function
			//! @param		pixelArray 	Monochromatic pixel array to draw on screen
			//! @param		startPage 	Page to start on
			//! @param		endPage 	Page to end on
			//! @param		startCol 	Column to start on
			//! @param		totalCol 	Total number of columns to show
			//! @public
			void ShowPattern(
				uint8 *pixelArray, 
				uint8 startPage,
				uint8 endPage,
				uint8 startCol,
				uint8 totalCol);

			//! @brief		Fills the RAM with a single byte.
			//! @param 		byteToFillRamWith Single byte to fill RAM with.
			//! @public
			void FillRam(unsigned char byteToFillRamWith);

			//! @brief		Shows a full-screen checkerboard.
			void Checkerboard();
			
			//! @brief		Shows a full-screen 1-pixel wide frame (border).
			//! @details	The border is 1 pixel wide and draw right on the edge of the screen. 	
			void DrawFrame();
			
			void ContinuousScroll(
				unsigned char a,
				unsigned char b,
				unsigned char c,
				unsigned char d,
				unsigned char e,
				unsigned char f,
				unsigned char g,
				unsigned char h);
			
			//! @brief		Continuous horizontal scrolling (partial or full screen).
			//! @param   	scrollDir 		Scroll direction.
			//!       			"0x00" (Rightward)
			//!       			"0x01" (Leftward)
			//! @param		startPage		Define start page address.
			//! @param		endPage 		Define end page address.
			//! @param		timeInterval	Set time interval between each scroll step in terms of frame frequency.
			//! @param		delayTime		Delay time
			//! @note 		Currently not working correctly.
			void HorizontalScroll(
				unsigned char scrollDir,
				unsigned char startPage,
				unsigned char endPage,
				unsigned char timeInterval,
				unsigned char delayTime);
				
			void VerticalScroll(
				unsigned char a,
				unsigned char b,
				unsigned char c,
				unsigned char d,
				unsigned char e);
			
			//! @brief		Deactivates scrolling which has been started with ContinuousScroll() or HorizontalScroll().
			void DeactivateScroll();
			
			#if(ssd1306ENABLE_FONTS == 1)
				//! @brief		Displays a character from one of the two font databases.
				//! @param		fontArraySel	The font database to use (valid range 1-2).
				//! @param		asciiChar		The ascii character to display.
				//! @param		startPage		The start page to display the character.
				//! @param		startCol		The start column to display the character.
				//! @note		Only defined if #ssd1306ENABLE_FONTS == 1
				void ShowFont57(
					unsigned char fontArraySel,
					unsigned char asciiChar,
					unsigned char startPage,
					unsigned char startCol);
					
				//! @brief		Displays a message on the screen, using the internal font database to convert
				//!				the string to pixels.
				//! @param		databaseNum		The font database to select from. Valid range is 1-2.
				//! @param		*msg			Pointer to an array of characters to print.
				//! @param		startPage		The screen page to start at.
				//!	@param		startCol		The screen columnh to start at.
				//! @note		Only defined if (#ssd1306ENABLE_FONTS == 1).
				void ShowString(
					uint8_t databaseNum,
					char *msg,
					uint8_t startPage,
					uint8_t startCol);
			#endif
			
		//===============================================================================================//
		//====================================== PUBLIC VARIABLES =======================================//
		//===============================================================================================//

		// none

		private:
		
			//! @brief		Object contains all port-specific functions/variables.
			Port port;
	
			void Sleep(unsigned char a);
			
			//! @brief		Connection test
			//! @note		Non-returning
			void Test();
			
			void SetStartColumn(unsigned char d);
			void SetColumnAddress(unsigned char a, unsigned char b);
			void SetPageAddress(unsigned char a, unsigned char b);
			
			//! @brief		Sets the start page to begin writing from (every 8 columns is one page)
			//! @details	Called from ShowPattern(). 
			//! @note		See page 33 of the SSD1306 reference manual
			//! @private
			void SetStartPage(unsigned char d);
			void SetNOP();
			
			//! @brief		Show a regular byte-wide pattern (partial or full screen)
			//! @details	Pattern will repeat every byte.
			//! @param 		byteToFillRamWith	Byte which contains pattern to repeat.
			//! @param		startPage 	Start page.
			//! @param 		endPage 	End page.
			//! @param  	startCol	Start column.
			//! @param		numCols		Total Columns
			void FillBlock(
				unsigned char byteToFillRamWith,
				unsigned char startPage,
				unsigned char endPage,
				unsigned char startCol,
				unsigned char numCols);
				
			//! @brief 		Writes a single command to the SSD1306 chip over the I2C.
			//! @todo		Remove.
			void WriteCommand(unsigned char cmd);
			
			void WriteCommandArray(uint8 *commandArray, uint8 numCommands);
			
			//! @brief		Writes data to the SSD1306 chip over I2C.
			//! @details	Uses data stored in buffer.
			void WriteData(uint8 cmd);
	};
} // namespace SSD1306Ns

#endif // #ifndef SSD1306_H

// EOF
