# ESPlay GNUBoy - ESP32 Gameboy emulator
----------------

Port of GNUBoy to ESP32, i use WROOM-32 module but applicable to other esp32 that has similar specs or higher like WROVER.

NEW FUNCTION
- SD Card Support, now you can load ROM from SD, also save and load emulator state on SD card.
- New UI menu for selecting rom on sdcard.
- Press Menu Button short time for save state action and then sleep the esp, press short again to wake up and load previously state file.
- Press Menu long time until menu UI appear to select ROM (play tab). to navigate the ui use select button to choose menu component, then use navigation button up/down/left/right to choose list of rom and then use A button to select rom.
- Choose settings tab for change screen brightness. 

Connect SDCard to the following pins

Pin | GPIO
---- | ----
MISO | 22
MOSI | 23
CLK | 18
CS | 4

Place ROMS in folder named 'roms' on root of sdcard, create following structure on sdcard:

	/-

	|

 	--roms (place your ROMS here)

 	|

 	--esplay

   		|

   		--data (this to place state file as .sav)


Compiling
---------

This code is an esp-idf project. You will need esp-idf to compile it. Newer versions of esp-idf may introduce incompatibilities with this code;
for your reference, the code was tested against release/v3.1 branch of esp-idf.


Display
-------

To display the NES output, please connect a 160x128 1,8" ST7735R or ILI9341 SPI display to the ESP32 in this way:

Pin | GPIO
---- | ----
MOSI/SDA | 23
CLK | 18
CS | 17
DC | 16
RST | 19
BCKL | 5

(BCKL = backlight enable - PWM Controlled)

Also connect the power supply and ground. For now, the LCD is controlled using a DMA controller and fed to 2nd CPU. You can change different layout of lcd pins and type of LCD using menuconfig to adapt your hardware configuration, just select custom hardware on menuconfig and change your pins settings. If you have another type of spi lcd display write your own driver and place in driver folder, adjust the code to use your newly created lcd driver. please notify me if you write your own driver so i can add in the repo.


Controller
----------

To control the NES, connect GPIO pins to a common ground pcb gamepad:

Key | GPIO
---- | ----
A | 0
B | 13
START | 12
SELECT | 14
RIGHT | 27
DOWN | 25
UP | 33
LEFT | 32
MENU | 2

Also connect the ground line. Same like lcd pins you can change different layout of gamepad pins via menuconfig.

ROM
--- 
Place your ROM on SD Card in roms folder. Please provide ROM by yourself.

