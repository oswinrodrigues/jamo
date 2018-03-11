#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

#include <stdint.h>
#include "TouchScreen.h"

//Old Setup
//#define YP A2  // must be an analog pin, use "An" notation!
//#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

//Actual Setup
#define YP A4  // must be an analog pin, use "An" notation!
#define XM A5  // must be an analog pin, use "An" notation!

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Display G&G Test!"); 
  
  tft.begin();
  tft.setRotation(3);
  //sendText("Null");

  delay(500);
  callibrationScreen("Please place your finger where the blinking LED indicates");

  delay(1000);
  selectModeScreen();
  
//  delay(1000);
//  chordModeScreen();
  
//  delay(1000);
//  keyModeScreen();
//
//  delay(1000);
//  playScreen("C", "Place fingers where LEDs indicate");
  delay(1000);
  playScreenV2(1, true, true, false);
//
//  delay(1000);
//  menuScreen();
}

void loop() {
  // put your main code here, to run repeatedly:
  TSPoint p = ts.getPoint();
  int w = tft.width();
  int h = tft.height();

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
//  if (p.z > ts.pressureThreshhold) {
//     Serial.print("X = "); Serial.print(double(p.x)*double(w)/1024);
//     Serial.print("\tY = "); Serial.print(double(p.y)*double(h)/1024);
//     Serial.print("\tPressure = "); Serial.println(p.z);
//  }


  String pls = getModeSelect();
  //String pls = getChord();
  //String pls = getKey();
  //String pls = getPlayScreenMenu();
  //String pls = getMenuOption();
  Serial.print(pls);
  if (pls != "") {
     Serial.print("Y = "); Serial.print(double(p.x)*double(h)/1024);
     Serial.print("\tX = "); Serial.print(320-double(p.y)*double(w)/1024);
     Serial.print("\tPressure = "); Serial.println(p.z);
  }
  
  delay(100);
}

void introductionScreen(String str) {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/8, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("Introduction");

  // Message
  tft.setCursor(w/8, h/2);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println(str);
}

void callibrationScreen(String str) {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/8, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("Callibrating");

  // Message
  tft.setCursor(w/8, h/2);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println(str);
}

