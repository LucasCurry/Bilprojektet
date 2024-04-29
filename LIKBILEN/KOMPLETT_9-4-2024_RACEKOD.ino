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

#define left 0
#define straight 1
#define right 2
#define min_speed 120
#define max_speed 255
#define min_turn 800
#define max_turn 1940
#define straight_turn 1420

uint8_t reversevariable = 0;
uint8_t turn = straight;

/*Skapar en instans med alla delar som
OPT3101-biblioteket innehåller och
döper den till "sensor"*/
OPT3101 sensor;

/* * * * * * * * * * * * * * * *
* Array som håller värdena     *
* för de tre sensor-områdena:  *
* [0] = vänster                *
* [1] = rakt fram              *
* [2] = höger                  *
* * * * * * * * * * * * * * * */
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
  OCR1A = min_turn;
}

/*Högersväng*/
void right_turn() {
  OCR1A = max_turn;
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
  OCR1A = straight_turn;
}

/*Framåt med hastighetsvärde 80(min) - 255(max)*/
void fwd(int speed) {
  if(speed < min_speed) {
    speed = min_speed;
  }
  else if(speed > max_speed) {
    speed = max_speed;
  }
  OCR2A = speed;
  OCR2B = 0;
}

/*Bakåt med hastighetsvärde 80(min) - 255(max)*/
void reverse(uint8_t speed) {
    if(speed < min_speed) {
        speed = min_speed;
    }
    else if(speed > max_speed) {
      speed = max_speed;
    }
    OCR2B = speed;
    OCR2A = 0;
}

/*Broms genom låsning av hjulen*/
void brake() {
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
  /*PWM-inställning för motor*/
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
  PORTD = 0b10000000;
  /*Sätter start-svängningen i läge rakt fram*/
  OCR1A = straight_turn;
  /*Inväntar signal från start-modulen*/
  while(!(PIND & 0b00010000));
}

void loop() {
  /*Hämtar avståndsvärden från sensorn
  och lägger i arrayen distances*/
  if(dataReady) {
    sensor.readOutputRegs();
    distances[sensor.channelUsed] = sensor.distanceMillimeters;
    dataReady = false;
  }
  /*Svänger bilen beroende på dels hur
  nära respektive vägg är, dels vilken
  sensor som har närmast till sin vägg*/
  if(reversevariable == 0) {
    fwd(200);
    if((distances[left] < 254) && (distances[left] < distances[right])) {
      right_turn();
    }
    else if((distances[right] < 254) && (distances[right] < distances[left])) {
      left_turn();
    } else {
      straight_ahead();
    }
  }
  /*Backar bilen om väggen framför är nära,
  och fortsätter backa till väggen framför
  är lite längre bort, á la schmitt-trigger*/
  if((distances[straight] < 55) || (reversevariable == 1)) {
    reversevariable = 1;
    straight_ahead();
    reverse(200);
    if(distances[straight] > 200) {
      reversevariable = 0;
    }
  }
}
