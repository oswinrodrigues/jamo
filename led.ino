const int SER_PIN = 2;    // pin 14 on the 75HC595
const int RCLK_PIN = 3;   // pin 12 on the 75HC595
const int SRCLK_PIN = 4;  // pin 11 on the 75HC595

const int NUM_REGS = 16;  // total registers available
const int NUM_LEDS = 12;  // number of LEDs on fretboard
bool led[NUM_LEDS];       // total registers that are used 

/* 
 * The diagram below shows how LED
 * array indices are referenced
 * ---------------------
 *      Fret Number
 *   03     02     01
 * ---------------------
 *|--07--|------|------||
 *|--08--|--02--|------||
 *|--09--|--03--|------||
 *|--10--|--04--|--00--||
 *|--11--|--05--|--01--||
 *|------|--06--|------||
 */

void setup(){
  // define Arduino pins
  pinMode(SER_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);

  // reset all LEDs
  turnAllOff();
}

void loop(){
}             

// turn all LEDs off
void turnAllOff(){
  setAllRegisters(0);
  writeRegisters();
}

// turn all LEDs on
void turnAllOn(){
  setAllRegisters(1);
  writeRegisters();
}

// set all register pins to 'value'
void setAllRegisters(int value){
  for(int i = NUM_LEDS; i > 0; i--){
    led[i] = value;
  }
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