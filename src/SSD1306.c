//!
//! @file 		SSD1306.c
//! @author 	Geoffrey Hunter <gbmhunter@gmail.com>
//! @edited 	n/a
//! @date 		25/01/2012
//! @brief 		Driver for the SDD1306 OLED segment/common controller.
//! @details
//!		<b>Last Modified:			</b> 14/12/2012					\n
//!		<b>Version:					</b> v1.0.1						\n
//!		<b>Company:					</b> Beta Solutions				\n
//!		<b>Project:					</b> Procreate: Trial Frames	\n
//!		<b>Language:				</b> C							\n
//!		<b>Computer Architecture:	</b> 8051						\n
//!		<b>Compiler:				</b> Keil						\n
//! 	<b>uC Model:				</b> PSoC3						\n
//! 	<b>Operating System:		</b> n/a						\n
//!		<b>Documentation Format:	</b> Doxygen					\n
//!		<b>License:					</b> OLP						\n
//!
//! See SSD1306.h for more information.
//!

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
#include "./SSD1306/include/Port.h"
#include "./SSD1306/include/SSD1306.h"


//===============================================================================================//
//===================================== PRE-COMPILER CHECKS =====================================//
//===============================================================================================//

#ifndef configDEBUG_SSD1306
	#error Please define the switch configDEBUG_SSD1306
#endif

#ifndef configDEBUG_SSD1306_ERROR
	#error Please define the switch configDEBUG_SSD1306_ERROR
#endif

#ifndef configDEBUG_SSD1306_VERBOSE
	#error Please define the switch configDEBUG_SSD1306_VERBOSE
#endif
	


	

namespace SSD1306Ns
{

	//===============================================================================================//
	//========================================== DEFINES ============================================//
	//===============================================================================================//

	//! 7-bit, right-adjusted SSD1306 I2C address if SA0 is pulled low
	//! 7-bit, right-adjusted SSD1306 I2C address if SA0 is pulled high
	#define SSD1306_I2C_ADD_SA0_0	0x3C	
	#define SSD1306_I2C_ADD_SA0_1	0x3D	

	// Check to make sure SA0 has a defined position
	#if((SSD1306_I2C_SA0_0 ^ SSD1306_I2C_SA0_1) != 1)
		#error Please set either SSD1306_I2C_SA0_0 or SSD1306_I2C_SA0_1 to 1, but not both
	#endif

	// Assign SSD1306_I2C_ADD depending on the position of SA0
	#if(SSD1306_I2C_SA0_0 == 1)
		#define SSD1306_I2C_ADD SSD1306_I2C_ADD_SA0_0	//!< Calculated conditional 7-bit, right-adj I2C address of SSD1306
	#else
		#define SSD1306_I2C_ADD SSD1306_I2C_ADD_SA0_1 	//!< Calculated conditional 7-bit, right-adj I2C address of SSD1306
	#endif
	
	//! @brief		Delay (in microseconds) between asserting reset signal and de-asserting again.
	//! @details	Delay needs to be at least 3us (as per datasheet).
	#define ssd1306_RESET_DELAY_TIME_US				(200)
	
	#define ssd1306REG_VAL_ACTIVATE_DISPLAY 		0xAF
	#define ssd1306REG_VAL_DEACTIVATE_DISPLAY 		0xAE
	
	#define ssd1306REG_VAL_EVERY_PIXEL_ON			0xA5
	#define ssd1306REG_VAL_EVERY_PIXEL_OFF			0xA4
	
	#define ssd1306_REG_ADD_CHARGE_PUMP				0x8D
		#define ssd1306_REG_VAL_CHARGE_PUMP_ENABLED		0x14
		#define ssd1306_REG_VAL_CHARGE_PUMP_DISABLED 	0x10
		
	//! @brief		Controls both the clock divide ratio and oscillator frequency.
	//! @details	Default val = 0b10000000
	#define ssd1306_REG_ADD_CLOCK_DIVIDE_RATIO_AND_OSC_FREQ		0xD5
		//! @brief		The clock divide ratio. Bits 0-3. Valid range is 0-15
		//!				which corresponds to a clock divide ratio of 1-16.
		//!				Default = 0b0
		#define ssd1306_REG_BIT_POS_CLOCK_DIVIDE_RATIO 				0
		//! @brief		Determines the oscillator frequency. Bits 4-7. Valid range
		//!				is 0-15. See datasheet for what frequency each value corresponds
		//!				to.
		//! @details	Default val = 0b1000
		#define ssd1306_REG_BIT_POS_OSC_FREQ						4

	//===============================================================================================//
	//====================================== PRIVATE VARIABLES ======================================//
	//===============================================================================================//
	
