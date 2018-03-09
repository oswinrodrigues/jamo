const int CHORD_MODE = 0;
const int KEY_MODE = 1;

const int G_CHORD = 0;
const int C_CHORD = 1;
const int D_CHORD = 2;
const int F_CHORD = 3;
const int Am_CHORD = 4;
const int Em_CHORD = 5;
const int NUM_CHORDS = 6;

const int G_KEY = 0;
const int C_KEY = 1;
const int NUM_KEYS = 2;

const int NUM_CHORDS_IN_KEY = 4;
const int KEY_CHORDS[NUM_KEYS][NUM_CHORDS_IN_KEY] = {
  {G_CHORD, C_CHORD, D_CHORD, Em_CHORD}, // G_KEY
  {C_CHORD, F_CHORD, G_CHORD, Am_CHORD}  // C_KEY
};

const int POLL_DELAY = 100; // 0.1 seconds
const int PLAY_LIMIT = 10000; // 10 seconds
const int FEEDBACK_LIMIT = 3000; // 3 seconds

unsigned long time_start = 0;
bool back_pressed = false;
int mode = -1;
int chord = -1;
int key = -1;
int key_chord_counter = 0;
bool menu_pressed = true;
bool played_correctly = false;
int feedback_counter = 0;
String previous_feedback = "";

const int SCREEN_MENU_BACK = 1;
String SCREEN_FEEDBACK = "";

void setup(){
  // debug code
  Serial.begin(9600);

  // led setup
  ledSetup();
  // screen setup
  screenSetup();
  // fsr setup
  sensorSetup();

  // led initialize
  ledInitialize();
  // screen initialize
  screenInitialize();
  // fsr initialize
  // TODO: Remove delays!
  //sensorInitialize();
}

void loop(){
  // Serial.println("Enter main loop");
  if (menu_pressed) {
    // Serial.println("Enter prompt logic");
    if (!back_pressed){
      // Serial.println("Enter mode prompt logic");
      // mode prompt
      screenSelectMode();
      mode = -1;
      while (mode == -1){
        mode = screenGetMode();
        delay(POLL_DELAY);
      }
      // Serial.println("Got mode");
    }

    back_pressed = false;

    // if chord mode, chord prompt
    if (mode == CHORD_MODE){
      // Serial.println("Enter chord prompt logic");
      screenSelectChord();
      chord = -1;
      while (chord == -1){
        chord = screenGetChord();
        delay(POLL_DELAY);
      }
      // Serial.println("Got chord");
    }
    // else if key mode, key prompt
    else if (mode == KEY_MODE){
      // Serial.println("Enter key prompt logic");
      screenSelectKey();
      key = -1;
      while (key == -1){
        key = screenGetKey();
        delay(POLL_DELAY);
      }
      // Serial.println("Got key");
      chord = KEY_CHORDS[key][key_chord_counter];
    }
  }

  // main logic for chord-playing:
  // instruct how play is expected

  // Serial.print("Display chord screen "); Serial.println(chord);
  screenPlayChord(chord);
  // command chord to be played
  // TODO: WHY ON GOD'S GREEN EARTH IS THIS A PROBLEM?
  ledTurnOnChord(chord);

  // wait until play is complete
  // Serial.println("Reset all variables");
  time_start = millis();
  // Serial.print("time_start is "); Serial.println(time_start);
  played_correctly = false;
  menu_pressed = false;
  previous_feedback = "";
  feedback_counter = 0;

  // Serial.println("Entering main while loop");
  while (((millis() - time_start) <= (PLAY_LIMIT + feedback_counter * FEEDBACK_LIMIT)) && (!played_correctly) && (!menu_pressed)) {
    // collect sensor data about play
    // verify play correctness
    // TODO: Check why returning false positives
    played_correctly = getSensorFeedback(chord);
    if (SCREEN_FEEDBACK != previous_feedback) {
      // Serial.println("Enter feedback display logic");
      // give feedback about play
      screenGiveFeedback();
      previous_feedback = SCREEN_FEEDBACK;
      // Serial.println("Delay for feedback reading");
      delay(FEEDBACK_LIMIT);
      feedback_counter += 1;
      // Serial.println("Display chord again after feedback");
      screenPlayChord(chord);
    }
    menu_pressed = screenGetMenuPress();
  }
  // Serial.println("Exited main while loop");
  // Serial.print("Duration check is "); Serial.println(PLAY_LIMIT + feedback_counter * FEEDBACK_LIMIT);
  // Serial.print("Elapsed duration is "); Serial.println(millis() - time_start);
  // Serial.print("played_correctly is "); Serial.println(played_correctly);
  // Serial.print("menu_pressed is "); Serial.println(menu_pressed);

  // if play is unacceptable, loop back to same chord
  // else if play is good, next chord in sequence
  if (played_correctly) {
    // Serial.println("Enter played_correctly handling logic");
    if (mode == KEY_MODE) {
      // Serial.println("Since key mode, increment chord");
      key_chord_counter += 1 ;
      chord = KEY_CHORDS[key][key_chord_counter];
    }
  }

  else if (menu_pressed) {
    // Serial.println("Enter menu_press handling logic");
    screenSelectMenuOption();
    int menu_option = -1;
    while (menu_option == -1) {
     menu_option = screenGetMenuOption();
     delay(POLL_DELAY);
    }
    // Serial.println("Got menu option press");
    // if 'change chord/key' pressed, loop back to chord/key prompt
    if (menu_option == SCREEN_MENU_BACK) {
      // Serial.println("Back button in menu was pressed");
      back_pressed = true;
    }
    // else if 'change  mode' pressed, loop back to mode prompt
  }
}