void selectModeScreen() {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/8, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("Select Playing Mode");

  // Box 1: Chord
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  tft.fillRoundRect((w/8), h/2, (3*w/8), (h/3), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((w/8), h/2, (3*w/8), (h/3), 10, ILI9341_WHITE);
  tft.setCursor((w/8+(3*w/8)/4), (h/2+(h/3)/2));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println("Chord");

  // Box 2: Key
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  tft.fillRoundRect((w/2), h/2, (3*w/8), (h/3), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((w/2), h/2, (3*w/8), (h/3), 10, ILI9341_WHITE);
  tft.setCursor((w/2+(3*w/8)/4), (h/2+(h/3)/2));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println("Key");
}

String getModeSelect() {
  String mode;
  // Box Info
  int w = tft.width();
  int h = tft.height();
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
  TSPoint p = ts.getPoint();
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

void chordModeScreen() {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/8, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("Choose to Play");

  // Box 1: C
  chordModeScreenBoxes("C", 0);
  // Box 2: D
  chordModeScreenBoxes("D", 1);
  // Box 3: Em
  chordModeScreenBoxes("Em", 2);
  // Box 4: Am
  chordModeScreenBoxes("Am", 3);
  // Box 5: F
  chordModeScreenBoxes("F", 4);
  // Box 6: G
  chordModeScreenBoxes("G", 5);
}

// Box spots = numbers 0 to 5
void chordModeScreenBoxes(String chord_name, int box_spot) {
  // Box Parameters
  int w = tft.width();
  int h = tft.height();
  int x_starting_spot = (w/8)*((box_spot%3)*2+1);
  int y_starting_spot = (h/3);
  // if greater it is the 4th box put on second row
  if (box_spot > 2) {
    y_starting_spot = 2*(h/3);
  }
  int box_width = w/4;
  int box_height = h/3;

  // Create Box
  tft.fillRoundRect(x_starting_spot, y_starting_spot, box_width, box_height, 10, ILI9341_MAGENTA);
  tft.drawRoundRect(x_starting_spot, y_starting_spot, box_width, box_height, 10, ILI9341_WHITE);

  // Write Inside box (approx center)
  tft.setCursor((x_starting_spot+box_width/2), (y_starting_spot+box_height/2));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println(chord_name);
}

String getChord() {
  String chord;
  int box_selected = 6;
  // Box Parameters
  int box_spot = 0;
  int w = tft.width();
  int h = tft.height();
  // Get Point Instance
  TSPoint p = ts.getPoint();
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

void keyModeScreen() {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/8, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("Choose to Play");

  // Box 1: Key of G
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  tft.fillRoundRect((w/4), h/2, (w/4), (h/4), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((w/4), h/2, (w/4), (h/4), 10, ILI9341_WHITE);
  tft.setCursor((w/4+(w/4)/2), (h/2+h/16));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("G");

  // Box 2: Key of C
  //fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t ILI9341_MAGENTA);
  tft.fillRoundRect((w/2), h/2, (w/4), (h/4), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((w/2), h/2, (w/4), (h/4), 10, ILI9341_WHITE);
  tft.setCursor((w/2+(w/4)/2), (h/2+h/16));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("C");
}

String getKey() {
  String mode;
  // Box Info
  int w = tft.width();
  int h = tft.height();
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
  TSPoint p = ts.getPoint();
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

String getPlayScreenMenu() {
  String button;
  // Box Info
  int w = tft.width();
  int h = tft.height();
  // Menu Box
  int x_starting_spot = 3*(w/4);
  int y_starting_spot = 0;
  // Box Dimensions
  int box_width = (w/4);
  int box_height = (h/3);

  // Get Point Instance
  TSPoint p = ts.getPoint();
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

void playScreen(String chord, String instructions) {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/2, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(5);
  tft.println(chord);

  // Message
  tft.setCursor(w/8, h/2);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println(instructions);

  // Menu Box
  tft.fillRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_WHITE);
  tft.setCursor((3*w/4+20), (h/16));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println("MENU");
}

void playScreenV2(int chord, bool f1, bool f2, bool f3) {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/2, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(5);
  tft.println(chord);

  // Fret Boxes
  bool fret_feedback[3] = {f1, f2, f3}; 
  for (int i = 2; i >= 0; i--)
  {
    if (fret_feedback[i] == true) {
      tft.fillRoundRect(((w/8)+3*i*w/16), (h/3), (3*w/16), (h/2), 2, ILI9341_GREEN);
    } else {
      tft.fillRoundRect(((w/8)+3*i*w/16), (h/3), (3*w/16), (h/2), 2, ILI9341_RED);
    }
    tft.drawRoundRect(((w/8)+3*i*w/16), (h/3), (3*w/16), (h/2), 2, ILI9341_BLACK);
    tft.setCursor(((w/8)+(3*i*w/16)+(3*w/16)/2), ((h/3)+(h/2)/2));
    tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
    tft.println(String((3-i)));
  }
  // Guitar Head
  for (int i = 0; i < 6; i++)
  {
    tft.fillRoundRect(((w/8)+(3*3*w/16)), ((h/3)+i*(h/12)), (h/12), (h/12), 2, ILI9341_WHITE);
    tft.drawRoundRect(((w/8)+(3*3*w/16)), ((h/3)+i*(h/12)), (h/12), (h/12), 2, ILI9341_BLACK);
    tft.setCursor(((w/8)+(3*3*w/16)+2), (((h/3)+i*(h/12))+2));
    tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
    tft.println(String(i));

  }

  // Menu Box
  tft.fillRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((3*w/4), 0, (w/4), (h/4), 10, ILI9341_WHITE);
  tft.setCursor((3*w/4+20), (h/16));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println("MENU");
}

void menuScreen() {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Re-Select Mode
  tft.fillRoundRect((w/4), 0, (w/2), (h/2), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((w/4), 0, (w/2), (h/2), 10, ILI9341_WHITE);
  tft.setCursor((w/4+(w/4)/8), (0+h/16));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("ReSelect");
  tft.setCursor((w/4+(w/4)/8), (2*h/16+15));
  tft.println("Mode");

  // Re-Choose What Is Chord (Chord Mode) Re-Choose Key (Key Mode)
  tft.fillRoundRect((w/4), h/2, (w/2), (h/2), 10, ILI9341_MAGENTA);
  tft.drawRoundRect((w/4), h/2, (w/2), (h/2), 10, ILI9341_WHITE);
  tft.setCursor((w/4+(w/2)/8), (h/2+h/8));
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("Back");
}

String getMenuOption() {
  String option;
  // Box Info
  int w = tft.width();
  int h = tft.height();
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
  TSPoint p = ts.getPoint();
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

void feedbackScreen(String str) {
  int w = tft.width();
  int h = tft.height();
  tft.fillScreen(ILI9341_WHITE);
  
  // Title 
  tft.setCursor(w/8, 0);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(3);
  tft.println("Feedback");

  // Message
  tft.setCursor(w/8, h/2);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(2);
  tft.println(str);
}