	#if(ssd1306ENABLE_FONTS == 1)
		const unsigned char Ascii_1[240][5]={		// Refer to "Times New Roman" Font Database...
								//   Basic Characters
			{0x00,0x00,0x4F,0x00,0x00},		//   (  1)  ! - 0x0021 Exclamation Mark
			{0x00,0x07,0x00,0x07,0x00},		//   (  2)  " - 0x0022 Quotation Mark
			{0x14,0x7F,0x14,0x7F,0x14},		//   (  3)  # - 0x0023 Number Sign
			{0x24,0x2A,0x7F,0x2A,0x12},		//   (  4)  $ - 0x0024 Dollar Sign
			{0x23,0x13,0x08,0x64,0x62},		//   (  5)  % - 0x0025 Percent Sign
			{0x36,0x49,0x55,0x22,0x50},		//   (  6)  & - 0x0026 Ampersand
			{0x00,0x05,0x03,0x00,0x00},		//   (  7)  ' - 0x0027 Apostrophe
			{0x00,0x1C,0x22,0x41,0x00},		//   (  8)  ( - 0x0028 Left Parenthesis
			{0x00,0x41,0x22,0x1C,0x00},		//   (  9)  ) - 0x0029 Right Parenthesis
			{0x14,0x08,0x3E,0x08,0x14},		//   ( 10)  * - 0x002A Asterisk
			{0x08,0x08,0x3E,0x08,0x08},		//   ( 11)  + - 0x002B Plus Sign
			{0x00,0x50,0x30,0x00,0x00},		//   ( 12)  , - 0x002C Comma
			{0x08,0x08,0x08,0x08,0x08},		//   ( 13)  - - 0x002D Hyphen-Minus
			{0x00,0x60,0x60,0x00,0x00},		//   ( 14)  . - 0x002E Full Stop
			{0x20,0x10,0x08,0x04,0x02},		//   ( 15)  / - 0x002F Solidus
			{0x3E,0x51,0x49,0x45,0x3E},		//   ( 16)  0 - 0x0030 Digit Zero
			{0x00,0x42,0x7F,0x40,0x00},		//   ( 17)  1 - 0x0031 Digit One
			{0x42,0x61,0x51,0x49,0x46},		//   ( 18)  2 - 0x0032 Digit Two
			{0x21,0x41,0x45,0x4B,0x31},		//   ( 19)  3 - 0x0033 Digit Three
			{0x18,0x14,0x12,0x7F,0x10},		//   ( 20)  4 - 0x0034 Digit Four
			{0x27,0x45,0x45,0x45,0x39},		//   ( 21)  5 - 0x0035 Digit Five
			{0x3C,0x4A,0x49,0x49,0x30},		//   ( 22)  6 - 0x0036 Digit Six
			{0x01,0x71,0x09,0x05,0x03},		//   ( 23)  7 - 0x0037 Digit Seven
			{0x36,0x49,0x49,0x49,0x36},		//   ( 24)  8 - 0x0038 Digit Eight
			{0x06,0x49,0x49,0x29,0x1E},		//   ( 25)  9 - 0x0039 Dight Nine
			{0x00,0x36,0x36,0x00,0x00},		//   ( 26)  : - 0x003A Colon
			{0x00,0x56,0x36,0x00,0x00},		//   ( 27)  ; - 0x003B Semicolon
			{0x08,0x14,0x22,0x41,0x00},		//   ( 28)  < - 0x003C Less-Than Sign
			{0x14,0x14,0x14,0x14,0x14},		//   ( 29)  = - 0x003D Equals Sign
			{0x00,0x41,0x22,0x14,0x08},		//   ( 30)  > - 0x003E Greater-Than Sign
			{0x02,0x01,0x51,0x09,0x06},		//   ( 31)  ? - 0x003F Question Mark
			{0x32,0x49,0x79,0x41,0x3E},		//   ( 32)  @ - 0x0040 Commercial At
			{0x7E,0x11,0x11,0x11,0x7E},		//   ( 33)  A - 0x0041 Latin Capital Letter A
			{0x7F,0x49,0x49,0x49,0x36},		//   ( 34)  B - 0x0042 Latin Capital Letter B
			{0x3E,0x41,0x41,0x41,0x22},		//   ( 35)  C - 0x0043 Latin Capital Letter C
			{0x7F,0x41,0x41,0x22,0x1C},		//   ( 36)  D - 0x0044 Latin Capital Letter D
			{0x7F,0x49,0x49,0x49,0x41},		//   ( 37)  E - 0x0045 Latin Capital Letter E
			{0x7F,0x09,0x09,0x09,0x01},		//   ( 38)  F - 0x0046 Latin Capital Letter F
			{0x3E,0x41,0x49,0x49,0x7A},		//   ( 39)  G - 0x0047 Latin Capital Letter G
			{0x7F,0x08,0x08,0x08,0x7F},		//   ( 40)  H - 0x0048 Latin Capital Letter H
			{0x00,0x41,0x7F,0x41,0x00},		//   ( 41)  I - 0x0049 Latin Capital Letter I
			{0x20,0x40,0x41,0x3F,0x01},		//   ( 42)  J - 0x004A Latin Capital Letter J
			{0x7F,0x08,0x14,0x22,0x41},		//   ( 43)  K - 0x004B Latin Capital Letter K
			{0x7F,0x40,0x40,0x40,0x40},		//   ( 44)  L - 0x004C Latin Capital Letter L
			{0x7F,0x02,0x0C,0x02,0x7F},		//   ( 45)  M - 0x004D Latin Capital Letter M
			{0x7F,0x04,0x08,0x10,0x7F},		//   ( 46)  N - 0x004E Latin Capital Letter N
			{0x3E,0x41,0x41,0x41,0x3E},		//   ( 47)  O - 0x004F Latin Capital Letter O
			{0x7F,0x09,0x09,0x09,0x06},		//   ( 48)  P - 0x0050 Latin Capital Letter P
			{0x3E,0x41,0x51,0x21,0x5E},		//   ( 49)  Q - 0x0051 Latin Capital Letter Q
			{0x7F,0x09,0x19,0x29,0x46},		//   ( 50)  R - 0x0052 Latin Capital Letter R
			{0x46,0x49,0x49,0x49,0x31},		//   ( 51)  S - 0x0053 Latin Capital Letter S
			{0x01,0x01,0x7F,0x01,0x01},		//   ( 52)  T - 0x0054 Latin Capital Letter T
			{0x3F,0x40,0x40,0x40,0x3F},		//   ( 53)  U - 0x0055 Latin Capital Letter U
			{0x1F,0x20,0x40,0x20,0x1F},		//   ( 54)  V - 0x0056 Latin Capital Letter V
			{0x3F,0x40,0x38,0x40,0x3F},		//   ( 55)  W - 0x0057 Latin Capital Letter W
			{0x63,0x14,0x08,0x14,0x63},		//   ( 56)  X - 0x0058 Latin Capital Letter X
			{0x07,0x08,0x70,0x08,0x07},		//   ( 57)  Y - 0x0059 Latin Capital Letter Y
			{0x61,0x51,0x49,0x45,0x43},		//   ( 58)  Z - 0x005A Latin Capital Letter Z
			{0x00,0x7F,0x41,0x41,0x00},		//   ( 59)  [ - 0x005B Left Square Bracket
			{0x02,0x04,0x08,0x10,0x20},		//   ( 60)  \ - 0x005C Reverse Solidus
			{0x00,0x41,0x41,0x7F,0x00},		//   ( 61)  ] - 0x005D Right Square Bracket
			{0x04,0x02,0x01,0x02,0x04},		//   ( 62)  ^ - 0x005E Circumflex Accent
			{0x40,0x40,0x40,0x40,0x40},		//   ( 63)  _ - 0x005F Low Line
			{0x01,0x02,0x04,0x00,0x00},		//   ( 64)  ` - 0x0060 Grave Accent
			{0x20,0x54,0x54,0x54,0x78},		//   ( 65)  a - 0x0061 Latin Small Letter A
			{0x7F,0x48,0x44,0x44,0x38},		//   ( 66)  b - 0x0062 Latin Small Letter B
			{0x38,0x44,0x44,0x44,0x20},		//   ( 67)  c - 0x0063 Latin Small Letter C
			{0x38,0x44,0x44,0x48,0x7F},		//   ( 68)  d - 0x0064 Latin Small Letter D
			{0x38,0x54,0x54,0x54,0x18},		//   ( 69)  e - 0x0065 Latin Small Letter E
			{0x08,0x7E,0x09,0x01,0x02},		//   ( 70)  f - 0x0066 Latin Small Letter F
			{0x06,0x49,0x49,0x49,0x3F},		//   ( 71)  g - 0x0067 Latin Small Letter G
			{0x7F,0x08,0x04,0x04,0x78},		//   ( 72)  h - 0x0068 Latin Small Letter H
			{0x00,0x44,0x7D,0x40,0x00},		//   ( 73)  i - 0x0069 Latin Small Letter I
			{0x20,0x40,0x44,0x3D,0x00},		//   ( 74)  j - 0x006A Latin Small Letter J
			{0x7F,0x10,0x28,0x44,0x00},		//   ( 75)  k - 0x006B Latin Small Letter K
			{0x00,0x41,0x7F,0x40,0x00},		//   ( 76)  l - 0x006C Latin Small Letter L
			{0x7C,0x04,0x18,0x04,0x7C},		//   ( 77)  m - 0x006D Latin Small Letter M
			{0x7C,0x08,0x04,0x04,0x78},		//   ( 78)  n - 0x006E Latin Small Letter N
			{0x38,0x44,0x44,0x44,0x38},		//   ( 79)  o - 0x006F Latin Small Letter O
			{0x7C,0x14,0x14,0x14,0x08},		//   ( 80)  p - 0x0070 Latin Small Letter P
			{0x08,0x14,0x14,0x18,0x7C},		//   ( 81)  q - 0x0071 Latin Small Letter Q
			{0x7C,0x08,0x04,0x04,0x08},		//   ( 82)  r - 0x0072 Latin Small Letter R
			{0x48,0x54,0x54,0x54,0x20},		//   ( 83)  s - 0x0073 Latin Small Letter S
			{0x04,0x3F,0x44,0x40,0x20},		//   ( 84)  t - 0x0074 Latin Small Letter T
			{0x3C,0x40,0x40,0x20,0x7C},		//   ( 85)  u - 0x0075 Latin Small Letter U
			{0x1C,0x20,0x40,0x20,0x1C},		//   ( 86)  v - 0x0076 Latin Small Letter V
			{0x3C,0x40,0x30,0x40,0x3C},		//   ( 87)  w - 0x0077 Latin Small Letter W
			{0x44,0x28,0x10,0x28,0x44},		//   ( 88)  x - 0x0078 Latin Small Letter X
			{0x0C,0x50,0x50,0x50,0x3C},		//   ( 89)  y - 0x0079 Latin Small Letter Y
			{0x44,0x64,0x54,0x4C,0x44},		//   ( 90)  z - 0x007A Latin Small Letter Z
			{0x00,0x08,0x36,0x41,0x00},		//   ( 91)  { - 0x007B Left Curly Bracket
			{0x00,0x00,0x7F,0x00,0x00},		//   ( 92)  | - 0x007C Vertical Line
			{0x00,0x41,0x36,0x08,0x00},		//   ( 93)  } - 0x007D Right Curly Bracket
			{0x02,0x01,0x02,0x04,0x02},		//   ( 94)  ~ - 0x007E Tilde
			{0x3E,0x55,0x55,0x41,0x22},		//   ( 95)  C - 0x0080 <Control>
			{0x00,0x00,0x00,0x00,0x00},		//   ( 96)    - 0x00A0 No-Break Space
			{0x00,0x00,0x79,0x00,0x00},		//   ( 97)  ! - 0x00A1 Inverted Exclamation Mark
			{0x18,0x24,0x74,0x2E,0x24},		//   ( 98)  c - 0x00A2 Cent Sign
			{0x48,0x7E,0x49,0x42,0x40},		//   ( 99)  L - 0x00A3 Pound Sign
			{0x5D,0x22,0x22,0x22,0x5D},		//   (100)  o - 0x00A4 Currency Sign
			{0x15,0x16,0x7C,0x16,0x15},		//   (101)  Y - 0x00A5 Yen Sign
			{0x00,0x00,0x77,0x00,0x00},		//   (102)  | - 0x00A6 Broken Bar
			{0x0A,0x55,0x55,0x55,0x28},		//   (103)    - 0x00A7 Section Sign
			{0x00,0x01,0x00,0x01,0x00},		//   (104)  " - 0x00A8 Diaeresis
			{0x00,0x0A,0x0D,0x0A,0x04},		//   (105)    - 0x00AA Feminine Ordinal Indicator
			{0x08,0x14,0x2A,0x14,0x22},		//   (106) << - 0x00AB Left-Pointing Double Angle Quotation Mark
			{0x04,0x04,0x04,0x04,0x1C},		//   (107)    - 0x00AC Not Sign
			{0x00,0x08,0x08,0x08,0x00},		//   (108)  - - 0x00AD Soft Hyphen
			{0x01,0x01,0x01,0x01,0x01},		//   (109)    - 0x00AF Macron
			{0x00,0x02,0x05,0x02,0x00},		//   (110)    - 0x00B0 Degree Sign
			{0x44,0x44,0x5F,0x44,0x44},		//   (111) +- - 0x00B1 Plus-Minus Sign
			{0x00,0x00,0x04,0x02,0x01},		//   (112)  ` - 0x00B4 Acute Accent
			{0x7E,0x20,0x20,0x10,0x3E},		//   (113)  u - 0x00B5 Micro Sign
			{0x06,0x0F,0x7F,0x00,0x7F},		//   (114)    - 0x00B6 Pilcrow Sign
			{0x00,0x18,0x18,0x00,0x00},		//   (115)  . - 0x00B7 Middle Dot
			{0x00,0x40,0x50,0x20,0x00},		//   (116)    - 0x00B8 Cedilla
			{0x00,0x0A,0x0D,0x0A,0x00},		//   (117)    - 0x00BA Masculine Ordinal Indicator
			{0x22,0x14,0x2A,0x14,0x08},		//   (118) >> - 0x00BB Right-Pointing Double Angle Quotation Mark
			{0x17,0x08,0x34,0x2A,0x7D},		//   (119) /4 - 0x00BC Vulgar Fraction One Quarter
			{0x17,0x08,0x04,0x6A,0x59},		//   (120) /2 - 0x00BD Vulgar Fraction One Half
			{0x30,0x48,0x45,0x40,0x20},		//   (121)  ? - 0x00BF Inverted Question Mark
			{0x70,0x29,0x26,0x28,0x70},		//   (122) `A - 0x00C0 Latin Capital Letter A with Grave
			{0x70,0x28,0x26,0x29,0x70},		//   (123) 'A - 0x00C1 Latin Capital Letter A with Acute
			{0x70,0x2A,0x25,0x2A,0x70},		//   (124) ^A - 0x00C2 Latin Capital Letter A with Circumflex
			{0x72,0x29,0x26,0x29,0x70},		//   (125) ~A - 0x00C3 Latin Capital Letter A with Tilde
			{0x70,0x29,0x24,0x29,0x70},		//   (126) "A - 0x00C4 Latin Capital Letter A with Diaeresis
			{0x70,0x2A,0x2D,0x2A,0x70},		//   (127)  A - 0x00C5 Latin Capital Letter A with Ring Above
			{0x7E,0x11,0x7F,0x49,0x49},		//   (128) AE - 0x00C6 Latin Capital Letter Ae
			{0x0E,0x51,0x51,0x71,0x11},		//   (129)  C - 0x00C7 Latin Capital Letter C with Cedilla
			{0x7C,0x55,0x56,0x54,0x44},		//   (130) `E - 0x00C8 Latin Capital Letter E with Grave
			{0x7C,0x55,0x56,0x54,0x44},		//   (131) 'E - 0x00C9 Latin Capital Letter E with Acute
			{0x7C,0x56,0x55,0x56,0x44},		//   (132) ^E - 0x00CA Latin Capital Letter E with Circumflex
			{0x7C,0x55,0x54,0x55,0x44},		//   (133) "E - 0x00CB Latin Capital Letter E with Diaeresis
			{0x00,0x45,0x7E,0x44,0x00},		//   (134) `I - 0x00CC Latin Capital Letter I with Grave
			{0x00,0x44,0x7E,0x45,0x00},		//   (135) 'I - 0x00CD Latin Capital Letter I with Acute
			{0x00,0x46,0x7D,0x46,0x00},		//   (136) ^I - 0x00CE Latin Capital Letter I with Circumflex
			{0x00,0x45,0x7C,0x45,0x00},		//   (137) "I - 0x00CF Latin Capital Letter I with Diaeresis
			{0x7F,0x49,0x49,0x41,0x3E},		//   (138)  D - 0x00D0 Latin Capital Letter Eth
			{0x7C,0x0A,0x11,0x22,0x7D},		//   (139) ~N - 0x00D1 Latin Capital Letter N with Tilde
			{0x38,0x45,0x46,0x44,0x38},		//   (140) `O - 0x00D2 Latin Capital Letter O with Grave
			{0x38,0x44,0x46,0x45,0x38},		//   (141) 'O - 0x00D3 Latin Capital Letter O with Acute
			{0x38,0x46,0x45,0x46,0x38},		//   (142) ^O - 0x00D4 Latin Capital Letter O with Circumflex
			{0x38,0x46,0x45,0x46,0x39},		//   (143) ~O - 0x00D5 Latin Capital Letter O with Tilde
			{0x38,0x45,0x44,0x45,0x38},		//   (144) "O - 0x00D6 Latin Capital Letter O with Diaeresis
			{0x22,0x14,0x08,0x14,0x22},		//   (145)  x - 0x00D7 Multiplcation Sign
			{0x2E,0x51,0x49,0x45,0x3A},		//   (146)  O - 0x00D8 Latin Capital Letter O with Stroke
			{0x3C,0x41,0x42,0x40,0x3C},		//   (147) `U - 0x00D9 Latin Capital Letter U with Grave
			{0x3C,0x40,0x42,0x41,0x3C},		//   (148) 'U - 0x00DA Latin Capital Letter U with Acute
			{0x3C,0x42,0x41,0x42,0x3C},		//   (149) ^U - 0x00DB Latin Capital Letter U with Circumflex
			{0x3C,0x41,0x40,0x41,0x3C},		//   (150) "U - 0x00DC Latin Capital Letter U with Diaeresis
			{0x0C,0x10,0x62,0x11,0x0C},		//   (151) `Y - 0x00DD Latin Capital Letter Y with Acute
			{0x7F,0x12,0x12,0x12,0x0C},		//   (152)  P - 0x00DE Latin Capital Letter Thom
			{0x40,0x3E,0x01,0x49,0x36},		//   (153)  B - 0x00DF Latin Capital Letter Sharp S
			{0x20,0x55,0x56,0x54,0x78},		//   (154) `a - 0x00E0 Latin Small Letter A with Grave
			{0x20,0x54,0x56,0x55,0x78},		//   (155) 'a - 0x00E1 Latin Small Letter A with Acute
			{0x20,0x56,0x55,0x56,0x78},		//   (156) ^a - 0x00E2 Latin Small Letter A with Circumflex
			{0x20,0x55,0x56,0x55,0x78},		//   (157) ~a - 0x00E3 Latin Small Letter A with Tilde
			{0x20,0x55,0x54,0x55,0x78},		//   (158) "a - 0x00E4 Latin Small Letter A with Diaeresis
			{0x20,0x56,0x57,0x56,0x78},		//   (159)  a - 0x00E5 Latin Small Letter A with Ring Above
			{0x24,0x54,0x78,0x54,0x58},		//   (160) ae - 0x00E6 Latin Small Letter Ae
			{0x0C,0x52,0x52,0x72,0x13},		//   (161)  c - 0x00E7 Latin Small Letter c with Cedilla
			{0x38,0x55,0x56,0x54,0x18},		//   (162) `e - 0x00E8 Latin Small Letter E with Grave
			{0x38,0x54,0x56,0x55,0x18},		//   (163) 'e - 0x00E9 Latin Small Letter E with Acute
			{0x38,0x56,0x55,0x56,0x18},		//   (164) ^e - 0x00EA Latin Small Letter E with Circumflex
			{0x38,0x55,0x54,0x55,0x18},		//   (165) "e - 0x00EB Latin Small Letter E with Diaeresis
			{0x00,0x49,0x7A,0x40,0x00},		//   (166) `i - 0x00EC Latin Small Letter I with Grave
			{0x00,0x48,0x7A,0x41,0x00},		//   (167) 'i - 0x00ED Latin Small Letter I with Acute
			{0x00,0x4A,0x79,0x42,0x00},		//   (168) ^i - 0x00EE Latin Small Letter I with Circumflex
			{0x00,0x4A,0x78,0x42,0x00},		//   (169) "i - 0x00EF Latin Small Letter I with Diaeresis
			{0x31,0x4A,0x4E,0x4A,0x30},		//   (170)    - 0x00F0 Latin Small Letter Eth
			{0x7A,0x11,0x0A,0x09,0x70},		//   (171) ~n - 0x00F1 Latin Small Letter N with Tilde
			{0x30,0x49,0x4A,0x48,0x30},		//   (172) `o - 0x00F2 Latin Small Letter O with Grave
			{0x30,0x48,0x4A,0x49,0x30},		//   (173) 'o - 0x00F3 Latin Small Letter O with Acute
			{0x30,0x4A,0x49,0x4A,0x30},		//   (174) ^o - 0x00F4 Latin Small Letter O with Circumflex
			{0x30,0x4A,0x49,0x4A,0x31},		//   (175) ~o - 0x00F5 Latin Small Letter O with Tilde
			{0x30,0x4A,0x48,0x4A,0x30},		//   (176) "o - 0x00F6 Latin Small Letter O with Diaeresis
			{0x08,0x08,0x2A,0x08,0x08},		//   (177)  + - 0x00F7 Division Sign
			{0x38,0x64,0x54,0x4C,0x38},		//   (178)  o - 0x00F8 Latin Small Letter O with Stroke
			{0x38,0x41,0x42,0x20,0x78},		//   (179) `u - 0x00F9 Latin Small Letter U with Grave
			{0x38,0x40,0x42,0x21,0x78},		//   (180) 'u - 0x00FA Latin Small Letter U with Acute
			{0x38,0x42,0x41,0x22,0x78},		//   (181) ^u - 0x00FB Latin Small Letter U with Circumflex
			{0x38,0x42,0x40,0x22,0x78},		//   (182) "u - 0x00FC Latin Small Letter U with Diaeresis
			{0x0C,0x50,0x52,0x51,0x3C},		//   (183) 'y - 0x00FD Latin Small Letter Y with Acute
			{0x7E,0x14,0x14,0x14,0x08},		//   (184)  p - 0x00FE Latin Small Letter Thom
			{0x0C,0x51,0x50,0x51,0x3C},		//   (185) "y - 0x00FF Latin Small Letter Y with Diaeresis
			{0x1E,0x09,0x09,0x29,0x5E},		//   (186)  A - 0x0104 Latin Capital Letter A with Ogonek
			{0x08,0x15,0x15,0x35,0x4E},		//   (187)  a - 0x0105 Latin Small Letter A with Ogonek
			{0x38,0x44,0x46,0x45,0x20},		//   (188) 'C - 0x0106 Latin Capital Letter C with Acute
			{0x30,0x48,0x4A,0x49,0x20},		//   (189) 'c - 0x0107 Latin Small Letter C with Acute
			{0x38,0x45,0x46,0x45,0x20},		//   (190)  C - 0x010C Latin Capital Letter C with Caron
			{0x30,0x49,0x4A,0x49,0x20},		//   (191)  c - 0x010D Latin Small Letter C with Caron
			{0x7C,0x45,0x46,0x45,0x38},		//   (192)  D - 0x010E Latin Capital Letter D with Caron
			{0x20,0x50,0x50,0x7C,0x03},		//   (193) d' - 0x010F Latin Small Letter D with Caron
			{0x1F,0x15,0x15,0x35,0x51},		//   (194)  E - 0x0118 Latin Capital Letter E with Ogonek
			{0x0E,0x15,0x15,0x35,0x46},		//   (195)  e - 0x0119 Latin Small Letter E with Ogonek
			{0x7C,0x55,0x56,0x55,0x44},		//   (196)  E - 0x011A Latin Capital Letter E with Caron
			{0x38,0x55,0x56,0x55,0x18},		//   (197)  e - 0x011B Latin Small Letter E with Caron
			{0x00,0x44,0x7C,0x40,0x00},		//   (198)  i - 0x0131 Latin Small Letter Dotless I
			{0x7F,0x48,0x44,0x40,0x40},		//   (199)  L - 0x0141 Latin Capital Letter L with Stroke
			{0x00,0x49,0x7F,0x44,0x00},		//   (200)  l - 0x0142 Latin Small Letter L with Stroke
			{0x7C,0x08,0x12,0x21,0x7C},		//   (201) 'N - 0x0143 Latin Capital Letter N with Acute
			{0x78,0x10,0x0A,0x09,0x70},		//   (202) 'n - 0x0144 Latin Small Letter N with Acute
			{0x7C,0x09,0x12,0x21,0x7C},		//   (203)  N - 0x0147 Latin Capital Letter N with Caron
			{0x78,0x11,0x0A,0x09,0x70},		//   (204)  n - 0x0148 Latin Small Letter N with Caron
			{0x38,0x47,0x44,0x47,0x38},		//   (205) "O - 0x0150 Latin Capital Letter O with Double Acute
			{0x30,0x4B,0x48,0x4B,0x30},		//   (206) "o - 0x0151 Latin Small Letter O with Double Acute
			{0x3E,0x41,0x7F,0x49,0x49},		//   (207) OE - 0x0152 Latin Capital Ligature Oe
			{0x38,0x44,0x38,0x54,0x58},		//   (208) oe - 0x0153 Latin Small Ligature Oe
			{0x7C,0x15,0x16,0x35,0x48},		//   (209)  R - 0x0158 Latin Capital Letter R with Caron
			{0x78,0x11,0x0A,0x09,0x10},		//   (210)  r - 0x0159 Latin Small Letter R with Caron
			{0x48,0x54,0x56,0x55,0x20},		//   (211) 'S - 0x015A Latin Capital Letter S with Acute
			{0x20,0x48,0x56,0x55,0x20},		//   (212) 's - 0x015B Latin Small Letter S with Acute
			{0x48,0x55,0x56,0x55,0x20},		//   (213)  S - 0x0160 Latin Capital Letter S with Caron
			{0x20,0x49,0x56,0x55,0x20},		//   (214)  s - 0x0161 Latin Small Letter S with Caron
			{0x04,0x05,0x7E,0x05,0x04},		//   (215)  T - 0x0164 Latin Capital Letter T with Caron
			{0x08,0x3C,0x48,0x22,0x01},		//   (216) t' - 0x0165 Latin Small Letter T with Caron
			{0x3C,0x42,0x45,0x42,0x3C},		//   (217)  U - 0x016E Latin Capital Letter U with Ring Above
			{0x38,0x42,0x45,0x22,0x78},		//   (218)  u - 0x016F Latin Small Letter U with Ring Above
			{0x3C,0x43,0x40,0x43,0x3C},		//   (219) "U - 0x0170 Latin Capital Letter U with Double Acute
			{0x38,0x43,0x40,0x23,0x78},		//   (220) "u - 0x0171 Latin Small Letter U with Double Acute
			{0x0C,0x11,0x60,0x11,0x0C},		//   (221) "Y - 0x0178 Latin Capital Letter Y with Diaeresis
			{0x44,0x66,0x55,0x4C,0x44},		//   (222) 'Z - 0x0179 Latin Capital Letter Z with Acute
			{0x48,0x6A,0x59,0x48,0x00},		//   (223) 'z - 0x017A Latin Small Letter Z with Acute
			{0x44,0x64,0x55,0x4C,0x44},		//   (224)  Z - 0x017B Latin Capital Letter Z with Dot Above
			{0x48,0x68,0x5A,0x48,0x00},		//   (225)  z - 0x017C Latin Small Letter Z with Dot Above
			{0x44,0x65,0x56,0x4D,0x44},		//   (226)  Z - 0x017D Latin Capital Letter Z with Caron
			{0x48,0x69,0x5A,0x49,0x00},		//   (227)  z - 0x017E Latin Small Letter Z with Caron
			{0x00,0x02,0x01,0x02,0x00},		//   (228)  ^ - 0x02C6 Modifier Letter Circumflex Accent
			{0x00,0x01,0x02,0x01,0x00},		//   (229)    - 0x02C7 Caron
			{0x00,0x01,0x01,0x01,0x00},		//   (230)    - 0x02C9 Modifier Letter Macron
			{0x01,0x02,0x02,0x01,0x00},		//   (231)    - 0x02D8 Breve
			{0x00,0x00,0x01,0x00,0x00},		//   (232)    - 0x02D9 Dot Above
			{0x00,0x02,0x05,0x02,0x00},		//   (233)    - 0x02DA Ring Above
			{0x02,0x01,0x02,0x01,0x00},		//   (234)  ~ - 0x02DC Small Tilde
			{0x7F,0x05,0x15,0x3A,0x50},		//   (235) Pt - 0x20A7 Peseta Sign
			{0x3E,0x55,0x55,0x41,0x22},		//   (236)  C - 0x20AC Euro Sign
			{0x18,0x14,0x08,0x14,0x0C},		//   (237)    - 0x221E Infinity
			{0x44,0x4A,0x4A,0x51,0x51},		//   (238)  < - 0x2264 Less-Than or Equal to
			{0x51,0x51,0x4A,0x4A,0x44},		//   (239)  > - 0x2265 Greater-Than or Equal to
			{0x74,0x42,0x41,0x42,0x74},		//   (240)    - 0x2302 House
		};


