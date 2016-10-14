/***********************************************************************************************
TH_sensor
v1.0 13/02/2016 
Copyright (C) 2016 Maxime KELLER 
maxime.keller@gmail.com

Oregon Scientific protocol implementation is widely based on the work of Olivier LEBRUN
connectingStuff, Oregon Scientific v2.1 Emitter
http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
***********************************************************************************************/

#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include "LowPower.h"

// Connect VCC to 3-5VDC
// Connect GND to ground
// For GY-21
// Connect SCL to I2C clock pin (A5 on ATmega328)
// Connect SDA to I2C data pin (A4 on ATmega328)
// Connect GND to ground
// Connect VCC to D10
// For FS1000A 433Mhz transmiter
// Connect DATA Tx to D3
// Connect VCC to D2
// Connect GND to ground

#define TX_PIN  3
#define CP_PIN  10
#define PW_PIN  2
#define TIME  512
#define TWOTIME 1024

// Test mode if true
const boolean test = false;
// Activate lowpower mode if true
const boolean lowpower = false; // can prevent the sensor to read the temperature on some Arduino Board (unsolved)

long vcc=0;

#define SEND_HIGH() digitalWrite(TX_PIN, HIGH)
#define SEND_LOW() digitalWrite(TX_PIN, LOW)

// Sensor ID
#define MY_ID  0xA1

// sensor calibration coefficient
#define ADJ_T  -0.1
#define ADJ_H  -1

// battery detection threshold
#define LOWBAT 3500
#define DEEPLOWBAT 3300


#ifdef THN132N
  byte OregonMessageBuffer[8];
#else
  byte OregonMessageBuffer[9];
#endif

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

/**
 * \brief    Send logical "0" over RF
 * \details  azero bit be represented by an off-to-on transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first
 */
inline void sendZero(void)
{
  SEND_HIGH();
  delayMicroseconds(TIME);
  SEND_LOW();
  delayMicroseconds(TWOTIME);
  SEND_HIGH();
  delayMicroseconds(TIME);
}
 
/**
 * \brief    Send logical "1" over RF
 * \details  a one bit be represented by an on-to-off transition
 * \         of the RF signal at the middle of a clock period.
 * \         Remenber, the Oregon v2.1 protocol add an inverted bit first
 */
inline void sendOne(void)
{
   SEND_LOW();
   delayMicroseconds(TIME);
   SEND_HIGH();
   delayMicroseconds(TWOTIME);
   SEND_LOW();
   delayMicroseconds(TIME);
}
 
/**
* Send a bits quarter (4 bits = MSB from 8 bits value) over RF
*
* @param data Source data to process and sent
*/
 
/**
 * \brief    Send a bits quarter (4 bits = MSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void sendQuarterMSB(const byte data)
{
  (bitRead(data, 4)) ? sendOne() : sendZero();
  (bitRead(data, 5)) ? sendOne() : sendZero();
  (bitRead(data, 6)) ? sendOne() : sendZero();
  (bitRead(data, 7)) ? sendOne() : sendZero();
}
 
/**
 * \brief    Send a bits quarter (4 bits = LSB from 8 bits value) over RF
 * \param    data   Data to send
 */
