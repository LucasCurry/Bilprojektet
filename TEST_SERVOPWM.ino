void setup() {
  /*PWM-inställning 
  inklusive frekvens*/
  TCCR1A = 0b10000000;
  TCCR1B = 0b00010010;
  ICR1 = 20000;
  /*Servovinkel: 
  864 = vänster 
  1399 = rakt 
  1926 = höger*/
  OCR1A = 1399;
  /*Sätter D9 som styrpin*/
  DDRB = 0b00000010;
}

void loop() {
}