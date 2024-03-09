/*Servots vinkel: 
864 = vänster 
1399 = rak 
1926 = höger*/

/*Den här koden ger 1 sampling / 8,4 ms*/

/*Sensorns bibliotek*/
#include <OPT3101.h>
/*I2C-biblioteket*/
#include <Wire.h>

/*Skapar en instans med alla delar som
OPT3101-biblioteket innehåller och
döper den till "sensor"*/
OPT3101 sensor;

/*Array som håller värdena
för de tre sensor-områdena*/
int16_t distances[3];
/*Variabel som indikerar när
data kan tas emot från sensorn*/
volatile bool dataReady = false;

void setDataReadyFlag() {
  dataReady = true;
}

/* * * * * * * * * * * * * * * * * * *
* Nedanstående funktioner sätter ett *
* värde för att variera pulsbredden  *
* på PWM-signalen som genereras.     *
* 864(vänster)    = 4,32% duty cycle *
* 1399(rakt fram) = 7,00% duty cycle *
* 1926(höger)     = 9,63% duty cycle *
* * * * * * * * * * * * * * * * * * */

/*Vänstersväng*/
void left_turn() {
  OCR1A = 864; 
}

/*Högersväng*/
void right_turn() {
  OCR1A = 1926; 
}

/*Sväng med valfri vinkel*/
void specific_turn(uint16_t stop) {
  OCR1A = stop; 
}

/*Sätter vinkeln rak*/
void straight_ahead() {
  OCR1A = 1399; 
}

void setup() {
  /*Sätter igång monitor-funktionen i Arduino IDE*/
  Serial.begin(9600);
  /*Sätter igång I2C-kommunikation*/
  Wire.begin();
  /*Startar upp sensorn*/
  sensor.init();
  /*Ställer in sensorn att göra
  så snabba samplingar som möjligt*/
  sensor.setContinuousMode();
  /*Ställer in att GP1 ska
  användas för continous mode*/
  sensor.enableDataReadyOutput(1);
  /*Väljer hur många samples som
  ska användas per dataleverans*/
  sensor.setFrameTiming(32);
  /*Vet ej detaljer om nedanstående 4 funktioner :(*/
  sensor.setChannel(OPT3101ChannelAutoSwitch);
  sensor.setBrightness(OPT3101Brightness::Adaptive);
  attachInterrupt(digitalPinToInterrupt(2), setDataReadyFlag, RISING);
  sensor.enableTimingGenerator();
  /*PWM-inställning inklusive
  frekvens = 50 Hz*/
  TCCR1A = 0b10000000;
  TCCR1B = 0b00010010;
  ICR1 = 20000;
  /*Sätter D9 som styrpin*/
  DDRB = 0b00000010;
  /*Rak vinkel*/
  OCR1A = 1399;
}

void loop() {
  /*Hämtar avståndsvärden från sensorn
  och lägger i arrayen distances*/
  if(dataReady) {
    sensor.readOutputRegs();
    distances[sensor.channelUsed] = sensor.distanceMillimeters;
    dataReady = false;
  }
  /*Svänger åt rätt håll baserat
  på värdet från arrayen distances
  samt skriver ut respektive
  sensors värde när den används*/
  if(distances[0] < 300) {
    right_turn();
    Serial.print("LEFT: ");
    Serial.print(distances[0]);
    Serial.println();
  }
  else if(distances[2] < 300) {
    left_turn();
    Serial.print("RIGHT: ");
    Serial.print(distances[2]);
    Serial.println();
  } else {
    straight_ahead();
    Serial.print("FRONT: ");
    Serial.print(distances[1]);
    Serial.println();
  }
}