inline void sendQuarterLSB(const byte data)
{
  (bitRead(data, 0)) ? sendOne() : sendZero();
  (bitRead(data, 1)) ? sendOne() : sendZero();
  (bitRead(data, 2)) ? sendOne() : sendZero();
  (bitRead(data, 3)) ? sendOne() : sendZero();
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/
 
/**
 * \brief    Send a buffer over RF
 * \param    data   Data to send
 * \param    size   size of data to send
 */
void sendData(byte *data, byte size)
{
  for(byte i = 0; i < size; ++i)
  {
    sendQuarterLSB(data[i]);
    sendQuarterMSB(data[i]);
  }
}
 
/**
 * \brief    Send an Oregon message
 * \param    data   The Oregon message
 */
void sendOregon(byte *data, byte size)
{
    sendPreamble();
    //sendSync();
    sendData(data, size);
    sendPostamble();
}
 
/**
 * \brief    Send preamble
 * \details  The preamble consists of 16 "1" bits
 */
inline void sendPreamble(void)
{
  byte PREAMBLE[]={0xFF,0xFF};
  sendData(PREAMBLE, 2);
}
 
/**
 * \brief    Send postamble
 * \details  The postamble consists of 8 "0" bits
 */
inline void sendPostamble(void)
{
#ifdef THN132N
  sendQuarterLSB(0x00);
#else
  byte POSTAMBLE[]={0x00};
  sendData(POSTAMBLE, 1); 
#endif
}
 
/**
 * \brief    Send sync nibble
 * \details  The sync is 0xA. It is not use in this version since the sync nibble
 * \         is include in the Oregon message to send.
 */
inline void sendSync(void)
{
  sendQuarterLSB(0xA);
}
 
/******************************************************************/
/******************************************************************/
/******************************************************************/
 
/**
 * \brief    Set the sensor type
 * \param    data       Oregon message
 * \param    type       Sensor type
 */
inline void setType(byte *data, byte* type)
{
  data[0] = type[0];
  data[1] = type[1];
}
 
/**
 * \brief    Set the sensor channel
 * \param    data       Oregon message
 * \param    channel    Sensor channel (0x10, 0x20, 0x30)
 */
inline void setChannel(byte *data, byte channel)
{
    data[2] = channel;
}
 
/**
 * \brief    Set the sensor ID
 * \param    data       Oregon message
 * \param    ID         Sensor unique ID
 */
inline void setId(byte *data, byte ID)
{
  data[3] = ID;
}
 
/**
 * \brief    Set the sensor battery level
 * \param    data       Oregon message
 * \param    level      Battery level (0 = low, 1 = high)
 */
void setBatteryLevel(byte *data, byte level)
{
  if(!level) data[4] = 0x0C;
  else data[4] = 0x00;
}
 
/**
 * \brief    Set the sensor temperature
 * \param    data       Oregon message
 * \param    temp       the temperature
 */
void setTemperature(byte *data, float temp)
{
  // Set temperature sign
  if(temp < 0)
  {
    data[6] = 0x08;
    temp *= -1; 
  }
  else
  {
    data[6] = 0x00;
  }
 
  // Determine decimal and float part
  int tempInt = (int)temp;
  int td = (int)(tempInt / 10);
  int tf = (int)round((float)((float)tempInt/10 - (float)td) * 10);
 
  int tempFloat =  (int)round((float)(temp - (float)tempInt) * 10);
 
  // Set temperature decimal part
  data[5] = (td << 4);
  data[5] |= tf;
 
  // Set temperature float part
  data[4] |= (tempFloat << 4);
}
 
/**
 * \brief    Set the sensor humidity
 * \param    data       Oregon message
 * \param    hum        the humidity
 */
void setHumidity(byte* data, byte hum)
{
    data[7] = (hum/10);
    data[6] |= (hum - data[7]*10) << 4;
}
 
/**
 * \brief    Sum data for checksum
 * \param    count      number of bit to sum
 * \param    data       Oregon message
 */
int Sum(byte count, const byte* data)
{
  int s = 0;
 
  for(byte i = 0; i<count;i++)
  {
    s += (data[i]&0xF0) >> 4;
    s += (data[i]&0xF);
  }
 
  if(int(count) != count)
    s += (data[count]&0xF0) >> 4;
 
  return s;
}
 
/**
 * \brief    Calculate checksum
 * \param    data       Oregon message
 */
void calculateAndSetChecksum(byte* data)
{
#ifdef THN132N
    int s = ((Sum(6, data) + (data[6]&0xF) - 0xa) & 0xff);
 
    data[6] |=  (s&0x0F) << 4;     data[7] =  (s&0xF0) >> 4;
#else
    data[8] = ((Sum(8, data) - 0xa) & 0xFF);
#endif
}
 
long readVcc()
{
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
} 


void setup()
{
  // set pinMode to OUTPUT
  pinMode(TX_PIN, OUTPUT);
  pinMode(PW_PIN, OUTPUT);
  pinMode(CP_PIN, OUTPUT);
  pinMode(13, OUTPUT);
  SEND_LOW();                   // put radio data output to 0
  digitalWrite(PW_PIN, HIGH);   // Turn the radio module on 
  digitalWrite(CP_PIN, HIGH);   // Turn the GY-21 sensor on
  
  randomSeed(analogRead(2)); // set random seed to a random reading in the air
  
#ifdef THN132N 
  // Create the Oregon message for a temperature only sensor (TNHN132N)
  byte ID[] = {0xEA,0x4C};
#else
  // Create the Oregon message for a temperature/humidity sensor (THGR2228N)
  byte ID[] = {0x1A,0x2D};
#endif 
  setType(OregonMessageBuffer, ID);
  setChannel(OregonMessageBuffer, 0x20); // Channel two but it could be 1,2 or 3
  setId(OregonMessageBuffer, MY_ID);
}


void loop()
{
    int k=1;
    int r=1;
    float temp=0;
    float humi=0;
      
    digitalWrite(PW_PIN, HIGH);  // Turn the radio module on 
    digitalWrite(CP_PIN, HIGH);  // Turn the GY-21 sensor on
    // Flash the on-board LED
    digitalWrite(13, HIGH);      
    delay(1);                     
    digitalWrite(13, LOW);
    delay(50);
    
    if (vcc <= LOWBAT)
    {
      delay(69);    
      digitalWrite(13, HIGH);      
      delay(30);                     
      digitalWrite(13, LOW);
    }
    else delay(99);
    
    if (!htu.begin()) // if the GY-21 dont answer, flash the LED continuously
    {
      while (1)
      {
        digitalWrite(13, HIGH);
        delay(20);
        digitalWrite(13, LOW);
        delay(480);
      }
    }
    
    if(!test) r = random(25,31);
    
    vcc = readVcc();
    
    if(vcc > LOWBAT) setBatteryLevel(OregonMessageBuffer, 1); else setBatteryLevel(OregonMessageBuffer, 0);
 
  // Prevent battery deep discharge (this can save the life of your NiMH battery)
    if(vcc <= DEEPLOWBAT)
    {
      SEND_LOW();
      digitalWrite(PW_PIN, LOW);
      digitalWrite(CP_PIN, LOW);
      digitalWrite(13, LOW);
      digitalWrite( SDA, LOW);
      digitalWrite( SCL, LOW);
      delay(100);
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); // sleep forever
    }
  
    temp = htu.readTemperature() + ADJ_T;
    
    if(test) temp=(float)vcc/100.0;
    
    humi = htu.readHumidity() + ADJ_H;
  
    setTemperature(OregonMessageBuffer, temp);
    setHumidity(OregonMessageBuffer, humi);
 
  // Calculate the checksum
  calculateAndSetChecksum(OregonMessageBuffer);
 
  // Send the Message over RF
  sendOregon(OregonMessageBuffer, sizeof(OregonMessageBuffer));
  // Send a "pause"
  SEND_LOW();
  delayMicroseconds(TWOTIME*8);
  
  // Send a copie of the first message. The v2.1 protocol send the
  // message two time
  sendOregon(OregonMessageBuffer, sizeof(OregonMessageBuffer));
 
  SEND_LOW();
  digitalWrite(PW_PIN, LOW);
  digitalWrite(CP_PIN, LOW);
  digitalWrite(13, LOW);
  
  if (lowpower)
  { 
    // set Arduino outputs to GND
    digitalWrite( SDA, LOW);
    digitalWrite( SCL, LOW);
  }
  
  for(k=1;k<=r;k++)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // Sleep for 8 seconds and re-sleep r time 
  }
  
}
