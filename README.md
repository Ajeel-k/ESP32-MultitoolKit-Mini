 This system features a photoresistor, thermistor, time of flight sensor (for measuring distance), passive buzzer for sounds, 1.8 inch tft lcd & 4 buttons, and an esp32-c3 supermini microcontroller. It also has some small fun games.
 Components:
  ESP32-C3 supermini
  1.8 inch tft lcd ST7735S with 4 buttons
  Photoresistor
  Thermistor
  VL53L0X ToF sensor
  Passive buzzer
  Thin wires (I used 25AWG)
  2 1kΩ resistor
  2 10kΩ resistors
  Universal PCB

ESP32-C3 Supermini pinout:
  GPIO 0: Photoresistor Analog Input
  GPIO 1: Thermistor Analog Input
  GPIO 2: Passive Buzzer Output
  GPIO 3: Buttons B2 & B3 (Resistor Ladder Input)
  GPIO 4: ST7735S TFT Screen SCK (SPI Clock)
  GPIO 5: Button B1
  GPIO 6: ST7735 TFT Screen SDA
  GPIO 7: ST7735 TFT Screen CS
  GPIO 8: VL53L0X sensor SDA
  GPIO 9: VL53L0X sensor SCL
  GPIO 10: ST7735 TFT Screen RST
  GPIO 20: ST7735 TFT Screen DC
  GPIO 21: Button B4

  The ESP32-C3 SuperMini is a compact board with only 13 available GPIO pins. Since the project uses several components, including a photoresistor, thermistor, SPI display, ToF sensor, buzzer, and four buttons, the available GPIO pins were not sufficient. To overcome this limitation, buttons B2 and B3 share a single GPIO pin (GPIO 3) using a resistor ladder circuit. When B2 is pressed, GPIO 3 is connected directly to ground. When B3 is pressed, GPIO 3 is connected to ground through a 1 kΩ resistor. This produces different voltage levels, allowing the ESP32 to identify which button is pressed. To avoid floating of the adc pin when no buttons pressed, i have added 1k pull up to GPIO-3 from 3.3V. The photoresistor and thermistor use 10 kΩ resistors to form voltage divider circuits. As the sensor resistance changes, the voltage at the corresponding GPIO pin also changes, allowing the ESP32 to measure light intensity and temperature.

  Not mentioning the features here, explore your self!!
