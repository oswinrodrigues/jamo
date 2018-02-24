int current_chord = -1;

void setup(){
	// led setup
	ledSetup();
	// screen setup
	// fsr setup

	// led initialize
	ledInitialize();
	// screen initialize
	// fsr initialize
}

void loop(){
	// mode prompt
	// if chord mode, chord prompt
	// set chord 'sequence' i.e. same chord
	// else if key mode, key prompt
	// set chord sequence for given key

	// main logic for chord-playing:
	// instruct how play is expected
	// command chord to be played
	ledTurnOnChord(current_chord);
	// wait until play is complete
	// collect sensor data about play
	// verify play correctness
	// give feedback about play
	// if play is unacceptable, loop back to same chord
	// else if play is good, next chord in sequence

	// all the while:
	// if 'change chord/key' pressed, loop back to chord/key prompt
	// else if 'change mode' pressed, loop back to mode prompt
}

/* * * * * * * * * * *
 * LED-related code  *
 * * * * * * * * * * */

const int LED_SER_PIN = 4;    // pin 14 on the 75HC595
const int LED_RCLK_PIN = 3;   // pin 12 on the 75HC595
const int LED_SRCLK_PIN = 2;  // pin 11 on the 75HC595

const bool LED_ON = 1;          // LED on-state indicator
const bool LED_OFF = 0;         // LED off-state indicator
const int LED_NUM_REGS = 16;    // total registers available
const int LED_NUM_LEDS = 10;    // number of LEDs on fretboard
bool led[LED_NUM_LEDS];         // total registers that are used

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
 * [1, 5, 9]    for LED_G_CHORD
 * [0, 2, 6]    for LED_C_CHORD
 * [3, 4, 8]    for LED_D_CHORD
 * [0, 3, 6, 7] for LED_F_CHORD
 * [0, 2, 3]    for LED_Am_CHORD
 * [1, 2]       for LED_Em_CHORD
 */

const int LED_G_CHORD = 0;
const int LED_C_CHORD = 1;
const int LED_D_CHORD = 2;
const int LED_F_CHORD = 3;
const int LED_Am_CHORD = 4;
const int LED_Em_CHORD = 5;
const int LED_NUM_CHORDS = 6;
const int LED_MAX_LEDS_FOR_CHORD = 4;

// For following, consider using a struct instead that has the
// chord name, led indices and number of leds too

const int LED_CHORD_LED_MAPPING[LED_NUM_CHORDS][LED_MAX_LEDS_FOR_CHORD] = {
  // Use -1 as flag to skip, since not a valid led[] index
  {1, 5, 9, -1}, // LED_G_CHORD
  {0, 2, 6, -1}, // LED_C_CHORD
  {3, 4, 8, -1}, // LED_D_CHORD
  {0, 3, 6, 7},  // LED_F_CHORD
  {0, 2, 3, -1}, // LED_Am_CHORD
  {1, 2, -1, -1} // LED_Em_CHORD
};

// Array for comparison for debug purposes only
const bool LED_IDEAL[LED_NUM_CHORDS][LED_NUM_LEDS] = {
// {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
   {0, 1, 0, 0, 0, 1, 0, 0, 0, 1}, // LED_G_CHORD
   {1, 0, 1, 0, 0, 0, 1, 0, 0, 0}, // LED_C_CHORD
   {0, 0, 0, 1, 1, 0, 0, 0, 1, 0}, // LED_D_CHORD
   {1, 0, 0, 1, 0, 0, 1, 1, 0, 0}, // LED_F_CHORD
   {1, 0, 1, 1, 0, 0, 0, 0, 0, 0}, // LED_Am_CHORD
   {0, 1, 1, 0, 0, 0, 0, 0, 0, 0}  // LED_Em_CHORD
};

void ledSetup(){
  // define Arduino pins
  pinMode(LED_SER_PIN, OUTPUT);
  pinMode(LED_RCLK_PIN, OUTPUT);
  pinMode(LED_SRCLK_PIN, OUTPUT);
}

void ledInitialize(){
  // reset all LEDs
  ledTurnAll(LED_OFF);
}

// turn led[i] LED_OFF or LED_ON
void ledTurn(int i, bool state){
  led[i] = state;
  ledWriteRegisters();
}

// turn all LEDs LED_OFF or LED_ON
void ledTurnAll(bool state){
  ledSetAll(state);
  ledWriteRegisters();
}

// set, don't write, all LEDs' state
void ledSetAll(bool state){
  for(int i = LED_NUM_LEDS-1; i >= 0; i--){
    led[i] = state;
  }
}

// turn chord ON
void ledTurnOnChord(int chord){
  ledTurnAll(LED_OFF);
  for (int i = LED_MAX_LEDS_FOR_CHORD-1; i >= 0; i--){
    if (led[LED_CHORD_LED_MAPPING[chord][i]] != -1) {
      led[LED_CHORD_LED_MAPPING[chord][i]] = 1;
    }
  }
  ledWriteRegisters();
}

// write to 75HC595's registers from led[] array
void ledWriteRegisters(){
  digitalWrite(LED_RCLK_PIN, LOW);

  // only using first LED_NUM_LEDS of LED_NUM_REGS pins
  // so populate remaining with 0 state
  for (int i = LED_NUM_REGS - LED_NUM_LEDS; i > 0; i--){
    digitalWrite(LED_SRCLK_PIN, LOW);
    digitalWrite(LED_SER_PIN, 0);
    digitalWrite(LED_SRCLK_PIN, HIGH);   
  }

  // start backwards because of 'push' nature!
  for(int i = LED_NUM_LEDS-1; i >= 0; i--){
    digitalWrite(LED_SRCLK_PIN, LOW);
    digitalWrite(LED_SER_PIN, led[i]);
    digitalWrite(LED_SRCLK_PIN, HIGH);
  }

  digitalWrite(LED_RCLK_PIN, HIGH);
}

/* * * * * * * * * * * *
 * Screen-related code *
 * * * * * * * * * * * */

/* * * * * * * * * * *
 * FSR-related code  *
 * * * * * * * * * * */