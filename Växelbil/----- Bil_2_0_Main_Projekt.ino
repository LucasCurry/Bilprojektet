/*Servots vinkel:
800 = vänster
1420 = rak
1940 = höger*/


#include <SharpDistSensor.h>

#define left 0
#define straight 1
#define right 2
#define min_speed 120
#define max_speed 255
#define min_right_turn 780
#define max_left_turn 2080
#define straight_turn 1480

/*Shapsensor initalize*/
const byte sensorPin1 = A2;
const byte sensorPin2 = A1;
const byte sensorPin3 = A0;


unsigned int distance1 = 0;
unsigned int distance2 = 0;
unsigned int distance3 = 0;

const byte medianFilterWindowSize = 5;



uint8_t reversevariable = 0;
uint8_t turn = straight;


// Create an object instance of the SharpDistSensor class
SharpDistSensor sensor1(sensorPin1, medianFilterWindowSize);
SharpDistSensor sensor2(sensorPin2, medianFilterWindowSize);
SharpDistSensor sensor3(sensorPin3, medianFilterWindowSize);

/* * * * * * * * * * * * * * * *
* Array som håller värdena     *
* för de tre sensor-områdena:  *
* [0] = vänster                *
* [1] = rakt fram              *
* [2] = höger                  *
* * * * * * * * * * * * * * * */
int16_t distances[3];

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
  OCR1A = min_right_turn;
}

/*Högersväng*/
void right_turn() {
  OCR1A = max_left_turn;
}

/*Sväng med valfri vinkel*/
void specific_turn(uint16_t stop) {
  if(stop < min_right_turn) {
    stop = min_right_turn;
  }
  else if(stop > max_left_turn) {
    stop = max_left_turn;
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

  /* Sätter sensormodellerna för de 3 sensornera
  sensor 1 och 3 har 10-80cm räckvidd och sitter
  på sidorna. Sensor 2 har 4-30cm räckvidd och 
  sitter frammåt. Baud rate sätts till 9600 så att vi kan läsa in värden från sensorn*/
  Serial.begin(9600);
  sensor1.setModel(SharpDistSensor::GP2Y0A21F_5V_DS);
  sensor2.setModel(SharpDistSensor::GP2Y0A41SK0F_5V_DS);
  sensor3.setModel(SharpDistSensor::GP2Y0A21F_5V_DS);

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
  while(!(PINC & 0b00010000));
}

void loop() {
  /*Hämtar värdena från sensorerna och lägger i en array */
  distances[2] = sensor1.getDist();
  distances[1] = sensor2.getDist();
  distances[0] = sensor3.getDist();
  // Print distance to Serial
  Serial.print(distances[0]);
  Serial.print(", ");
  Serial.print(distances[1]);
  Serial.print(", ");
  Serial.println(distances[2]);

  /*Svänger bilen beroende på dels hur
  nära respektive vägg är, dels vilken
  sensor som har närmast till sin vägg*/
  if(reversevariable == 0) {
    fwd(255);
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