		const unsigned char Ascii_2[107][5]={		// Refer to "Times New Roman" Font Database...
								//   Greek & Japanese Letters
			{0x7E,0x11,0x11,0x11,0x7E},		//   (  1)  A - 0x0391 Greek Capital Letter Alpha
			{0x7F,0x49,0x49,0x49,0x36},		//   (  2)  B - 0x0392 Greek Capital Letter Beta
			{0x7F,0x02,0x01,0x01,0x03},		//   (  3)    - 0x0393 Greek Capital Letter Gamma
			{0x70,0x4E,0x41,0x4E,0x70},		//   (  4)    - 0x0394 Greek Capital Letter Delta
			{0x7F,0x49,0x49,0x49,0x41},		//   (  5)  E - 0x0395 Greek Capital Letter Epsilon
			{0x61,0x51,0x49,0x45,0x43},		//   (  6)  Z - 0x0396 Greek Capital Letter Zeta
			{0x7F,0x08,0x08,0x08,0x7F},		//   (  7)  H - 0x0397 Greek Capital Letter Eta
			{0x3E,0x49,0x49,0x49,0x3E},		//   (  8)    - 0x0398 Greek Capital Letter Theta
			{0x00,0x41,0x7F,0x41,0x00},		//   (  9)  I - 0x0399 Greek Capital Letter Iota
			{0x7F,0x08,0x14,0x22,0x41},		//   ( 10)  K - 0x039A Greek Capital Letter Kappa
			{0x70,0x0E,0x01,0x0E,0x70},		//   ( 11)    - 0x039B Greek Capital Letter Lamda
			{0x7F,0x02,0x0C,0x02,0x7F},		//   ( 12)  M - 0x039C Greek Capital Letter Mu
			{0x7F,0x04,0x08,0x10,0x7F},		//   ( 13)  N - 0x039D Greek Capital Letter Nu
			{0x63,0x5D,0x49,0x5D,0x63},		//   ( 14)    - 0x039E Greek Capital Letter Xi
			{0x3E,0x41,0x41,0x41,0x3E},		//   ( 15)  O - 0x039F Greek Capital Letter Omicron
			{0x41,0x3F,0x01,0x3F,0x41},		//   ( 16)    - 0x03A0 Greek Capital Letter Pi
			{0x7F,0x09,0x09,0x09,0x06},		//   ( 17)  P - 0x03A1 Greek Capital Letter Rho
			{0x63,0x55,0x49,0x41,0x41},		//   ( 18)    - 0x03A3 Greek Capital Letter Sigma
			{0x01,0x01,0x7F,0x01,0x01},		//   ( 19)  T - 0x03A4 Greek Capital Letter Tau
			{0x03,0x01,0x7E,0x01,0x03},		//   ( 20)    - 0x03A5 Greek Capital Letter Upsilon
			{0x08,0x55,0x7F,0x55,0x08},		//   ( 21)    - 0x03A6 Greek Capital Letter Phi
			{0x63,0x14,0x08,0x14,0x63},		//   ( 22)  X - 0x03A7 Greek Capital Letter Chi
			{0x07,0x48,0x7F,0x48,0x07},		//   ( 23)    - 0x03A8 Greek Capital Letter Psi
			{0x5E,0x61,0x01,0x61,0x5E},		//   ( 24)    - 0x03A9 Greek Capital Letter Omega
			{0x38,0x44,0x48,0x30,0x4C},		//   ( 25)  a - 0x03B1 Greek Small Letter Alpha
			{0x7C,0x2A,0x2A,0x2A,0x14},		//   ( 26)  B - 0x03B2 Greek Small Letter Beta
			{0x44,0x38,0x04,0x04,0x08},		//   ( 27)  r - 0x03B3 Greek Small Letter Gamma
			{0x30,0x4B,0x4D,0x59,0x30},		//   ( 28)    - 0x03B4 Greek Small Letter Delta
			{0x28,0x54,0x54,0x44,0x20},		//   ( 29)    - 0x03B5 Greek Small Letter Epsilon
			{0x00,0x18,0x55,0x52,0x22},		//   ( 30)    - 0x03B6 Greek Small Letter Zeta
			{0x3E,0x04,0x02,0x02,0x7C},		//   ( 31)  n - 0x03B7 Greek Small Letter Eta
			{0x3C,0x4A,0x4A,0x4A,0x3C},		//   ( 32)    - 0x03B8 Greek Small Letter Theta
			{0x00,0x3C,0x40,0x20,0x00},		//   ( 33)  i - 0x03B9 Greek Small Letter Iota
			{0x7C,0x10,0x28,0x44,0x40},		//   ( 34)  k - 0x03BA Greek Small Letter Kappa
			{0x41,0x32,0x0C,0x30,0x40},		//   ( 35)    - 0x03BB Greek Small Letter Lamda
			{0x7E,0x20,0x20,0x10,0x3E},		//   ( 36)  u - 0x03BC Greek Small Letter Mu
			{0x1C,0x20,0x40,0x20,0x1C},		//   ( 37)  v - 0x03BD Greek Small Letter Nu
			{0x14,0x2B,0x2A,0x2A,0x60},		//   ( 38)    - 0x03BE Greek Small Letter Xi
			{0x38,0x44,0x44,0x44,0x38},		//   ( 39)  o - 0x03BF Greek Small Letter Omicron
			{0x44,0x3C,0x04,0x7C,0x44},		//   ( 40)    - 0x03C0 Greek Small Letter Pi
			{0x70,0x28,0x24,0x24,0x18},		//   ( 41)  p - 0x03C1 Greek Small Letter Rho
			{0x0C,0x12,0x12,0x52,0x60},		//   ( 42)    - 0x03C2 Greek Small Letter Final Sigma
			{0x38,0x44,0x4C,0x54,0x24},		//   ( 43)    - 0x03C3 Greek Small Letter Sigma
			{0x04,0x3C,0x44,0x20,0x00},		//   ( 44)  t - 0x03C4 Greek Small Letter Tau
			{0x3C,0x40,0x40,0x20,0x1C},		//   ( 45)  v - 0x03C5 Greek Small Letter Upsilon
			{0x18,0x24,0x7E,0x24,0x18},		//   ( 46)    - 0x03C6 Greek Small Letter Phi
			{0x44,0x28,0x10,0x28,0x44},		//   ( 47)  x - 0x03C7 Greek Small Letter Chi
			{0x0C,0x10,0x7E,0x10,0x0C},		//   ( 48)    - 0x03C8 Greek Small Letter Psi
			{0x38,0x44,0x30,0x44,0x38},		//   ( 49)  w - 0x03C9 Greek Small Letter Omega
			{0x0A,0x0A,0x4A,0x2A,0x1E},		//   ( 50)    - 0xFF66 Katakana Letter Wo
			{0x04,0x44,0x34,0x14,0x0C},		//   ( 51)    - 0xFF67 Katakana Letter Small A
			{0x20,0x10,0x78,0x04,0x00},		//   ( 52)    - 0xFF68 Katakana Letter Small I
			{0x18,0x08,0x4C,0x48,0x38},		//   ( 53)    - 0xFF69 Katakana Letter Small U
			{0x48,0x48,0x78,0x48,0x48},		//   ( 54)    - 0xFF6A Katakana Letter Small E
			{0x48,0x28,0x18,0x7C,0x08},		//   ( 55)    - 0xFF6B Katakana Letter Small O
			{0x08,0x7C,0x08,0x28,0x18},		//   ( 56)    - 0xFF6C Katakana Letter Small Ya
			{0x40,0x48,0x48,0x78,0x40},		//   ( 57)    - 0xFF6D Katakana Letter Small Yu
			{0x54,0x54,0x54,0x7C,0x00},		//   ( 58)    - 0xFF6E Katakana Letter Small Yo
			{0x18,0x00,0x58,0x40,0x38},		//   ( 59)    - 0xFF6F Katakana Letter Small Tu
			{0x08,0x08,0x08,0x08,0x08},		//   ( 60)    - 0xFF70 Katakana-Hiragana Prolonged Sound Mark
			{0x01,0x41,0x3D,0x09,0x07},		//   ( 61)    - 0xFF71 Katakana Letter A
			{0x10,0x08,0x7C,0x02,0x01},		//   ( 62)    - 0xFF72 Katakana Letter I
			{0x0E,0x02,0x43,0x22,0x1E},		//   ( 63)    - 0xFF73 Katakana Letter U
			{0x42,0x42,0x7E,0x42,0x42},		//   ( 64)    - 0xFF74 Katakana Letter E
			{0x22,0x12,0x0A,0x7F,0x02},		//   ( 65)    - 0xFF75 Katakana Letter O
			{0x42,0x3F,0x02,0x42,0x3E},		//   ( 66)    - 0xFF76 Katakana Letter Ka
			{0x0A,0x0A,0x7F,0x0A,0x0A},		//   ( 67)    - 0xFF77 Katakana Letter Ki
			{0x08,0x46,0x42,0x22,0x1E},		//   ( 68)    - 0xFF78 Katakana Letter Ku
			{0x04,0x03,0x42,0x3E,0x02},		//   ( 69)    - 0xFF79 Katakana Letter Ke
			{0x42,0x42,0x42,0x42,0x7E},		//   ( 70)    - 0xFF7A Katakana Letter Ko
			{0x02,0x4F,0x22,0x1F,0x02},		//   ( 71)    - 0xFF7B Katakana Letter Sa
			{0x4A,0x4A,0x40,0x20,0x1C},		//   ( 72)    - 0xFF7C Katakana Letter Shi
			{0x42,0x22,0x12,0x2A,0x46},		//   ( 73)    - 0xFF7D Katakana Letter Su
			{0x02,0x3F,0x42,0x4A,0x46},		//   ( 74)    - 0xFF7E Katakana Letter Se
			{0x06,0x48,0x40,0x20,0x1E},		//   ( 75)    - 0xFF7F Katakana Letter So
			{0x08,0x46,0x4A,0x32,0x1E},		//   ( 76)    - 0xFF80 Katakana Letter Ta
			{0x0A,0x4A,0x3E,0x09,0x08},		//   ( 77)    - 0xFF81 Katakana Letter Chi
			{0x0E,0x00,0x4E,0x20,0x1E},		//   ( 78)    - 0xFF82 Katakana Letter Tsu
			{0x04,0x45,0x3D,0x05,0x04},		//   ( 79)    - 0xFF83 Katakana Letter Te
			{0x00,0x7F,0x08,0x10,0x00},		//   ( 80)    - 0xFF84 Katakana Letter To
			{0x44,0x24,0x1F,0x04,0x04},		//   ( 81)    - 0xFF85 Katakana Letter Na
			{0x40,0x42,0x42,0x42,0x40},		//   ( 82)    - 0xFF86 Katakana Letter Ni
			{0x42,0x2A,0x12,0x2A,0x06},		//   ( 83)    - 0xFF87 Katakana Letter Nu
			{0x22,0x12,0x7B,0x16,0x22},		//   ( 84)    - 0xFF88 Katakana Letter Ne
			{0x00,0x40,0x20,0x1F,0x00},		//   ( 85)    - 0xFF89 Katakana Letter No
			{0x78,0x00,0x02,0x04,0x78},		//   ( 86)    - 0xFF8A Katakana Letter Ha
			{0x3F,0x44,0x44,0x44,0x44},		//   ( 87)    - 0xFF8B Katakana Letter Hi
			{0x02,0x42,0x42,0x22,0x1E},		//   ( 88)    - 0xFF8C Katakana Letter Fu
			{0x04,0x02,0x04,0x08,0x30},		//   ( 89)    - 0xFF8D Katakana Letter He
			{0x32,0x02,0x7F,0x02,0x32},		//   ( 90)    - 0xFF8E Katakana Letter Ho
			{0x02,0x12,0x22,0x52,0x0E},		//   ( 91)    - 0xFF8F Katakana Letter Ma
			{0x00,0x2A,0x2A,0x2A,0x40},		//   ( 92)    - 0xFF90 Katakana Letter Mi
			{0x38,0x24,0x22,0x20,0x70},		//   ( 93)    - 0xFF91 Katakana Letter Mu
			{0x40,0x28,0x10,0x28,0x06},		//   ( 94)    - 0xFF92 Katakana Letter Me
			{0x0A,0x3E,0x4A,0x4A,0x4A},		//   ( 95)    - 0xFF93 Katakana Letter Mo
			{0x04,0x7F,0x04,0x14,0x0C},		//   ( 96)    - 0xFF94 Katakana Letter Ya
			{0x40,0x42,0x42,0x7E,0x40},		//   ( 97)    - 0xFF95 Katakana Letter Yu
			{0x4A,0x4A,0x4A,0x4A,0x7E},		//   ( 98)    - 0xFF96 Katakana Letter Yo
			{0x04,0x05,0x45,0x25,0x1C},		//   ( 99)    - 0xFF97 Katakana Letter Ra
			{0x0F,0x40,0x20,0x1F,0x00},		//   (100)    - 0xFF98 Katakana Letter Ri
			{0x7C,0x00,0x7E,0x40,0x30},		//   (101)    - 0xFF99 Katakana Letter Ru
			{0x7E,0x40,0x20,0x10,0x08},		//   (102)    - 0xFF9A Katakana Letter Re
			{0x7E,0x42,0x42,0x42,0x7E},		//   (103)    - 0xFF9B Katakana Letter Ro
			{0x0E,0x02,0x42,0x22,0x1E},		//   (104)    - 0xFF9C Katakana Letter Wa
			{0x42,0x42,0x40,0x20,0x18},		//   (105)    - 0xFF9D Katakana Letter N
			{0x02,0x04,0x01,0x02,0x00},		//   (106)    - 0xFF9E Katakana Voiced Sound Mark
			{0x07,0x05,0x07,0x00,0x00},		//   (107)    - 0xFF9F Katakana Semi-Voiced Sound Mark
		};
	#endif

