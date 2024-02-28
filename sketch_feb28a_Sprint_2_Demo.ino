const int motorPin = 9; 

/*Sätter pin 9 till output*/
void setup() {
  pinMode(motorPin, OUTPUT);
}

/*Demo som kör bilen i olika hastigheter*/
void loop() {
  for(uint16_t j = 0; j < 600; j++) {
    digitalWrite (motorPin, HIGH);
    delay(1);
    digitalWrite (motorPin, LOW);
    delay(2);
  }
  delay(3000);
  for(uint16_t j = 0; j < 400; j++) {
    digitalWrite (motorPin, HIGH);
    delay(3);
    digitalWrite (motorPin, LOW);
    delay(1);
  }
  delay(3000); 
  for(uint16_t j = 0; j < 20; j++) {
    digitalWrite (motorPin, HIGH);
    delay(50);
    digitalWrite (motorPin, LOW);
    delay(1);
  }
  delay(6000);
}