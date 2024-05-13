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

/*Konstanter för sensorn*/
#define left 0
#define straight 1
#define right 2

/*Konstanter för motorn*/
#define min_speed 150
#define max_speed 255

/*Konstanter för servot*/
#define min_turn 800       //Vänster
#define max_turn 1940      //Höger
#define straight_turn 1420 //Rakt fram

/*Variabler som används i programmet*/
uint8_t reverse_variable = 0;
uint8_t current_speed = 0;
uint8_t top_speed = 0;
uint8_t active_left = 0;
uint8_t active_right = 0;
uint8_t active_turn = 0;

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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Nedanstående funktioner sätter ett värde för att     *
* variera pulsbredden på PWM-signalen som genereras.   *
*                                                      *
* 800  (min_turn)      (vänster)   = 4,00% duty cycle  *
* 1420 (straight_turn) (rakt fram) = 7,10% duty cycle  *
* 1940 (max_turn)      (höger)     = 9,70% duty cycle  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

/*Framåt med hastighetsvärde 150(min) - 255(max)*/
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

/*Bakåt med hastighetsvärde 150(min) - 255(max)*/
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

  /*Ställer in att GP1 på sensorn ska
    användas för att skicka en signal när
    sensorn är redo att skicka nytt värde*/
  sensor.enableDataReadyOutput(1);

  /*Väljer hur många samples som
    ska användas per dataleverans*/
  sensor.setFrameTiming(32);

  /*Vet ej detaljer om nedanstående 4 funktioner :(*/
  /*setChannel ställer in vilken sensor som ska läsas av*/
  sensor.setChannel(OPT3101ChannelAutoSwitch);

  /*setBrightness ställer in sensorns
    ljusupptagningskänslighet*/
  sensor.setBrightness(OPT3101Brightness::Adaptive);

  /*Sätter D2 till interrupt att reagera på
    när ny data kan tas emot från sensorn*/
  attachInterrupt(digitalPinToInterrupt(2), setDataReadyFlag, RISING);

  /*Det saknas ens grundläggande förståelse
    för nedanstående funktion tyvärr*/
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

  /*Sätter topphastigheten för programmet*/
  top_speed = 220;
  current_speed = top_speed;

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
    sensor som har närmast till sin vägg.
    
    reverse_variable är 1 då backfunktionen
    är aktiverad*/
  if(reverse_variable == 0) {

    /*Bilens hastighet framåt bestäms
      innan svängningen avgörs.
      
      active_turn avgör om bilen är i en sväng eller rakt fram. 
      OBS! active_turn's värde bestäms inte med left/right/straight konstanterna!
      1 = bilen är i en sväng
      0 = bilen kör rakt fram*/
    fwd(current_speed);
    active_turn = 0;

    /*Då vänstersensorn är "hyfsat" nära en vägg och det är
      närmre vänstervägg än högervägg börjar bilen svänga
      höger baserat på avståndet till vänsterväggen.
      
      Då vänstersensorn är "väldigt" nära en vägg och det är
      närmre vänstervägg än högervägg svänger bilen fullt åt
      höger och en variabel aktiveras för en schmitt-trigger-effekt.
      
      Hastigheten anpassas baserat på avståndet till vänsterväggen*/
    if((distances[left] < 340) && (distances[left] < (distances[right] + 55))) {
      if(distances[left] < 230) {
        current_speed = (top_speed - (270 - distances[left]));
        active_right = 1;
        active_turn = 1;
        right_turn();
      }
      else if(active_right == 0) {
        active_turn = 1;
        current_speed = (top_speed - (270 - distances[left]));
        specific_turn((1940 - distances[left]) + 20);
      }
    }

    /*Då högersensorn är "hyfsat" nära en vägg och det är
      närmre högstervägg än vänstervägg börjar bilen svänga
      vänster baserat på avståndet till högerväggen.
      
      Då högersensorn är "väldigt" nära en vägg och det är
      närmre högervägg än vänstervägg svänger bilen fullt åt
      vänster och en variabel aktiveras för en schmitt-trigger-effekt.
      
      Hastigheten anpassas baserat på avståndet till högerväggen*/
    else if(((distances[right] + 55) < 340) && ((distances[right] + 55) < distances[left])) {
      if((distances[right] + 55) < 230) {
        current_speed = (top_speed - (270 - distances[left]));
        active_left = 1;
        active_turn = 1;
        left_turn();
      }
      else if(active_left == 0) {
        active_turn = 1;
        current_speed = (top_speed - (270 - (distances[right] + 55)));
        specific_turn((800 + (distances[right] + 55)) - 20);
      }
    }

    /*Då varken höger-, eller vänster-sensorerna känner av att en
      vägg är "väldigt" nära kör bilen rakt fram med full hastighet.
      
      Då respektive sensors avstånd blir större än "hyfsat" nollställs
      schmitt-triggern vilket innebär att varje svängriktning kommer
      reagera på ett "hyfsat" nära avstånd igen, istället för ett "väldigt"
      nära avstånd vilket var fallet efter schmitt-triggern aktiverats i
      respektive sväng-del av koden(rad 211 respektive 233 för höger-, samt vänster-sväng)*/
    if(active_turn == 0) {
      straight_ahead();
      current_speed = top_speed;
      if(distances[left] > 340) {
        active_right = 0;
      }
      if(distances[right] > 340) {
        active_left = 0;
      }
    }
  }

  /*Backar bilen om väggen framför är nära,
    och fortsätter backa till väggen framför
    är lite längre bort, á la schmitt-trigger.
    
    reverse_variable sätts och förbiser övrig
    kod för framåtkörning och svängning*/
  if((distances[straight] < 50) || (reverse_variable == 1)) {
    reverse_variable = 1;
    straight_ahead();
    reverse(max_speed);
    if(distances[straight] > 160) {
      reverse_variable = 0;
    }
  }
}