	//===============================================================================================//
	//===================================== GLOBAL FUNCTIONS ========================================//
	//===============================================================================================//

	// See Doxygen documentation or function declarations in SSD1306.h for more info.

	void SSD1306::EnableI2c()
	{
		
		port.I2cStart();
		port.I2cEnableInt();
	}

	void SSD1306::SetI2cPort(Port::i2cPort_t i2cPort)
	{
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Changing I2C port...\r\n");
		#endif
		
		port.i2cPort = i2cPort;
		
		if(port.i2cPort == Port::i2cLeft)
		{
			#if(configDEBUG_SSD1306 == 1)
				port.PrintDebug("SSD1306: I2C port changed to left.\r\n");
			#endif
		}
		else if(port.i2cPort == Port::i2cRight)
		{
			#if(configDEBUG_SSD1306 == 1)
				port.PrintDebug("SSD1306: I2C port changed to right.\r\n");
			#endif
		}
	}

	void SSD1306::EnableVddb()
	{
		// Enable OLED screens Vddb (P-ch MOSFETs)
		PinCpRightVddbEnN_Write(0);
		PinCpLeftVddbEnN_Write(0);
	}

	void SSD1306::DisableVddb()
	{
		// Disable OLED screens Vddb (P-ch MOSFETs)
		PinCpRightVddbEnN_Write(1);
		PinCpLeftVddbEnN_Write(1);
	}
	
