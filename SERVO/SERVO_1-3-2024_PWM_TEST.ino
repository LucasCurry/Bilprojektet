/*Servots vinkel: 
864 = vänster 
1399 = rak 
1926 = höger*/

void setup() {
  /*PWM-inställning 
  inklusive frekvens*/
  TCCR1A = 0b10000000;
  TCCR1B = 0b00010010;
  ICR1 = 20000;
  /*Sätter D9 som styrpin*/
  DDRB = 0b00000010;
  /*Rak vinkel*/
  OCR1A = 1399;
}

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

void loop() {
  delay(3000);
  right_turn();
  delay(1000);
  left_turn();
  delay(1000);
  straight_ahead();
  delay(2000);
  right_turn();
  delay(500);
  specific_turn(1000);
  delay(500);
  specific_turn(1800);
  delay(500);
  specific_turn(1200);
  delay(500);
  specific_turn(1600);
  delay(500);
  straight_ahead();
  delay(5000);
}
