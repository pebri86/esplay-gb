# esp32-micro-gb
----------------

Port of GNUBoy to ESP32, i use WROOM-32 module but applicable to other esp32 that has similar specs or higher like WROVER.

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
CLK | 19
CS | 22
DC | 5
RST | 18
BCKL | 17

(BCKL = backlight enable - PWM Controlled)

Also connect the power supply and ground. For now, the LCD is controlled using a DMA controller and fed to 2nd CPU. You can change different layout of lcd pins and type of LCD using menuconfig to adapt your hardware configuration, just select custom hardware on menuconfig and change your pins settings. If you have another type of spi lcd display write your own driver and place in driver folder, adjust the code to use your newly created lcd driver. please notify me if you write your own driver so i can add in the repo.


Controller
----------

To control the NES, connect GPIO pins to a common ground pcb gamepad:

Key | GPIO
---- | ----
A | 0
B | 4
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
Use script flashrom.sh to flash your ROM. Please provide ROM by yourself.

