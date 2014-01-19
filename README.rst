===========
ssd1306-cpp
===========

---------------------------------------------
Solomon SSD1306 128x64 OLED Controller Driver
---------------------------------------------

- Author: gbmhunter <gbmhunter@gmail.com> (http://www.cladlab.com)
- Created: 2012/01/25
- Last Modified: 2014/01/20
- Version: v2.1.0.0
- Company: CladLabs
- Project: Open-source Embedded Code Libraries
- Language: C++
- Compiler: GCC	
- uC Model: n/a
- Computer Architecture: n/a
- Operating System: n/a
- Documentation Format: Doxygen
- License: GPLv3

Description
===========

A open-source, portable, embedded driver for the Solomon SSD1306 IC, a 128x64 OLED controller.

Some ideas, original code, and font-libraries taken from Humphrey Lin (WiseChip Semiconductor Inc.)

Although the SSD1306 supports parallel, I2C or SPI comms, this driver supports the I2C interface only (commonly used with OLED screens). Change wrapper functions to suit platform.

The I2C address of the SSD1306 depends on whether the SA0 pin is pulled high or low. To define the address, set either :code:`SSD1306_I2C_SA0_0` or :code:`SSD1306_I2C_SA0_1` to :code:`1` in `Port.hpp` (but not both).

This driver has functions to automatically set up SSD1306 into common configurations for OLED screens. This takes out much of the work in going through and changing all the settings manually.

::

	// Initialises OLED screen when Vcc is supplied internally.
	OledInitVccInt();

	// Initialises OLED screen when Vcc is supplied externally.
	OledInitVccExt();
	
	

Pixel 0,0 is in the top corner of the side with FPC connector


This driver can be used with it's internal font library, or an external graphics library if desired (or nothing if you want do just display simple stuff). To enable the internal fonts:

::

	#define ssd1306ENABLE_FONTS		1
	
Setting this to :code:`0` saves a bit of RAM memory, but you will not be able to use functions such as :code:`ShowFont57()` and :code:`ShowString()`.

Port Independence
-----------------

This driver is designed to be port-independent. All port-specific functions are defined in a separate file, Port.hpp/.cpp, and these functions require filling in with code specific to the platform you are using. 

Port-specific functions include the I2C communication functions, delays, controlling the reset pin to the SSD1306 chip, and the debug print function.

The driver has built-in support for some platforms already, and the built-in platform to use can be set with :code:`MCU_PLATFORM`.

::

	#define MCU_PLATFORM	PSOC

There is built-in support for the following platforms:
- PSoC 3
- PSoC 4
- PSoC 5
- PSoC 5LP

Feel free to add your own!

OS Support
----------

This driver is compatible with operating systems. You can add operating-system specific delay functions to Port.h/.c to force context switches and prevent wasted clock cycles.

Low Power
---------

This driver supports low power operation with the capability of controlling I/O pins that turn on/off power to the Vddb pin. Implementation of these functions is defined in `Port.cpp`. Other power saving methods include lowering the brightness with ... and turning of the display when not in use.

Compiling
=========

Integrate into your embedded project and compile as usual.

Usage
=====

::

	main()
	{
		// Use namespace to shorten code calls to library (optional)
		using SSD1306Ns;
		
		// Create SSD1306 object
		SSD1306 ssd1306;
		
		ssd1306.OledInitVccInt();
		
		// Main loop
		while(true)
		{
		
		}
	}
	
	
Changelog
=========

======== ========== =============================================================================================================================
Version  Date       Comment
======== ========== =============================================================================================================================
v2.1.0.0 2014/01/20 Renamed project from 'Cpp-SSD1306' to 'ssd1306-cpp' (following new project naming convention). Fixed .h/.c references to .hpp/.cpp. Moved '/src/include' files to 'include/' to follow new folder layout convention.
v2.0.1.0 2013/06/17 Renamed file extensions to .cpp/.hpp.
v2.0.0.0 2013/06/17 Added code to repository (in 'src' directory). First versioned commit.
v1.0.1.0 2012/12/14 Continuing development. Added multiple-byte command write function. Debugged existing functions. Added Doxygen comments.
v1.0.0.0 2012/02/24 Modified existing c file provided by WiseChip. Added header file, made this file specific to the controller, not OLED screen. 
======== ========== =============================================================================================================================