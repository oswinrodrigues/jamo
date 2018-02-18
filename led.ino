const int SER_PIN = 2;    // pin 14 on the 75HC595
const int RCLK_PIN = 3;   // pin 12 on the 75HC595
const int SRCLK_PIN = 4;  // pin 11 on the 75HC595

const int NUM_REGS = 16;  // total registers available
const int NUM_LEDS = 12;  // number of LEDs on fretboard
bool led[NUM_LEDS];       // total registers that are used

const bool ON = 1;        // LED state indicator
const bool OFF = 0;       // LED state indicator

/* 
 * The diagram below shows how LED
 * array indices are referenced
 * ---------------------
 *      Fret Number
 *   03     02     01
 * ---------------------
 *|--05--|------|------||
 *|--06--|--01--|------||
 *|--07--|--02--|------||
 *|------|--03--|------||
 *|--08--|------|--00--||
 *|--09--|--04--|------||
 */

void setup(){
  // define Arduino pins
  pinMode(SER_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);

  // reset all LEDs
  turnAllLeds(OFF);
}

void loop(){
}             

// turn led[i] OFF or ON
void turnLed(int i, bool state){
  led[i] = state;
  writeRegisters();
}

// turn all LEDs OFF or ON
void turnAllLeds(bool state){
  for(int i = NUM_LEDS; i > 0; i--){
    led[i] = state;
  }
  writeRegisters();
}

// write to 75HC595's registers from led[] array
void writeRegisters(){
  digitalWrite(RCLK_PIN, LOW);

  // only using NUM_LEDS of NUM_REGS pins
  // so populate remaining with 0 state
  for (int i = NUM_REGS - NUM_LEDS; i > 0; i--){
    digitalWrite(SRCLK_PIN, LOW);
    digitalWrite(SER_PIN, 0);
    digitalWrite(SRCLK_PIN, HIGH);   
  }

  // start backwards because of 'push' nature!
  for(int i = NUM_LEDS; i > 0; i--){
    digitalWrite(SRCLK_PIN, LOW);
    digitalWrite(SER_PIN, led[i]);
    digitalWrite(SRCLK_PIN, HIGH);
  }

  digitalWrite(RCLK_PIN, HIGH);
}