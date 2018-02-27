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
    if (led[LED_CHORD_LED_MAPPING[chord][i]] != -1){
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
#define SCREEN_TFT_DC 9  // for the Adafruit shield, these are the default.
#define SCREEN_TFT_CS 10 // for the Adafruit shield, these are the default.
// Touchscreen (input) pins
#define SCREEN_YP A2     // must be an analog pin, use "An" notation!
#define SCREEN_XM A3     // must be an analog pin, use "An" notation!
#define SCREEN_YM 8      // can be a digital pin
#define SCREEN_XP 9      // can be a digital pin

// If using hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 screen_tft = Adafruit_ILI9341(SCREEN_TFT_CS, SCREEN_TFT_DC);

// For better pressure precision, need to know the resistance between X+ and
// X- Use multimeter; for ours, its 300 ohms across the X plate
TouchScreen screen_ts = TouchScreen(SCREEN_XP, SCREEN_YP, SCREEN_XM, SCREEN_YM, 300);

void screnSetup() {
  screen_tft.begin();
  screen_tft.setRotation(3);
}

void loop() {
  TSPoint p = screen_ts.getPoint();
  int w = screen_tft.width();
  int h = screen_tft.height();

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
//  if (p.z > screen_ts.pressureThreshhold) {
//     Serial.print("X = "); Serial.print(double(p.x)*double(w)/1024);
//     Serial.print("\tY = "); Serial.print(double(p.y)*double(h)/1024);
//     Serial.print("\tPressure = "); Serial.println(p.z);
//  }
}

void screenCallibration(String str) {
  int w = screen_tft.width();
  int h = screen_tft.height();
  screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  screen_tft.setCursor(w/8, 0);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("Callibrating");

  // Message
  screen_tft.setCursor(w/8, h/2);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(2);
  screen_tft.println(str);
}

void screenSelectMode() {
  int w = screen_tft.width();
  int h = screen_tft.height();
  screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  screen_tft.setCursor(w/8, 0);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("Select Playing Mode");

  // Box 1: Chord
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  screen_tft.fillRoundRect((w/8), h/2, (3*w/8), (h/3), 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect((w/8), h/2, (3*w/8), (h/3), 10, ILI9341_WHITE);
  screen_tft.setCursor((w/8+(3*w/8)/4), (h/2+(h/3)/2));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(2);
  screen_tft.println("Chord");

  // Box 2: Key
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  screen_tft.fillRoundRect((w/2), h/2, (3*w/8), (h/3), 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect((w/2), h/2, (3*w/8), (h/3), 10, ILI9341_WHITE);
  screen_tft.setCursor((w/2+(3*w/8)/4), (h/2+(h/3)/2));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(2);
  screen_tft.println("Key");
}

String screenGetMode() {
  String mode;
  // Box Info
  int w = screen_tft.width();
  int h = screen_tft.height();
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
  TSPoint p = screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Mode
  if (x > x_starting_spot_chord && x < x_starting_spot_chord + box_width && y > y_starting_spot_chord && y < y_starting_spot_chord + box_height && p.z > 0) {
    mode = "Chord";
  }
  else if (x > x_starting_spot_key && x < x_starting_spot_key + box_width && y > y_starting_spot_key && y < y_starting_spot_key + box_height && p.z > 0) {
    mode = "Key";
  }
  else {
    mode = "";
  }

  return mode;
}

void screenSelectChord() {
  int w = screen_tft.width();
  int h = screen_tft.height();
  screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  screen_tft.setCursor(w/8, 0);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("Choose to Play");

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
void screenChordModeBoxes(String chord_name, int box_spot) {
  // Box Parameters
  int w = screen_tft.width();
  int h = screen_tft.height();
  int x_starting_spot = (w/8)*((box_spot%3)*2+1);
  int y_starting_spot = (h/3);
  // if greater it is the 4th box put on second row
  if (box_spot > 2) {
    y_starting_spot = 2*(h/3);
  }
  int box_width = w/4;
  int box_height = h/3;

  // Create Box
  screen_tft.fillRoundRect(x_starting_spot, y_starting_spot, box_width, box_height, 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect(x_starting_spot, y_starting_spot, box_width, box_height, 10, ILI9341_WHITE);

  // Write Inside box (approx center)
  screen_tft.setCursor((x_starting_spot+box_width/2), (y_starting_spot+box_height/2));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(2);
  screen_tft.println(chord_name);
}

String screenGetChord() {
  String chord;
  int box_selected = 6;
  // Box Parameters
  int box_spot = 0;
  int w = screen_tft.width();
  int h = screen_tft.height();
  // Get Point Instance
  TSPoint p = screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320-double(p.y)*double(w)/1024;
  
  for (box_spot = 0; box_spot < 6; box_spot++) {
    int x_starting_spot = (w/8)*((box_spot%3)*2+1);
    int y_starting_spot = (h/3);
    // if greater it is the 4th box put on second row
    if (box_spot > 2) {
      y_starting_spot = 2*(h/3);
    }
    int box_width = w/4;
    int box_height = h/3;

    // Determine Chord
    if (x > x_starting_spot && x < x_starting_spot + box_width && y > y_starting_spot && y < y_starting_spot + box_height && p.z > 0) {
      box_selected = box_spot;
    }
  }

  switch(box_selected) {
    case 0: chord = "C"; break;
    case 1: chord = "D"; break;
    case 2: chord = "Em"; break;
    case 3: chord = "Am"; break;
    case 4: chord = "F"; break;
    case 5: chord = "G"; break;
    default: chord = ""; break;
  }
  return chord;
}

void screenSelectKey() {
  int w = screen_tft.width();
  int h = screen_tft.height();
  screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  screen_tft.setCursor(w/8, 0);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("Choose to Play");

  // Box 1: Key of G
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  screen_tft.fillRoundRect((w/4), h/2, (w/4), (h/4), 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect((w/4), h/2, (w/4), (h/4), 10, ILI9341_WHITE);
  screen_tft.setCursor((w/4+(w/4)/2), (h/2+h/16));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("G");

  // Box 2: Key of C
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  screen_tft.fillRoundRect((w/2), h/2, (w/4), (h/4), 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect((w/2), h/2, (w/4), (h/4), 10, ILI9341_WHITE);
  screen_tft.setCursor((w/2+(w/4)/2), (h/2+h/16));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("C");
}

String screenGetKey() {
  String mode;
  // Box Info
  int w = screen_tft.width();
  int h = screen_tft.height();
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
  TSPoint p = screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Mode
  if (x > x_starting_spot_gkey && x < x_starting_spot_gkey + box_width && y > y_starting_spot_gkey && y < y_starting_spot_gkey + box_height && p.z > 0) {
    mode = "G";
  }
  else if (x > x_starting_spot_ckey && x < x_starting_spot_ckey + box_width && y > y_starting_spot_ckey && y < y_starting_spot_ckey + box_height && p.z > 0) {
    mode = "C";
  }
  else {
    mode = "";
  }

  return mode;
}

void screenPlayChord(String chord, String instructions) {
  int w = screen_tft.width();
  int h = screen_tft.height();
  screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  screen_tft.setCursor(w/2, 0);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(5);
  screen_tft.println(chord);

  // Message
  screen_tft.setCursor(w/8, h/2);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(2);
  screen_tft.println(instructions);

  // Menu Box
  screen_tft.fillRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_WHITE);
  screen_tft.setCursor((3*w/4+20), (h/16));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(2);
  screen_tft.println("MENU");
}

String screenGetMenuPress() {
  String button;
  // Box Info
  int w = screen_tft.width();
  int h = screen_tft.height();
  // Menu Box
  int x_starting_spot = 3*(w/4);
  int y_starting_spot = 0;
  // Box Dimensions
  int box_width = (w/4);
  int box_height = (h/3);

  // Get Point Instance
  TSPoint p = screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Mode
  if (x > x_starting_spot && x < x_starting_spot + box_width && y > y_starting_spot && y < y_starting_spot + box_height && p.z > 0) {
    button = "Menu";
  }
  else {
    button = "";
  }

  return button;
}

void screenSelectMenuOption() {
  int w = screen_tft.width();
  int h = screen_tft.height();
  screen_tft.fillScreen(ILI9341_WHITE);
  
  // Re-Select Mode
  screen_tft.fillRoundRect((w/4), 0, (w/2), (h/2), 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect((w/4), 0, (w/2), (h/2), 10, ILI9341_WHITE);
  screen_tft.setCursor((w/4+(w/4)/8), (0+h/16));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("ReSelect");
  screen_tft.setCursor((w/4+(w/4)/8), (2*h/16+15));
  screen_tft.println("Mode");

  // Re-Choose What Is Chord (Chord Mode) Re-Choose Key (Key Mode)
  screen_tft.fillRoundRect((w/4), h/2, (w/2), (h/2), 10, ILI9341_MAGENTA);
  screen_tft.drawRoundRect((w/4), h/2, (w/2), (h/2), 10, ILI9341_WHITE);
  screen_tft.setCursor((w/4+(w/2)/8), (h/2+h/8));
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("Back");
}

String screenGetMenuOption() {
  String option;
  // Box Info
  int w = screen_tft.width();
  int h = screen_tft.height();
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
  TSPoint p = screen_ts.getPoint();
  // Rotation and Conversion and Calibration
  double y = double(p.x)*double(h)/1024;
  double x = 320 - double(p.y)*double(w)/1024;
  
  // Determine Selected Mode
  if (x > x_starting_spot_reselect && x < x_starting_spot_reselect + box_width && y > y_starting_spot_reselect && y < y_starting_spot_reselect + box_height && p.z > 0) {
    option = "ReSelect";
  }
  else if (x > x_starting_spot_back && x < x_starting_spot_back + box_width && y > y_starting_spot_back && y < y_starting_spot_back + box_height && p.z > 0) {
    option = "Back";
  }
  else {
    option = "";
  }

  return option;
}

void screenGiveFeedback(String str) {
  int w = screen_tft.width();
  int h = screen_tft.height();
  screen_tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  screen_tft.setCursor(w/8, 0);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(3);
  screen_tft.println("Feedback");

  // Message
  screen_tft.setCursor(w/8, h/2);
  screen_tft.setTextColor(ILI9341_BLACK);  screen_tft.setTextSize(2);
  screen_tft.println(str);
}



/* * * * * * * * * * *
 * FSR-related code  *
 * * * * * * * * * * */