	void SSD1306::OledInitVccExt()
	{
		
		Reset();
		DeactivateDisplay();		// Display Off (0xAE/0xAF)
		SetDisplayClock(0x80);		// Set Clock as 200 Frames/Sec
		SetMultiplexRatio(0x1F);		// 1/32 Duty (0x0F~0x3F)
		SetDisplayOffset(0x00);		// Shift Mapping RAM Counter (0x00~0x3F)
		SetStartLine(0x00);			// Set Mapping RAM Display Start Line (0x00~0x3F)
		SetChargePumpOff();			// Disable Built-in DC/DC Converter (0x10/0x14)
		SetAddressingMode(SSD1306::PAGE_ADDRESSING_MODE);		// Set Page Addressing Mode (0x00/0x01/0x02)
		SetSegmentRemap(0xA1);		// Set SEG/Column Mapping (0xA0/0xA1)
		SetCommonRemap(0xC8);			// Set COM/Row Scan Direction (0xC0/0xC8)
		SetComPinConfig(0x02);		// Set Alternative Configuration (0x02/0x12)
		SetContrastControl(0xFF);	// Set SEG Output Current
		SetPrechargePeriod(0x22);		// Set Pre-Charge as 2 Clocks & Discharge as 2 Clocks
		SetVCOMH(0x40);			// Set VCOM Deselect Level
		TurnEveryPixelOff();		// Disable Entire Display On (0xA4/0xA5)
		SetInverseDisplay(false);		// Disable Inverse Display On (0xA6/0xA7)

		FillRam(0x00);				// Clear Screen

		ActivateDisplay();		// Display On (0xAE/0xAF)
		
	}
	
