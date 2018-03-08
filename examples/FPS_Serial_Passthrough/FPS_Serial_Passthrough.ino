/*****************************************************************
 * FPS_Serial_Passthrough.ino
 *
 *  By: Ho Yun "Bobby" Chan @ SparkFun Electronics
 *  Date: April 3rd, 2017
 *  Description: This is a basic serial passthrough code that sets
 *  up a software serial port to pass data between the Fingerprint Scanner
 *  and the SDK Demo Software (SDK_Demo.exe) provided by ADH-Tech. This
 *  code should work with the any model of ADH-Tech's FPS as long as
 *  you are within the minimum logic level threshold for the FPS serial UART.
 *  This code has been tested with these models:
 *
 *            GT-511C3  [ https://www.sparkfun.com/products/11792 ]
 *            GT-511C1R [ https://www.sparkfun.com/products/13007 ]
 *
 *  A WORD OF CAUTION: It is recommended to use a 3.3V FTDI basic breakout
 *  board [ https://www.sparkfun.com/products/9873 ] and mini-b USB cable
 *  [ https://www.sparkfun.com/products/11301 ]. There is a higher
 *  probability of having problems when using an Arduino as an intermediate device
 *  between the FPS and your computer. Using the FTDI as the USB-to-serial
 *  converter is more realiable and easier to get started. Once you have
 *  enrolled the fingerprints, Hawley's "FPS_IDFinger.ino" code should recognize
 *  the fingerprint templates saved in memory.

-------------------- HARDWARE HOOKUP with 5V Arduino --------------------

1.) Dedicated Bi-Directional Logic Level Converter (LLC)

It is recommended to use a dedicated bi-direcitonal LLC
[ https://www.sparkfun.com/products/12009 ] for a reliable connection if you
are using a 5V Arduino microcontroller:

   Fingerprint Scanner (Pin #) <-> Logic Level Converter <-> 5V Arduino w/ Atmega328P
     UART_TX (3.3V TTL)(Pin 1) <->     LV1 <-> HV1       <->  RX (pin 4)
     UART_RX (3.3V TTL)(Pin 2) <->     LV4 <-> HV4       <->  TX (pin 5)
           GND         (Pin 3) <->     GND <-> GND       <->     GND
      Vin (3.3V~6V)    (Pin 4) <->        HV             <->      5V
                                          LV             <->     3.3V

2.) Voltage Division w/ 3x 10kOhm Resistors

Otherwise, you could use 3x 10kOhm resistors [ https://www.sparkfun.com/products/11508 ]
to divide the voltage from a 5V Arduino down to 3.3V FPS similar to the
"Uni-Directional" application circuit on our old logic level converter
[ https://cdn.sparkfun.com/assets/b/0/e/1/0/522637c6757b7f2b228b4568.png ]:

    Voltage Divider         <-> Fingerprint Scanner(Pin #) <-> Voltage Divider <-> 5V Arduino w/ Atmega328P
                            <-> UART_TX (3.3V TTL) (Pin 1) <->                 <->       RX (pin 4)
  GND <-> 10kOhm <-> 10kOhm <-> UART_RX (3.3V TTL) (Pin 2) <->      10kOhm     <->       TX (pin 5)
          GND               <->        GND         (Pin 3) <->       GND       <->        GND
                            <->    Vin (3.3V~6V)   (Pin 4) <->                 <->        5V

Note: You can add the two 10kOhm resistors in series for 20kOhms. =)

--------------------------------------------------------------------------------
*****************************************************************/

// We'll use SoftwareSerial to communicate with the FPS:
#include <SoftwareSerial.h>

// set up software serial pins for Arduino's w/ Atmega328P's
// FPS (TX) is connected to pin 4 (Arduino's Software RX)
// FPS (RX) is connected through a converter to pin 5 (Arduino's Software TX)
SoftwareSerial fps(4, 5); // (Arduino SS_RX = pin 4, Arduino SS_TX = pin 5)

/*If using another Arduino microcontroller, try commenting out line 64 and
uncommenting line 73 due to the limitations listed in the
library's note => https://www.arduino.cc/en/Reference/softwareSerial . Do
not forget to rewire the connection to the Arduino.*/

// FPS (TX) is connected to pin 10 (Arduino's Software RX)
// FPS (RX) is connected through a converter to pin 11 (Arduino's Software TX)
//SoftwareSerial fps(10, 11); // (Arduino SS_RX = pin 10, Arduino SS_TX = pin 11)

void setup()
{
  // Set up both ports at 9600 baud since that is the FPS's default baud.
  // Make sure the baud rate matches the config setting of SDK demo software.
  Serial.begin(9600); //set up Arduino's hardware serial UART
  fps.begin(9600);    //set up software serial UART for FPS
}

void loop()
{
  if (Serial.available())
  { // If data comes in from serial monitor, send it out to FPS
    fps.write(Serial.read());
  }
  if (fps.available())
  { // If data comes in from FPS, send it out to serial monitor
    Serial.write(fps.read());
  }
}