/* * * * * * * * * * *
 * LED-related code  *
 * * * * * * * * * * */

const int LED_SER_PIN = 4;    // pin 14 on the 75HC595
const int LED_RCLK_PIN = 3;   // pin 12 on the 75HC595
const int LED_SRCLK_PIN = 2;  // pin 11 on the 75HC595

const bool LED_ON = 1;        // LED on-state indicator
const bool LED_OFF = 0;       // LED off-state indicator
const int LED_NUM_REGS = 16;  // total registers available
const int LED_NUM_LEDS = 10;  // number of LEDs on fretboard
bool led[LED_NUM_LEDS];       // total registers that are used

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

const int LED_MAX_LEDS_FOR_CHORD = 4;

// For following, consider using a struct instead that has the
// chord name, led indices and number of leds too

const int LED_CHORD_LED_MAPPING[NUM_CHORDS][LED_MAX_LEDS_FOR_CHORD] = {
  // Use -1 as flag to skip, since not a valid led[] index
  {1, 5, 9, -1}, // G_CHORD
  {0, 2, 6, -1}, // C_CHORD
  {3, 4, 8, -1}, // D_CHORD
  {0, 3, 6, 7},  // F_CHORD
  {0, 2, 3, -1}, // Am_CHORD
  {1, 2, -1, -1} // Em_CHORD
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
  for (int i = 0; i < LED_MAX_LEDS_FOR_CHORD; i++){
    if (LED_CHORD_LED_MAPPING[chord][i] != -1){
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

#include <stdint.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "TouchScreen.h"

// Display (output) pins
const int SCREEN_TFT_DC_PIN = 9;  // for the Adafruit shield, these are the default.
const int SCREEN_TFT_CS_PIN = 10; // for the Adafruit shield, these are the default.

// Touchscreen (input) pins
const int SCREEN_YP_PIN = A4;     // must be an analog pin, use "An" notation!
const int SCREEN_XM_PIN = A5;     // must be an analog pin, use "An" notation!
const int SCREEN_YM_PIN = 8;      // can be a digital pin
const int SCREEN_XP_PIN = SCREEN_TFT_DC_PIN; // same as DC pin, according to Adafruit

// If using hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 _screen_tft = Adafruit_ILI9341(SCREEN_TFT_CS_PIN, SCREEN_TFT_DC_PIN);

// For better pressure precision, need to know the resistance between X+ and
// X- Use multimeter; for ours, its 300 ohms across the X plate
const int SCREEN_TS_OHMS = 300;
TouchScreen _screen_ts = TouchScreen(SCREEN_XP_PIN, SCREEN_YP_PIN, SCREEN_XM_PIN, SCREEN_YM_PIN, SCREEN_TS_OHMS);

// Names for menu buttons
const int SCREEN_MENU_RESELECT = 0;
// const int SCREEN_MENU_BACK = 1;

// The following strings can be edited as required
// Instructions to player for calibrating FSRs
String SCREEN_FSR_CALIBRATION = "";
// Instructions to player when chord displayed
String SCREEN_INSTRUCTIONS = "";
// Feedback to player after chord is played
// String SCREEN_FEEDBACK = "";

void screenSetup(){
  _screen_tft.begin();
  // 1 or 3 for landscape mode
  _screen_tft.setRotation(3);
}

void screenInitialize(){
  screenCallibration();
}

void screenCallibration(){
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  _screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  _screen_tft.setCursor(w/8, 0);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("Callibrating");

  // Message
  _screen_tft.setCursor(w/8, h/2);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(2);
  _screen_tft.println(SCREEN_FSR_CALIBRATION);
}

void screenSelectMode(){
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  _screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  _screen_tft.setCursor(w/8, 0);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("Select Playing Mode");

  // Box 1: Chord
  // fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  _screen_tft.fillRoundRect((w/8), h/2, (3*w/8), (h/3), 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect((w/8), h/2, (3*w/8), (h/3), 10, ILI9341_WHITE);
  _screen_tft.setCursor((w/8+(3*w/8)/4), (h/2+(h/3)/2));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(2);
  _screen_tft.println("Chord");

  // Box 2: Key
  // fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  _screen_tft.fillRoundRect((w/2), h/2, (3*w/8), (h/3), 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect((w/2), h/2, (3*w/8), (h/3), 10, ILI9341_WHITE);
  _screen_tft.setCursor((w/2+(3*w/8)/4), (h/2+(h/3)/2));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(2);
  _screen_tft.println("Key");
}

int screenGetMode(){
  // Box Info
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  // Box 1: Chord
  int x_starting_spot_chord = (w/8);
  int y_starting_spot_chord = (h/2);
  // Box 2: Key
  int x_starting_spot_key = (w/2);
  int y_starting_spot_key = (h/2);
  // Box Dimensions
  int box_width = (3*w/8);
  int box_height = (h/3);

  // Get Point Instance
  TSPoint p = _screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Mode
  if (x > x_starting_spot_chord && x < x_starting_spot_chord + box_width && y > y_starting_spot_chord && y < y_starting_spot_chord + box_height && p.z > 0){
    return CHORD_MODE;
  }
  else if (x > x_starting_spot_key && x < x_starting_spot_key + box_width && y > y_starting_spot_key && y < y_starting_spot_key + box_height && p.z > 0){
    return KEY_MODE; 
  }

  return -1;
}

void screenSelectChord(){
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  _screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  _screen_tft.setCursor(w/8, 0);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("Choose to Play");

  // Box 1: C
  screenChordModeBoxes("C", 0);
  // Box 2: D
  screenChordModeBoxes("D", 1);
  // Box 3: Em
  screenChordModeBoxes("Em", 2);
  // Box 4: Am
  screenChordModeBoxes("Am", 3);
  // Box 5: F
  screenChordModeBoxes("F", 4);
  // Box 6: G
  screenChordModeBoxes("G", 5);
}

// Box spots = numbers 0 to 5
void screenChordModeBoxes(String chord_name, int box_spot){
  // Box Parameters
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  int x_starting_spot = (w/8)*((box_spot%3)*2+1);
  int y_starting_spot = (h/3);
  // if greater it is the 4th box put on second row
  if (box_spot > 2){
    y_starting_spot = 2*(h/3);
  }
  int box_width = w/4;
  int box_height = h/3;

  // Create Box
  _screen_tft.fillRoundRect(x_starting_spot, y_starting_spot, box_width, box_height, 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect(x_starting_spot, y_starting_spot, box_width, box_height, 10, ILI9341_WHITE);

  // Write Inside box (approx center)
  _screen_tft.setCursor((x_starting_spot+box_width/2), (y_starting_spot+box_height/2));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(2);
  _screen_tft.println(chord_name);
}

int screenGetChord(){
  int chord = -1;

  int box_selected = 6;
  // Box Parameters
  int box_spot = 0;
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  // Get Point Instance
  TSPoint p = _screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320-double(p.y)*double(w)/1024;
  
  for (box_spot = 0; box_spot < 6; box_spot++){
    int x_starting_spot = (w/8)*((box_spot%3)*2+1);
    int y_starting_spot = (h/3);
    // if greater it is the 4th box put on second row
    if (box_spot > 2){
      y_starting_spot = 2*(h/3);
    }
    int box_width = w/4;
    int box_height = h/3;

    // Determine Chord
    if (x > x_starting_spot && x < x_starting_spot + box_width && y > y_starting_spot && y < y_starting_spot + box_height && p.z > 0){
      box_selected = box_spot;
    }
  }

  switch(box_selected){
    case 0: chord = C_CHORD;  break;
    case 1: chord = D_CHORD;  break;
    case 2: chord = Em_CHORD; break;
    case 3: chord = Am_CHORD; break;
    case 4: chord = F_CHORD;  break;
    case 5: chord = G_CHORD;  break;
    default: chord = -1;      break;
  }

  return chord;
}

void screenSelectKey(){
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  _screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  _screen_tft.setCursor(w/8, 0);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("Choose to Play");

  // Box 1: Key of G
  // fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  _screen_tft.fillRoundRect((w/4), h/2, (w/4), (h/4), 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect((w/4), h/2, (w/4), (h/4), 10, ILI9341_WHITE);
  _screen_tft.setCursor((w/4+(w/4)/2), (h/2+h/16));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("G");

  // Box 2: Key of C
  // fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  _screen_tft.fillRoundRect((w/2), h/2, (w/4), (h/4), 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect((w/2), h/2, (w/4), (h/4), 10, ILI9341_WHITE);
  _screen_tft.setCursor((w/2+(w/4)/2), (h/2+h/16));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("C");
}

int screenGetKey(){
  // Box Info
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  // Box 1: Key G
  int x_starting_spot_gkey = (w/4);
  int y_starting_spot_gkey = (h/2);
  // Box 2: Key C
  int x_starting_spot_ckey = (w/2);
  int y_starting_spot_ckey = (h/2);
  // Box Dimensions
  int box_width = (w/4);
  int box_height = (h/4);

  // Get Point Instance
  TSPoint p = _screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Key
  if (x > x_starting_spot_gkey && x < x_starting_spot_gkey + box_width && y > y_starting_spot_gkey && y < y_starting_spot_gkey + box_height && p.z > 0){
    return G_KEY;
  }
  else if (x > x_starting_spot_ckey && x < x_starting_spot_ckey + box_width && y > y_starting_spot_ckey && y < y_starting_spot_ckey + box_height && p.z > 0){
    return C_KEY;
  }

  return -1;
}

void screenPlayChord(int chord){
  String chord_string = "";
  switch (chord){
    case G_CHORD: chord_string = "G";   break;
    case C_CHORD: chord_string = "C";   break;
    case D_CHORD: chord_string = "D";   break;
    case F_CHORD: chord_string = "F";   break;
    case Am_CHORD: chord_string = "Am"; break;
    case Em_CHORD: chord_string = "Em"; break;
    default: chord_string = "Unknown";  break;
  }

  int w = _screen_tft.width();
  int h = _screen_tft.height();
  _screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  _screen_tft.setCursor(w/2, 0);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(5);
  _screen_tft.println(chord_string);

  // Message
  _screen_tft.setCursor(w/8, h/2);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(2);
  _screen_tft.println(SCREEN_INSTRUCTIONS);

  // Menu Box
  _screen_tft.fillRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_WHITE);
  _screen_tft.setCursor((3*w/4+20), (h/16));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(2);
  _screen_tft.println("MENU");
}

bool screenGetMenuPress(){
  // Box Info
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  // Menu Box
  int x_starting_spot = 3*(w/4);
  int y_starting_spot = 0;
  // Box Dimensions
  int box_width = (w/4);
  int box_height = (h/3);

  // Get Point Instance
  TSPoint p = _screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Mode
  if (x > x_starting_spot && x < x_starting_spot + box_width && y > y_starting_spot && y < y_starting_spot + box_height && p.z > 0){
    return true;
  }

  return false;
}

void screenSelectMenuOption(){
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  _screen_tft.fillScreen(ILI9341_WHITE);
  
  // Re-Select Mode
  _screen_tft.fillRoundRect((w/4), 0, (w/2), (h/2), 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect((w/4), 0, (w/2), (h/2), 10, ILI9341_WHITE);
  _screen_tft.setCursor((w/4+(w/4)/8), (0+h/16));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("ReSelect");
  _screen_tft.setCursor((w/4+(w/4)/8), (2*h/16+15));
  _screen_tft.println("Mode");

  // Re-Choose What Is Chord (Chord Mode) Re-Choose Key (Key Mode)
  _screen_tft.fillRoundRect((w/4), h/2, (w/2), (h/2), 10, ILI9341_MAGENTA);
  _screen_tft.drawRoundRect((w/4), h/2, (w/2), (h/2), 10, ILI9341_WHITE);
  _screen_tft.setCursor((w/4+(w/2)/8), (h/2+h/8));
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("Back");
}

int screenGetMenuOption(){
  int option = -1;

  // Box Info
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  // Box 1: Re-Select Option
  int x_starting_spot_reselect = (w/4);
  int y_starting_spot_reselect = 0;
  // Box 2: Back (to prev choosing screen -> Chord or Key Mode Screens)
  int x_starting_spot_back = (w/4);
  int y_starting_spot_back = (h/2);
  // Box Dimensions
  int box_width = (w/2);
  int box_height = (h/2);

  // Get Point Instance
  TSPoint p = _screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Mode
  if (x > x_starting_spot_reselect && x < x_starting_spot_reselect + box_width && y > y_starting_spot_reselect && y < y_starting_spot_reselect + box_height && p.z > 0){
    option = SCREEN_MENU_RESELECT;
  }
  else if (x > x_starting_spot_back && x < x_starting_spot_back + box_width && y > y_starting_spot_back && y < y_starting_spot_back + box_height && p.z > 0){
    option = SCREEN_MENU_BACK;
  }

  return option;
}

void screenGiveFeedback(){
  int w = _screen_tft.width();
  int h = _screen_tft.height();
  _screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  _screen_tft.setCursor(w/8, 0);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(3);
  _screen_tft.println("Feedback");

  // Message
  _screen_tft.setCursor(w/8, h/2);
  _screen_tft.setTextColor(ILI9341_BLACK);  _screen_tft.setTextSize(2);
  _screen_tft.println(SCREEN_FEEDBACK);
}

/* * * * * * * * * * *
 * FSR-related code  *
 * * * * * * * * * * */

// Pins connected to softpot
const int SOFT_POT_PIN_1 = A0;
const int SOFT_POT_PIN_2 = A1;
const int SOFT_POT_PIN_3 = A2;

const int TOL = 55; // TODO: Verify this tolerance value

int calibratedSingleStringValues[] = {750, 300, 410, 600, 920, 200, 260, 360, 740, 900};
// Array corresponds to {Em, Am, D, G, F}
int calibratedDoubleStringValues[] = {360, 460, 660, 700, 375};

void sensorSetup()
{
  pinMode(SOFT_POT_PIN_1, INPUT);
  pinMode(SOFT_POT_PIN_2, INPUT);
  pinMode(SOFT_POT_PIN_3, INPUT);
}

void sensorInitialize()
{
  // Calibrate all single notes
  for(int i = 0; i < 10; i++)
  {
    bool success = false; 
    int ledNumber = i + 1;
    SCREEN_FSR_CALIBRATION = "Place your finger on LED #" + ledNumber;
    screenCallibration();
    // Turn on LED #i
    ledTurn(i, LED_ON);
    while(!success) {
      success = calibrateSingleString(i, true);
    }
    int time_on = millis();
    while((millis() - time_on) < 500) {
      success = calibrateSingleString(i, true);
    }
    // Turn off LED #i
    ledTurn(i, LED_OFF);
    SCREEN_FSR_CALIBRATION = "Take your finger off LED #" + ledNumber;
    screenCallibration();
    int time_off = millis();
    while(success || (millis() - time_off) < 1000) {
      success = calibrateSingleString(i, false);
    }
    // Serial.println(calibratedSingleStringValues[i]);
  }
  // Calibrate all necessary intervals
  int note1 = -1;
  int note2 = -1;
  String calibrateText = "Place your fingers on LEDs #";
  String textAdding = " and #";
  for(int i = 0; i < 5; i++)
  {
    bool success = false;
    if(i == 3)
    {
      note1 = 5;
      note2 = 9;
    }
    else if(i == 4)
    {
      note1 = 6;
      note2 = 7;
    }
    else
    {
      note1 = i + 1;
      note2 = i + 2;
    }
    SCREEN_FSR_CALIBRATION = calibrateText + note1 + textAdding + note2;
    screenCallibration();
    // Turn on LEDs #note1 and #note2
    ledTurn(note1, LED_ON);
    ledTurn(note2, LED_ON);
    while(!success) {
      success = calibrateDoubleString(note1, note2, true);
    }
    int time_on2 = millis();
    while((millis() - time_on2) < 1000) {
      success = calibrateSingleString(i, true);
    }
    // Turn off all LEDs
    ledTurnAll(LED_OFF);
    SCREEN_FSR_CALIBRATION = "Take your fingers off the LEDs";
    screenCallibration();
    int time_off2 = millis();
    while(success || (millis() - time_off2) < 1000) {
      success = calibrateDoubleString(note1, 0, false);
    }
    // Serial.println(calibratedDoubleStringValues[i]);
  }
}

bool calibrateSingleString(int noteNumber, bool record)
{
  int softPotADC;
  if(noteNumber == 0)
  {
    softPotADC = analogRead(SOFT_POT_PIN_1);
  }
  else if(noteNumber == 1 || noteNumber == 2 || noteNumber == 3 || noteNumber == 4)
  {
    softPotADC = analogRead(SOFT_POT_PIN_2);
  }
  else
  {
    softPotADC = analogRead(SOFT_POT_PIN_3);
  }
  if(softPotADC == 0)
  {
    return false;
  }
  if(record)
  {
    calibratedSingleStringValues[noteNumber] = softPotADC;
  }
  return true;
}

bool calibrateDoubleString(int noteNumber1, int noteNumber2, bool record)
{
  int softPotADC;
  
  if(noteNumber1 == 1 || noteNumber1 == 2 || noteNumber1 == 3)
  {
    softPotADC = analogRead(SOFT_POT_PIN_2);
    if(softPotADC == 0)
    {
      return false;
    }
    if(record)
    {
      if(noteNumber1 == 1 && noteNumber2 == 2)
      {
        calibratedDoubleStringValues[0] = softPotADC;
      }
      else if(noteNumber1 == 2 && noteNumber2 == 3)
      {
        calibratedDoubleStringValues[1] = softPotADC;
      }
      else if(noteNumber1 == 3 && noteNumber2 == 4)
      {
        calibratedDoubleStringValues[2] = softPotADC;
      }
    }
    return true;
  }
  else
  {
    softPotADC = analogRead(SOFT_POT_PIN_3);
    if(softPotADC == 0)
    {
      return false;
    }
    if(record)
    {
      if(noteNumber1 == 5 && noteNumber2 == 9)
      {
        calibratedDoubleStringValues[3] = softPotADC;
      }
      else if(noteNumber1 == 6 && noteNumber2 == 7)
      {
        calibratedDoubleStringValues[4] = softPotADC;
      }
    }
    return true;
  }
  return false;
}

bool getSensorFeedback(int expectedChord)
{
  int softPotADC[] = {
    analogRead(SOFT_POT_PIN_1), analogRead(SOFT_POT_PIN_2), analogRead(SOFT_POT_PIN_3)
  };

  int softPotADC1 = analogRead(SOFT_POT_PIN_1);
  int softPotADC2 = analogRead(SOFT_POT_PIN_2);
  int softPotADC3 = analogRead(SOFT_POT_PIN_3);
  // Serial.println(softPotADC3);
  delay(100);
  bool success = evaluateChord(expectedChord, softPotADC1, softPotADC2, softPotADC3);

  if(success)
  {
    SCREEN_FEEDBACK = "Good Job";
    // Serial.println("Good Job");
    return true;
  }
  else
  {
    findError(expectedChord, softPotADC1, softPotADC2, softPotADC3);
    return false;
  }
}

bool evaluateChord(int expectedChord, int sensorValue1, int sensorValue2, int sensorValue3)
{
  switch (expectedChord) {
    // G chord
    case G_CHORD:
      if (abs(sensorValue1) == 0 && abs(sensorValue2 - calibratedSingleStringValues[1]) < TOL && abs(sensorValue3 - calibratedDoubleStringValues[3]) < TOL)
        return true;
      return false;
    // C chord
    case C_CHORD:
      // Serial.println(sensorValue3);
      if (abs(sensorValue1 - calibratedSingleStringValues[0]) < TOL && abs(sensorValue2 - calibratedSingleStringValues[2]) < TOL && abs(sensorValue3 - calibratedSingleStringValues[6]) < TOL)
        return true;
      return false;
    // D chord
    case D_CHORD:
      if (abs(sensorValue1) == 0 && abs(sensorValue2 - calibratedDoubleStringValues[2]) < TOL && abs(sensorValue3 - calibratedSingleStringValues[8]) < TOL)
        return true;
      return false;
    // F chord
    case F_CHORD:
      if (abs(sensorValue1 - calibratedSingleStringValues[0]) < TOL && abs(sensorValue2 - calibratedSingleStringValues[3]) < TOL && abs(sensorValue3 - calibratedDoubleStringValues[4]) < TOL)
        return true;
      return false;
    // Am Chord
    case Am_CHORD:
      if (abs(sensorValue1 - calibratedSingleStringValues[0]) < TOL && abs(sensorValue2 - calibratedDoubleStringValues[1]) < TOL && abs(sensorValue3) == 0)
        return true;
      return false;
    // Em chord
    case Em_CHORD:
      if (abs(sensorValue1) == 0 && abs(sensorValue2 - calibratedDoubleStringValues[0]) < TOL && abs(sensorValue3) == 0)
        return true;
      return false;
    default:
      return false;
  }
}

void findError(int expectedChord, int sensorValue1, int sensorValue2, int sensorValue3)
{
  // Go through every case for each possible chord
  // Three cases:
  // 1. User has not placed fingers on any strings
  // 2. User has placed fingers on some of the correct notes
  // 3. User has placed fingers on incorrect notes
  SCREEN_FEEDBACK = "";
  switch(expectedChord) {
    case G_CHORD:
      if(abs(sensorValue1) != 0)
      {
        SCREEN_FEEDBACK = "Check the first fret";
      }
      if(abs(sensorValue2 - calibratedSingleStringValues[1]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the second fret";
      }
      if(abs(sensorValue3 - calibratedDoubleStringValues[3]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the third fret";
      }
      break;
    // C chord
    case C_CHORD:
      if(abs(sensorValue1 - calibratedSingleStringValues[0]) > TOL)
      {
        SCREEN_FEEDBACK = "Check the first fret";
      }
      if(abs(sensorValue2 - calibratedSingleStringValues[2]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the second fret";
      }
      if(abs(sensorValue3 - calibratedSingleStringValues[6]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the third fret";
        // Serial.println(softPotString_3[1]);
        // Serial.println(sensorValue3);
      }
      break;
    // D chord
    case D_CHORD:
      if(abs(sensorValue1) != 0)
      {
        SCREEN_FEEDBACK = "Check the first fret";
      }
      if(abs(sensorValue2 - calibratedDoubleStringValues[2]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the second fret";
      }
      if(abs(sensorValue3 - calibratedSingleStringValues[8]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the third fret";
      }
      break;
    // F chord
    case F_CHORD:
      if(abs(sensorValue1 - calibratedSingleStringValues[0]) > TOL)
      {
        SCREEN_FEEDBACK = "Check the first fret";
      }
      if(abs(sensorValue2 - calibratedSingleStringValues[3]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the second fret";
      }
      if(abs(sensorValue3 - calibratedDoubleStringValues[4]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the third fret";
      }
      break;
    // Am Chord
    case Am_CHORD:
      if(abs(sensorValue1 - calibratedSingleStringValues[0]) > TOL)
      {
        SCREEN_FEEDBACK = "Check the first fret";
      }
      if(abs(sensorValue2 - calibratedDoubleStringValues[1]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the second fret";
      }
      if(abs(sensorValue3) != 0)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the third fret";
      }
      break;
    // Em chord
    case Em_CHORD:
      if(abs(sensorValue1) != 0)
      {
        SCREEN_FEEDBACK = "Check the first fret";
      }
      if(abs(sensorValue2 - calibratedDoubleStringValues[0]) > TOL)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the second fret";
      }
      if(abs(sensorValue3) != 0)
      {
        SCREEN_FEEDBACK = SCREEN_FEEDBACK + "Check the third fret";
      }
      break;
  }
}