	void SSD1306::OledInitVccInt()
	{
		
		// Applicable to both left and right
		EnableVddb();
		EnableI2c();
		
		// LEFT
		SetI2cPort(SSD1306Ns::Port::i2cLeft);
		Reset();
		
		//! @debug
		SetChargePumpOn();
		
		ActivateDisplay();	
		
		SetDisplayClock(0x80);			// Set Clock as 175 Frames/Sec
		SetMultiplexRatio(0x1F);		// 1/32 Duty (0x0F~0x3F)
		SetDisplayOffset(0x00);			// Shift Mapping RAM Counter (0x00~0x3F)
		SetStartLine(0x00);				// Set Mapping RAM Display Start Line (0x00~0x3F)
		SetAddressingMode(SSD1306::PAGE_ADDRESSING_MODE);		// Set Page Addressing Mode (0x00/0x01/0x02)
		SetSegmentRemap(0xA1);			// Set SEG/Column Mapping (0xA0/0xA1)
		SetCommonRemap(0xC8);			// Set COM/Row Scan Direction (0xC0/0xC8)
		
		// This next one is important for pixels to be mapped correctly.
		// Sets COM pin config to sequential mapped
		SetComPinConfig(0x02);
		
		// Set contrast to full
		SetContrastControl(0xFF);
		
		// Set pre-charge as 15 DCLKs and discharge as 1 DCLKs (0b11110001)
		//ssd1306.SetPrechargePeriod(0xF1);		
		SetPrechargePeriod(0xF1);
		
		// Make screen blank
		FillRam(0x00);
	}

	void SSD1306::Reset()
	{
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Resetting...\r\n");
		#endif
		
		port.PullResetLow();
		
		// Delay for at least 3us (as per datasheet)
		//! @warning Processor stalling delay
		port.DelayUs(ssd1306_RESET_DELAY_TIME_US);
		
		port.PullResetHigh();
	}

	void SSD1306::SetChargePumpOn()
	{
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Enabling charge pump...\r\n");
		#endif
		
		uint8 commandArray[2];

		// Enable charge pump
		commandArray[0] = ssd1306_REG_ADD_CHARGE_PUMP;
		commandArray[1] = ssd1306_REG_VAL_CHARGE_PUMP_ENABLED;
		
		WriteCommandArray(commandArray, 2);				
	}

	void SSD1306::SetChargePumpOff()
	{
		uint8 commandArray[2];

		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Disabling charge pump.\r\n");
		#endif
		
		// Set to charge pump address
		commandArray[0] = ssd1306_REG_ADD_CHARGE_PUMP;
		commandArray[1] = ssd1306_REG_VAL_CHARGE_PUMP_DISABLED;
		
		// Send command to IC
		WriteCommandArray(commandArray, 2);				
	}

	void SSD1306::ActivateDisplay()	
	{
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Turning display on...\r\n");
		#endif
		
		// Set display on
		WriteCommand(ssd1306REG_VAL_ACTIVATE_DISPLAY);		
	}

	void SSD1306::DeactivateDisplay()	
	{
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Turning display off...\r\n");
		#endif
		
		// Set display off
		WriteCommand(ssd1306REG_VAL_DEACTIVATE_DISPLAY);		
	}

	void SSD1306::TurnEveryPixelOn()
	{
		uint8 commandArray[1];
		
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Turning entire display on...\r\n");
		#endif
		
		// Turn all pixels on
		commandArray[0] = ssd1306REG_VAL_EVERY_PIXEL_ON;
		WriteCommandArray(commandArray, 1);		
	}

	void SSD1306::TurnEveryPixelOff()
	{
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Turning entire display off...\r\n");
		#endif
		
		uint8 commandArray[1];

		// Turn all pixels off
		commandArray[0] = ssd1306REG_VAL_EVERY_PIXEL_OFF;
		WriteCommandArray(commandArray, 1);	
	}

