/*Servots vinkel:
800 = vänster
1420 = rak
1940 = höger*/

/*Koden för sensoravläsning
ger 1 sampling per 8,4 ms*/

/*Sensorns bibliotek*/
#include <OPT3101.h>
/*I2C-biblioteket*/
#include <Wire.h>

#define min_speed 70
#define max_speed 255
#define min_turn 864
#define max_turn 1926

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

/* * * * * * * * * * * * * * * * * * * *
* Nedanstående funktioner sätter ett   *
* värde för att variera pulsbredden    *
* på PWM-signalen som genereras.       *
* 800 (vänster)    = 4,00% duty cycle  *
* 1420 (rakt fram) = 7,10% duty cycle  *
* 1940 (höger)     = 9,70% duty cycle  *
* * * * * * * * * * * * * * * * * * * */

/*Vänstersväng*/
void left_turn() {
  OCR1A = 800;
}

/*Högersväng*/
void right_turn() {
  OCR1A = 1940;
}

/*Sväng med valfri vinkel*/
void specific_turn(uint16_t stop) {
  if(stop < min_turn) {
    stop = min_turn;
  }
  else if(stop > max_turn) {
    stop = max_turn;
  }
  OCR1A = stop;
}

/*Sätter vinkeln rak*/
void straight_ahead() {
  OCR1A = 1420;
}

/*Framåt med hastighetsvärde 70(min) - 255(max)*/
void fwd(uint8_t speed) {
  PORTD = 0b10000000;
  if(speed < 100) {
    OCR2A = 120;
    OCR2B = 0;
    _delay_ms(20);
  }
  if(speed < min_speed) {
    speed = min_speed;
  }
  if(speed > max_speed) {
    speed = max_speed;
  }
  OCR2A = speed;
  OCR2B = 0;
}

/*Bakåt med hastighetsvärde 70(min) - 255(max)*/
void back(uint8_t speed) {
    PORTD = 0b10000000;
    if(speed < 100) {
        OCR2B = 120;
        OCR2A = 0;
        _delay_ms(20);
    }
    if(speed < min_speed) {
        speed = min_speed;
    }
    if(speed > max_speed) {
      speed = max_speed;
    }
    OCR2B = speed;
    OCR2A = 0;
}

/*Broms genom låsning av hjulen*/
void brake() {
    PORTD = 0b10000000;
    OCR2A = 0;
    OCR2B = 0;
}

void setup() {
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
  /*PWM-inställning för servo
  inklusive frekvens = 50 Hz*/
  TCCR1A = 0b10000000;
  TCCR1B = 0b00010010;
  ICR1 = 20000;
  /*PWM-inställning
  för motor*/
  TCCR2A = 0b10100011;
  TCCR2B = 0b00000001;
  /* * * * * * * * * * * * * * *
  * Sätter följande utgångar:  *
  * D3  = Motordrivare IN_2    *
  * D7  = Motordrivare EN_A    *
  * D9  = Servostyrning        *
  * D11 = Motordrivare IN_1    *
  * * * * * * * * * * * * * * */
  DDRB = 0b00001010;
  DDRD = 0b10001000;
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
  platser 0(vänster) och 2(höger)*/
  if(distances[0] < 350) {
    right_turn();
  }
  else if(distances[2] < 350) {
    left_turn();
  } else {
    straight_ahead();
  }
  /*Kör rakt fram om det inte är
  något nära den främre sensorn*/
  if(distances[1] < 200) {
    brake();
  } else {
    fwd(180);
  }
}