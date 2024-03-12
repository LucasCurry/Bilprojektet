// This example shows how to use non-blocking code to read
// from all three channels on the OPT3101 continuously
// and store the results in arrays.
//
// In addition to the usual power and I2C connections, you
// will need to connect the GP1 pin to an interrupt on your
// Arduino. Use pin 2. To use a different pin, change the
// definition of dataReadyPin below, and refer to the
// documentation of attachInterrupt to make sure you
// pick a valid pin.
//
// "Advanced"-koden ger 1 sampling / 40 ms
// Den här koden ger 1 sampling / 8,4 ms

#include <OPT3101.h>
#include <Wire.h>

const uint8_t dataReadyPin = 2;

OPT3101 sensor;

int16_t distances[3];
volatile bool dataReady = false;

/* * * * * * * *
* Vänstersväng
* * * * * * * */
void left_turn() {
  OCR1A = 864; 
}

/* * * * * * *
* Högersväng
* * * * * * */
void right_turn() {
  OCR1A = 1926; 
}

/* * * * * * * * * * * * *
* Sväng med valfri vinkel
* * * * * * * * * * * * */
void specific_turn(uint16_t stop) {
  OCR1A = stop; 
}

/* * * * * * * * * * *
* Sätter vinkeln rak
* * * * * * * * * * */
void straight_ahead() {
  OCR1A = 1399;
}

void setDataReadyFlag()
{
  dataReady = true;
}

void setup()
{
  /*PWM-inställning 
  inklusive frekvens*/
  TCCR1A = 0b10000000;
  TCCR1B = 0b00010010;
  ICR1 = 20000;
  /*Sätter D9 som styrpin*/
  DDRB = 0b00000010;
  /*Rak vinkel*/
  OCR1A = 1399;
  Serial.begin(9600);
  Wire.begin();
  sensor.init();
  if (sensor.getLastError())
  {
    Serial.print(F("Failed to initialize OPT3101: error "));
    Serial.println(sensor.getLastError());
    while (1) {}
  }
  sensor.setContinuousMode();
  sensor.enableDataReadyOutput(1);
  sensor.setFrameTiming(32);
  sensor.setChannel(OPT3101ChannelAutoSwitch);
  sensor.setBrightness(OPT3101Brightness::Adaptive);
  attachInterrupt(digitalPinToInterrupt(dataReadyPin), setDataReadyFlag, RISING);
  sensor.enableTimingGenerator();
}

void loop()
{
  if (dataReady)
  {
    sensor.readOutputRegs();
    distances[sensor.channelUsed] = sensor.distanceMillimeters;
    if (sensor.channelUsed == 2)
    {
      for (uint8_t i = 0; i < 3; i++)
      {
        Serial.print(distances[i]);
        Serial.print(", ");
      }
      Serial.println();
    }
    dataReady = false;
  }

  if ((distances[0] > 350) && (distances[2] > 350)) {
    straight_ahead();
  } else if (distances[0] < 300) {
      right_turn(); 
  } else if (distances[2] < 300) {
      left_turn();
  } 
}
