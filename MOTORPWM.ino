void setup() {
  /*PWM-inställning*/
  TCCR2A = 0b10000011;
  TCCR2B = 0b00000001;
  /*Sätter D11 till motorpin*/
  DDRB = 0b00001000;
  /*Hastighet för motor med lägre gräns = 50 
  vid "kallstart" fungerar ej låga hastigheter 
  (<~90), då kan max-värde (255) sättas under 
  20 ms innan den låga hastigheten sätts*/
  OCR2A = 255;
  delay(20);
  OCR2A = 50;
}

void loop() {
}
