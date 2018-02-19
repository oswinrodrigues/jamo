const int SER_PIN = 2;    // pin 14 on the 75HC595
const int RCLK_PIN = 3;   // pin 12 on the 75HC595
const int SRCLK_PIN = 4;  // pin 11 on the 75HC595

const bool ON = 1;          // LED on-state indicator
const bool OFF = 0;         // LED off-state indicator
const int NUM_REGS = 16;    // total registers available
const int NUM_LEDS = 10;    // number of LEDs on fretboard
bool led[NUM_LEDS];         // total registers that are used

/* 
 * The diagram below shows how led[]
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
 *
 * [1, 5, 9]    for G_CHORD
 * [0, 2, 6]    for C_CHORD
 * [3, 4, 8]    for D_CHORD
 * [0, 3, 6, 7] for F_CHORD
 * [0, 2, 3]    for Am_CHORD
 * [1, 2]       for Em_CHORD
 */

const int G_CHORD = 0;
const int C_CHORD = 1;
const int D_CHORD = 2;
const int F_CHORD = 3;
const int Am_CHORD = 4;
const int Em_CHORD = 5;
const int NUM_CHORDS = 6;
const int MAX_LEDS_FOR_CHORD = 4;

// For following, consider using a struct instead that has the
// chord name, led indices and number of leds too

const int CHORD_LED_MAPPING[NUM_CHORDS][MAX_LEDS_FOR_CHORD] = {
  // Use -1 as flag to skip, since not a valid led[] index
  {1, 5, 9, -1}, // G_CHORD
  {0, 2, 6, -1}, // C_CHORD
  {3, 4, 8, -1}, // D_CHORD
  {0, 3, 6, 7},  // F_CHORD
  {0, 2, 3, -1}, // Am_CHORD
  {1, 2, -1, -1} // Em_CHORD
};

// Array for comparison for debug purposes only
const bool LED_IDEAL[NUM_CHORDS][NUM_LEDS] = {
// {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
   {0, 1, 0, 0, 0, 1, 0, 0, 0, 1}, // G_CHORD
   {1, 0, 1, 0, 0, 0, 1, 0, 0, 0}, // C_CHORD
   {0, 0, 0, 1, 1, 0, 0, 0, 1, 0}, // D_CHORD
   {1, 0, 0, 1, 0, 0, 1, 1, 0, 0}, // F_CHORD
   {1, 0, 1, 1, 0, 0, 0, 0, 0, 0}, // Am_CHORD
   {0, 1, 1, 0, 0, 0, 0, 0, 0, 0}  // Em_CHORD
};

void setup(){
  // define Arduino pins
  pinMode(SER_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);

  // reset all LEDs
  turnAllLeds(OFF);

  // initialize console for print statements
  Serial.begin(9600);
}

void loop(){
  for (int chord = G_CHORD; chord < NUM_CHORDS; chord++){
    turnOnChord(chord);
    debug_chord(chord);
    Serial.println("Chord done, next one...");
    delay(500);
  }
  delay(10000);
}

// turn led[i] OFF or ON
void turnLed(int i, bool state){
  led[i] = state;
  writeRegisters();
}

// turn all LEDs OFF or ON
void turnAllLeds(bool state){
  setAllLeds(state);
  writeRegisters();
}

// set, don't write, all LEDs' state
void setAllLeds(bool state){
  for(int i = NUM_LEDS-1; i >= 0; i--){
    led[i] = state;
  }
}

// turn chord ON
void turnOnChord(int chord){
  setAllLeds(OFF);
  for (int i = MAX_LEDS_FOR_CHORD-1; i >= 0; i--){
    if (led[CHORD_LED_MAPPING[chord][i]] != -1) {
      led[CHORD_LED_MAPPING[chord][i]] = 1;
    }
  }
  writeRegisters();
}

// print debug statements for correct chord setting
void debug_chord(int chord){
  for (int i = 0; i < NUM_LEDS; i++){
    if (led[i] != LED_IDEAL[chord][i]){
      Serial.print("Error! Chord ");
      Serial.print(chord);
      Serial.print(", led index ");
      Serial.print(i);
      Serial.print(", led[i] is ");
      Serial.print(led[i]);
      Serial.print(" and LED_IDEAL[chord][i] is ");
      Serial.println(LED_IDEAL[chord][i]);
    }
  }
}

// write to 75HC595's registers from led[] array
void writeRegisters(){
  digitalWrite(RCLK_PIN, LOW);

  // only using first NUM_LEDS of NUM_REGS pins
  // so populate remaining with 0 state
  for (int i = NUM_REGS - NUM_LEDS; i > 0; i--){
    digitalWrite(SRCLK_PIN, LOW);
    digitalWrite(SER_PIN, 0);
    digitalWrite(SRCLK_PIN, HIGH);   
  }

  // start backwards because of 'push' nature!
  for(int i = NUM_LEDS-1; i >= 0; i--){
    digitalWrite(SRCLK_PIN, LOW);
    digitalWrite(SER_PIN, led[i]);
    digitalWrite(SRCLK_PIN, HIGH);
  }

  digitalWrite(RCLK_PIN, HIGH);
}