	void SSD1306::ShowPattern(
		uint8 *pixelArray, 
		uint8 startPage,
		uint8 endPage,
		uint8 startCol,
		uint8 totalCol)
	{
		uint8 *pixelPointer;
		uint8 i,j;

		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Showing pattern...\r\n");
		#endif

		// Assign pixel pointer to the start of the array
		pixelPointer = &pixelArray[0];
		
		//! @todo Add bounds checking
		for(i = startPage; i < (endPage+1); i++)
		{
			SetStartPage(i);
			SetStartColumn(startCol);
			
			// Start transmission
			port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
			
			// Indicate we are writing data (ssd1306DATA_FOLLOWS_BYTE), not a command (0x00)
			port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);

			for(j = 0; j < totalCol; j++)
			{
				// Write byte of pixels (settings for 8 pixels)
				port.I2cMasterWriteByte(*pixelPointer);
				pixelPointer++;
			}

			// End transmission
			port.I2cMasterSendStop();
		}
	}

	void SSD1306::WriteData(uint8 cmd)
	{
		port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
		port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);
		port.I2cMasterWriteByte(cmd);
		port.I2cMasterSendStop();	
	}

	void SSD1306::FillRam(unsigned char byteToFillRamWith)
	{
		// Fills RAM with constant single byte
		unsigned char i,j;
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Filling RAM with constant byte...\r\n");
		#endif

		for(i = 0; i < 4; i++)
		{
			SetStartPage(i);
			SetStartColumn(0x00);

			port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
			port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);

			for(j = 0; j < 128; j++)
			{
				port.I2cMasterWriteByte(byteToFillRamWith);
			}

			port.I2cMasterSendStop();
		}
	}

	void SSD1306::SetDisplayClock(uint8 regVal)
	{
		uint8 command[2];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting display clock...\r\n");
		#endif
		
		// Set Display Clock Divide Ratio / Oscillator Frequency
		command[0] = ssd1306_REG_ADD_CLOCK_DIVIDE_RATIO_AND_OSC_FREQ;
		//   Default => 0x80
		//     D[3:0] => Display Clock Divider
		//     D[7:4] => Oscillator Frequency
		command[1] = regVal;
			
		WriteCommandArray(command, 2);			
				
	}

	void SSD1306::SetAddressingMode(addressingMode_t addressingMode)
	{
		uint8 cmdArray[2];

		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting addressing mode...\r\n");
		#endif
		
		// Set Memory Addressing Mode
		cmdArray[0] = 0x20;	
		
		// Default => 0x02
		// 0x00 => Horizontal Addressing Mode
		// 0x01 => Vertical Addressing Mode
		// 0x02 => Page Addressing Mode
		cmdArray[1] = (uint8)addressingMode;
		
		WriteCommandArray(cmdArray, 2);
	}

	void SSD1306::SetSegmentRemap(uint8 regVal)
	{
		uint8 cmdArray[1];
		
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Remapping segments...\r\n");
		#endif
		
		// Set Segment Re-Map
		// Default => 0xA0
		// 0xA0 => Column Address 0 Mapped to SEG0
		// 0xA1 => Column Address 0 Mapped to SEG127
		cmdArray[0] = regVal;	
		
		WriteCommandArray(cmdArray, 1);
	}

	void SSD1306::SetCommonRemap(uint8 regVal)
	{
		uint8 cmdArray[1];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting common remap...\r\n");
		#endif
		
		// Set COM Output Scan Direction
		//   Default => 0xC0
		//     0xC0 => Scan from COM0 to 63
		//     0xC8 => Scan from COM63 to 0
		cmdArray[0] = regVal;
		
		WriteCommandArray(cmdArray, 1);
	}

	void SSD1306::SetDisplayOffset(uint8 regVal)
	{
		uint8 cmdArray[2];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting display offset...\r\n");
		#endif
		
		// Set Display Offset
		cmdArray[0] = 0xD3;			
		// Default => 0x00
		cmdArray[1] = regVal;		
		
		WriteCommandArray(cmdArray, 2);
	}

	void SSD1306::SetStartLine(uint8 regVal)
	{
		uint8 cmdArray[1];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting start line...\r\n");
		#endif
		
		// Set Display Start Line
		// Default => 0x40 (0x00)
		cmdArray[0] = 0x40|regVal;
		
		WriteCommandArray(cmdArray, 1);
	}

	void SSD1306::SetComPinConfig(uint8 regVal)
	{
		uint8 cmdArray[2];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting common configuration settings...\r\n");
		#endif
		
		// Set COM Pins Hardware Configuration
		cmdArray[0] = 0xDA;			
		//   Default => 0x12
		//     Alternative COM Pin Configuration
		//     Disable COM Left/Right Re-Map
		cmdArray[1] = regVal;		
		
		WriteCommandArray(cmdArray, 2);
	}

	void SSD1306::SetContrastControl(uint8 d)
	{
		uint8 cmdArray[2];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting contrast control...\r\n");
		#endif
		
		// Set Contrast Control
		cmdArray[0] = 0x81;		
		//   Default => 0x7F
		cmdArray[1] = d;	
		
		WriteCommandArray(cmdArray, 2);
	}

	void SSD1306::SetPrechargePeriod(uint8 d)
	{
		uint8 cmdArray[2];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting pre-charge/discharge period...\r\n");
		#endif
		
		// Set pre-charge period
		cmdArray[0] = 0xD9;	
		//   Default => 0x22 (2 display clocks [phase 2] / 2 display clocks [phase 1])
		//     D[3:0] => Phase 1 period in 1-15 display clocks
		//     D[7:4] => Phase 2 period in 1-15 display clocks
		cmdArray[1] = d;	
		
		WriteCommandArray(cmdArray, 2);
	}

	void SSD1306::SetVCOMH(uint8 d)
	{
		uint8 cmdArray[2];
		
		#if(configDEBUG_SSD1306 == 1)
			port.PrintDebug("SSD1306: Setting VCOMH level...\r\n");
		#endif
		
		// Set VCOMH Deselect Level
		cmdArray[0] = 0xDB;	
		// Default => 0x20 (0.77*VCC)
		cmdArray[1] = d;	
		
		WriteCommandArray(cmdArray, 2);
	}

	void SSD1306::SetInverseDisplay(bool isInverse)
	{
		uint8 cmdArray[1];
		
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Chaning inverse display settings...\r\n");
		#endif
		
		// Set inverse display on/off
		// 0xA6 => normal display (default)
		// 0xA7 => inverse display
		if(isInverse)
			cmdArray[0] = 0xA7;
		else
			cmdArray[0] = 0xA6;
		
		WriteCommandArray(cmdArray, 1);
	}

	//===============================================================================================//
	//====================================== PRIVATE FUNCTIONS ======================================//
	//===============================================================================================//

	//=========================================== I/O WRAPPER FUNCTIONS =============================//



	//========================================= WRITING COMMANDS ====================================//

	void SSD1306::WriteCommand(unsigned char cmd)
	{
		uint8 error;
		
		error = port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
		
		if(error != I2cCpLeft_MSTR_NO_ERROR)
		{
			#if(configDEBUG_SSD1306_ERROR == 1)
				port.PrintDebug((char*)"SSD1306: Error. Sending start condition over I2C failed.\r\n");
			#endif
		}
		
		error = port.I2cMasterWriteByte(ssd1306CMD_FOLLOWS_BYTE);
		
		if(error != I2cCpLeft_MSTR_NO_ERROR)
		{
			#if(configDEBUG_SSD1306_ERROR == 1)
				port.PrintDebug((char*)"SSD1306: Error. Sending byte over I2C failed.\r\n");
			#endif
		}
		
		error = port.I2cMasterWriteByte(cmd);
		
		if(error != I2cCpLeft_MSTR_NO_ERROR)
		{
			#if(configDEBUG_SSD1306_ERROR == 1)
				port.PrintDebug((char*)"SSD1306: Error. Sending byte over I2C failed.\r\n");
			#endif
		}
		
		error = port.I2cMasterSendStop();
		
		if(error != I2cCpLeft_MSTR_NO_ERROR)
		{
			#if(configDEBUG_SSD1306_ERROR == 1)
				port.PrintDebug("SSD1306: Error. Sending stop condition over I2C failed.\r\n");
			#endif
		}
			
		
	}

	void SSD1306::WriteCommandArray(uint8 *commandArray, uint8 numCommands)
	{
		uint8 error;
		uint8 commandIndex = 0;
		
		error = port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
		
		if(error != I2cCpLeft_MSTR_NO_ERROR)
		{
			#if(configDEBUG_SSD1306_ERROR == 1)
				port.PrintDebug("SSD1306: Error. Sending start condition over I2C failed.\r\n");
			#endif
		}
		
		// Write byte to indicate a command rather than data is coming
		error = port.I2cMasterWriteByte(ssd1306CMD_FOLLOWS_BYTE);
		
		if(error != I2cCpLeft_MSTR_NO_ERROR)
		{
			#if(configDEBUG_SSD1306_ERROR == 1)
				port.PrintDebug("SSD1306: Error. Sending byte over I2C failed.\r\n");
			#endif
		}
		
		// Write the command bytes
		while(commandIndex < numCommands)
		{
			error = port.I2cMasterWriteByte(commandArray[commandIndex]);
			
			if(error != I2cCpLeft_MSTR_NO_ERROR)
			{
				#if(configDEBUG_SSD1306_ERROR == 1)
					port.PrintDebug("SSD1306: Error. Sending byte over I2C failed.\r\n");
				#endif
			}
			
			commandIndex++;
		}
		
		error = port.I2cMasterSendStop();
		
		if(error != I2cCpLeft_MSTR_NO_ERROR)
		{
			#if(configDEBUG_SSD1306_ERROR == 1)
				port.PrintDebug("SSD1306: Error. Sending stop condition over I2C failed.\r\n");
			#endif
		}
	}



	//===============================================================================================//
	//  Instruction Setting
	//===============================================================================================//

	//! @brief		Sets the start column
	//! @private
	void SSD1306::SetStartColumn(unsigned char d)
	{
		// Set Lower Column Start Address for Page Addressing Mode (modulo 16)
		//   Default => 0x00
		WriteCommand(0x00+d%16);	
		// Set Higher Column Start Address for Page Addressing Mode (dividsor 16)
		//   Default => 0x10
		WriteCommand(0x10+d/16);		
	}

	void SSD1306::SetColumnAddress(unsigned char a, unsigned char b)
	{
		WriteCommand(0x21);			// Set Column Address
		WriteCommand(a);			//   Default => 0x00 (Column Start Address)
		WriteCommand(b);			//   Default => 0x7F (Column End Address)
	}


	void SSD1306::SetPageAddress(unsigned char a, unsigned char b)
	{
		WriteCommand(0x22);			// Set Page Address
		WriteCommand(a);			//   Default => 0x00 (Page Start Address)
		WriteCommand(b);			//   Default => 0x07 (Page End Address)
	}

	void SSD1306::SetMultiplexRatio(uint8 regVal)
	{
		uint8 commandArray[2];
		
		#if(configDEBUG_SSD1306_VERBOSE == 1)
			port.PrintDebug("SSD1306: Setting multiplex ratio...\r\n");
		#endif
		
		// Set Multiplex Ratio
		commandArray[0] = 0xA8;
		//   Default => 0x3F (1/64 Duty)
		commandArray[1] = regVal;
		
		WriteCommandArray(commandArray, 2);		
	}

	void SSD1306::SetStartPage(unsigned char d)
	{
		// Set Page Start Address for Page Addressing Mode
		//   Default => 0xB0 (0x00)
		WriteCommand(0xB0|d);
	}

	void SSD1306::SetNOP()
	{
		WriteCommand(0xE3);			// Command for No Operation
	}

	void SSD1306::FillBlock(
		unsigned char byteToFillRamWith,
		unsigned char startPage,
		unsigned char endPage,
		unsigned char startCol,
		unsigned char numCols)
	{
		unsigned char i,j;
		
		// Can be partial or full screen
		for(i = startPage; i < (endPage + 1); i++)
		{
			SetStartPage(i);
			SetStartColumn(startCol);

			port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
			port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);

			for(j = 0; j < numCols; j++)
			{
				port.I2cMasterWriteByte(byteToFillRamWith);
			}

			port.I2cMasterSendStop();
		}
	}

	void SSD1306::Checkerboard()
	{
		unsigned char i,j;
		
		// Checkerboard is full-screen
		for(i = 0; i < 8; i++)
		{
			SetStartPage(i);
			SetStartColumn(0x00);

			port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
			port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);

			for(j = 0; j < 64; j++)
			{
				port.I2cMasterWriteByte(0x55);
				port.I2cMasterWriteByte(0xAA);
			}

			port.I2cMasterSendStop();
		}
	}

	void SSD1306::DrawFrame()
	{
		unsigned char i,j;
		
		// Draw 1-pixel wide top border
		
		// Full-screen
		SetStartPage(0x00);
		//SetStartColumn(XLevel);
		SetStartColumn(0x00);

		port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
		port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);

		for(i = 0; i < ssd1306NUM_COLS; i++)
		{
			port.I2cMasterWriteByte(0x01);
		}

		port.I2cMasterSendStop();
	
		// Draw 1-pixel wide bottom border

		SetStartPage(0x03);
		SetStartColumn(0x00);
		//SetStartColumn(XLevel);

		port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
		port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);

		for(i = 0; i< ssd1306NUM_COLS; i++)
		{
			port.I2cMasterWriteByte(0x80);
		}

		port.I2cMasterSendStop();

		// Draw 1-pixel wide left and right borders

		for(i = 0; i < 4; i++)
		{
			SetStartPage(i);

			for(j = 0; j < ssd1306NUM_COLS; j += (ssd1306NUM_COLS-1))
			{
				SetStartColumn(j);
				WriteData(0xFF);
			}
		}
	}

	#if(ssd1306ENABLE_FONTS == 1)
		void SSD1306::ShowFont57(
			unsigned char fontArraySel,
			unsigned char asciiChar,
			unsigned char startPage,
			unsigned char startCol)
		{
			
			const unsigned char *Src_Pointer;
			unsigned char i;

			switch(fontArraySel)
			{
				case 1:
					Src_Pointer = &Ascii_1[(asciiChar - 1)][0];
					break;
				case 2:
					Src_Pointer = &Ascii_2[(asciiChar - 1)][0];
					break;
			}
			SetStartPage(startPage);
			SetStartColumn(startCol);

			port.I2cMasterSendStart(SSD1306_I2C_ADD, 0);
			port.I2cMasterWriteByte(ssd1306DATA_FOLLOWS_BYTE);

			for(i = 0; i < 5; i++)
			{
				port.I2cMasterWriteByte(*Src_Pointer);
				Src_Pointer++;
			}
			
			port.I2cMasterWriteByte(0x00);
			port.I2cMasterSendStop();
			
		}
	
		void SSD1306::ShowString(
			uint8_t databaseNum,
			char *msg,
			uint8_t startPage,
			uint8_t startCol)
		{
			char *internalPtr;

			internalPtr = msg;
			ShowFont57(1, 96, startPage, startCol);			// No-Break Space
								//   Must be written first before the string start...

			while(1)
			{
				ShowFont57(databaseNum, *internalPtr, startPage, startCol);
				internalPtr++;
				startCol += 6;
				// Check for terminating null character
				if(*internalPtr == 0)
					break;
			}
		}
	#endif

	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//  Vertical / Fade Scrolling (Partial or Full Screen)
	//
	//    a: Scrolling Direction
	//       "0x00" (Upward)
	//       "0x01" (Downward)
	//    b: Set Top Fixed Area
	//    c: Set Vertical Scroll Area
	//    d: Set Numbers of Row Scroll per Step
	//    e: Set Time Interval between Each Scroll Step
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	void SSD1306::VerticalScroll(
		unsigned char a,
		unsigned char b,
		unsigned char c,
		unsigned char d,
		unsigned char e)
	{
		unsigned int i,j;	

		WriteCommand(0xA3);			// Set Vertical Scroll Area
		WriteCommand(b);			//   Default => 0x00 (Top Fixed Area)
		WriteCommand(c);			//   Default => 0x40 (Vertical Scroll Area)

		switch(a)
		{
			case 0:
				for(i = 0; i < c; i += d)
				{
					SetStartLine(i);
					for(j = 0; j < e; j++)
					{
						port.DelayUs(200);
					}
				}
				break;
			case 1:
				for(i = 0; i < c; i += d)
				{
					SetStartLine(c-i);
					for(j = 0; j < e; j++)
					{
						port.DelayUs(200);
					}
				}
				break;
		}
		SetStartLine(0x00);
	}

	void SSD1306::HorizontalScroll(
		unsigned char scrollDir,
		unsigned char startPage,
		unsigned char endPage,
		unsigned char timeInterval,
		unsigned char delayTime)
	{
		// Setup horizontal scroll.
		WriteCommand(0x26|scrollDir);		
		
		// Dummy write for first parameter
		WriteCommand(0x00);		
		WriteCommand(startPage);
		WriteCommand(timeInterval);
		WriteCommand(endPage);
		
		// Activate Scrolling
		WriteCommand(0x2F);			
		port.DelayMs(delayTime);
	}

	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//  Continuous Vertical / Horizontal / Diagonal Scrolling (Partial or Full Screen)
	//
	//    a: Scrolling Direction
	//       "0x00" (Vertical & Rightward)
	//       "0x01" (Vertical & Leftward)
	//    b: Define Start Row Address (Horizontal / Diagonal Scrolling)
	//    c: Define End Page Address (Horizontal / Diagonal Scrolling)
	//    d: Set Top Fixed Area (Vertical Scrolling)
	//    e: Set Vertical Scroll Area (Vertical Scrolling)
	//    f: Set Numbers of Row Scroll per Step (Vertical / Diagonal Scrolling)
	//    g: Set Time Interval between Each Scroll Step in Terms of Frame Frequency
	//    h: Delay Time
	//    * d+e must be less than or equal to the Multiplex Ratio...
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	void SSD1306::ContinuousScroll(
		unsigned char a,
		unsigned char b,
		unsigned char c,
		unsigned char d,
		unsigned char e,
		unsigned char f,
		unsigned char g,
		unsigned char h)
	{
		WriteCommand(0xA3);			// Set Vertical Scroll Area
		WriteCommand(d);			//   Default => 0x00 (Top Fixed Area)
		WriteCommand(e);			//   Default => 0x40 (Vertical Scroll Area)

		WriteCommand(0x29+a);			// Continuous Vertical & Horizontal Scroll Setup
		WriteCommand(0x00);			//           => (Dummy Write for First Parameter)
		WriteCommand(b);
		WriteCommand(g);
		WriteCommand(c);
		WriteCommand(f);
		WriteCommand(0x2F);			// Activate Scrolling
		port.DelayMs(h);
	}

	void SSD1306::DeactivateScroll()
	{
		// Deactivate Scrolling
		WriteCommand(0x2E);			
	}

	void SSD1306::FadeIn()
	{
		unsigned int i;	

		ActivateDisplay();
		for(i = 0; i < (ssd1306BRIGHTNESS + 1); i++)
		{
			SetContrastControl(i);
			port.DelayUs(200);
			port.DelayUs(200);
			port.DelayUs(200);
		}
	}

	void SSD1306::FadeOut()
	{
		unsigned int i;	

		for(i = (ssd1306BRIGHTNESS + 1); i > 0; i--)
		{
			SetContrastControl(i-1);
			port.DelayUs(200);
			port.DelayUs(200);
			port.DelayUs(200);
		}
		DeactivateDisplay();
	}

	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	//  Sleep Mode
	//
	//    "0x00" Enter Sleep Mode
	//    "0x01" Exit Sleep Mode
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	void SSD1306::Sleep(unsigned char a)
	{
		switch(a)
		{
			case 0:
				DeactivateDisplay();
				TurnEveryPixelOn();
				break;
			case 1:
				TurnEveryPixelOff();
				ActivateDisplay();
				break;
		}
	}

	void SSD1306::Test()
	{
		unsigned char i;

		//! #todo Implement reset
		// PIN_OLED_RESET_Write(0);
		//RES=0;
		
		for( i = 0; i < 200; i++)
		{
			port.DelayUs(200);
		}
		
		//! #todo Implement reset
		//PIN_OLED_RESET_Write(1);
		// RES=1;

		TurnEveryPixelOn();		// Enable Entire Display On (0xA4/0xA5)

		while(1)
		{
			ActivateDisplay();	// Display On (0xAE/0xAF)
			port.DelayMs(2);
			DeactivateDisplay();	// Display Off (0xAE/0xAF)
			port.DelayMs(2);
		}
	}

	//===============================================================================================//
	//============================================ GRAVEYARD ========================================//
	//===============================================================================================//

	// none

} // namespace SSD1306Ns

#endif	// #if(configINCLUDE_CAP_SENSE == 1)

// EOF
