
// include libraries
#include <EEPROM.h>  // include eeprom read/write library
#include <Adafruit_GFX.h>  //include graphics library
#include <MCUFRIEND_kbv.h>  // include tft library
#include <TouchScreen.h>  // include touchscreen library

// create objects
MCUFRIEND_kbv tft; // create tft object
TouchScreen ts = TouchScreen(6, A1, A2, 7, 330); //create touchscreen object and initialize it
TSPoint p; // create touchscreen point object

// declare miscellanious variables
int percentage = 100; // stores percentage readings
int onPercentage = 50; // stores on percentage
int offPercentage = 80; // stores off percentage
int offDelay = 0; // stores off delay
int msReadsCounter = 0; // used to make sure moisture sensor readings are correct. checks 10 times before firing on/off events
bool relayOnStatus = false; // stores relay on status
bool offDelayStatus = false; // stores off delay status
bool calibrationMode = false; // stores calibration mode status
unsigned long msReadDelayStart = 0; // stores start of delay time between moisture sensor readings using millis(), fires every 500ms
unsigned long relayOffDelayStart = 0; // stores start delay time for relay off using millis()
unsigned long calibrationPressStart = 0;// stores start time the calibration button was pressed using millis(), must be pressed for 5 seconds
unsigned long updateOffDelayStart = 0;// stores start of delay time between updates of the off delay count down

// declare moisture sensor raw data lmits
int MS_Min = 600; // stores moisture sensor mininum raw data
int MS_Max = 300; // stores moisture sensor maximum raw data

// declare touchscreen range raw data limits
const word TS_MINX = 190; // stores touchscreen x min value
const word TS_MAXX = 920; // stores touchscreen x max value
const word TS_MINY = 960; // stores touchscreen y min value
const word TS_MAXY = 190; // stores touchscreen y max value

// declare touchscreen colors
const int RED = 63488; // 0xF800
const int BLUE = 31; // 0x001F
const int CYAN = 2047; // 0x07FF
const int GREEN = 2016; // 0x07E0
const int WHITE = 65535; // 0xFFFF
const int BLEEN = 1748; // 0x06D4
const int BLACK = 0; // 0x0000
const int ORANGE = 64512; // 0xFC00
const int YELLOW = 65504; // 0xFFE0
const int MAGENTA = 63519; // 0xF81F
const int DARK_BLUE = 7; // 0x0003
const int DARK_GREY = 2145; // 0x0861
const int LIGHT_BLUE = 40443; // 0x9DFB
const int LIGHT_GREY = 63390; // 0xF79E
const int MEDIUM_GREY = 50712; // 0xC618
const int LIGHT_YELLOW = 65519; //

// delare EEPROM Memory Locations
const int onPercentageMemory = 0;
const int offPercentageMemory = 2;
const int offDelayMemory = 4;
const int MS_MinMemory = 6;
const int MS_MaxMemory = 8;

void setup() {
  initializeSerialPort();
  initializeEEPROM();
  initializeRelay();
  initializeTouchScreen();
  initializeTFT();
  initializeDisplay();
  initializeCalibrationMode();
}
void initializeSerialPort() {
  // enable serial debugging
  //Serial.begin(9600);
}
void initializeEEPROM() {
  // use the below line to write default settings into eeprom
  //EEPROM.put(onPercentageMemory, 100 );
  // check if values have been stored in eeprom, going on the theory that a factory 
  // eeprom will be 0 or 255 default on all bytes. I think i was getting 255 on a new uno
  if (EEPROM.read(onPercentageMemory) > 99 || EEPROM.read(offPercentageMemory) < 1 || EEPROM.read(offPercentageMemory) > 100) {
    // write default settings to eeprom
    EEPROM.put(onPercentageMemory, onPercentage);
    EEPROM.put(offPercentageMemory, offPercentage);
    EEPROM.put(offDelayMemory, offDelay);
    EEPROM.put(MS_MinMemory, MS_Min);
    EEPROM.put(MS_MaxMemory, MS_Max);
    // show calibration data as the moisture sensor has not been calibrated
    calibrationMode = true;
  }
  // read settings from eeprom
  EEPROM.get(onPercentageMemory, onPercentage);
  EEPROM.get(offPercentageMemory, offPercentage);
  EEPROM.get(offDelayMemory, offDelay);
  EEPROM.get(MS_MinMemory, MS_Min);
  EEPROM.get(MS_MaxMemory, MS_Max);
}
void initializeRelay() {
  // initialize digital pin as output for relay trigger
  pinMode(10, OUTPUT);
  // set digital pin low, relay off
  digitalWrite(10, LOW);
}
void initializeTouchScreen() {
  // already initialized
}
void initializeTFT() {
  // reset tft
  tft.reset();
  // start tft with appropriate driver;
  tft.begin(tft.readID());  //ILI9488  320x480  ID=0x9488  // set tft rotation to landscape
  // set tft rotatation to landscape
  tft.setRotation(1);
  // fill tft screen black
  tft.fillScreen(BLACK);
}
void initializeDisplay() {
  tft.setTextSize(2); // set text size (1-4)
  tft.setTextColor(WHITE, BLACK); // set text color (foreground, background)

  // border rectangle around entire screen
  tft.drawRect(0, 0, tft.width(), tft.height(), WHITE); // draw rectangle (right(x), down(y), width, height, color)

  // draw verticle lines
  for (int x = 10; x < tft.width(); x = x + 10) {
    tft.drawFastVLine(x, 0, tft.height(), DARK_BLUE);
  }

  // draw horizontal lines
  for (int y = 10; y < tft.height(); y = y + 10) {
    tft.drawFastHLine(0, y, tft.width(), DARK_BLUE);
  }

  // 'Moisture' tab
  tft.fillRoundRect(11, 11, 458, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(11, 11, 458, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  tft.setCursor(95, 37); // set cursor position (right(x), down(y))
  tft.print("Moisture"); // print to screen
  tft.setTextSize(3); // set text size (1-4)
  tft.setCursor(300, 35); // set cursor position (right(x), down(y))
  tft.print(String(percentage) + "%"); // print to screen

  // 'On' tab
  tft.fillRoundRect(11, 88, 458, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(11, 88, 458, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawCircle(44, 120, 12, GREEN); // draw outer circle for on status
  tft.setTextSize(2); // set text size (1-4)
  tft.setCursor(131, 114); // set cursor position (right(x), down(y))
  tft.print("On"); // print to screen
  tft.setCursor(227, 114); // set cursor position (right(x), down(y))
  tft.print("<"); // print to screen
  updateOnPercentage(); // update the on percentage readout
  tft.setTextSize(2); // set text size (1-4)
  tft.setTextColor(GREEN, BLACK); // set text color (foreground, background)
  tft.setCursor(432, 114); // set cursor position (right(x), down(y))
  tft.print(">"); // print to screen

  // 'Off' tab
  tft.fillRoundRect(11, 165, 458, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(11, 165, 458, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawCircle(44, 197, 12, RED); // draw outer circle for off status
  tft.fillCircle(44, 197, 8, RED); // fill circle for relay off status because it starts in the off state
  tft.setTextColor(WHITE, BLACK); // set text color (foreground, background)
  tft.setCursor(124, 191); // set cursor position (right(x), down(y)
  tft.print("Off"); // print to screen
  tft.setTextColor(RED, BLACK); // set text color (foreground, background)
  tft.setCursor(227, 191); // set cursor position (right(x), down(y))
  tft.print("<"); // print to screen
  tft.setTextColor(WHITE, BLACK); // set text color (foreground, background)
  updateOffPercentage(); // update the off percentage readout
  tft.setTextSize(2); // set text size (1-4)
  tft.setCursor(432, 191); // set cursor position (right(x), down(y))
  tft.print(">"); // print to screen

  // 'Off Delay' tab
  tft.fillRoundRect(11, 242, 458, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(11, 242, 458, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawCircle(44, 275, 12, ORANGE);// draw outer circle for off delay status
  tft.setCursor(85, 268); // set cursor position (right(x), down(y))
  tft.print("Off Delay"); // print to screen
  tft.setCursor(227, 268); // set cursor position (right(x), down(y))
  tft.print("<"); // print to screen
  updateOffDelay(offDelay); // update off delay readout with actual 'offDelay' value
  tft.setTextSize(2); // set text size (1-4)
  tft.setCursor(432, 268); // set cursor position (right(x), down(y))
  tft.print(">"); // print to screen
}
void initializeCalibrationMode() {
  // 'calibrationMode' is set true in 'initializeEEPROM()' once per arduino board.
  if (calibrationMode) {
    // view calibration data on initial startup
    showCalibrationData();
  }
}
void loop() {
  // gets the touch screen point
  getTouchPoint();
  // checks for touches
  checkTouch();
  // checks if the relay should be active
  checkEvents();
  // updates the current moisture reading
  updateMoisture();
}
void getTouchPoint() {
  // set touchscreen point p as current point
  p = ts.getPoint();
  // I'm pretty sure A1 and A2 are shared between touchscreen and tft so I believe that
  // after you get the touchscreen point you need give control back to the tft so it 
  // can draw to screen. If you look inside TouchScreen.cpp library you see it sets 
  // them to inputs at some stage.
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  // used to swap x/y coordinates due to screen rotation
  int x = p.y;
  int y = p.x;
  // converts touchscreen point p.x coordinate to pixels
  p.x = map(x, TS_MINX, TS_MAXX, tft.width(), 0);
  // converts touchscreen point p.y coordinate to pixels
  p.y = map(y, TS_MINY, TS_MAXY, tft.height(), 0);
}
void checkTouch() {
  // check if the screen is being touched
  if (p.z > ts.pressureThreshhold) {
    // check if pressed on the percentage readout a secret area
    // to activate calibration mode, press for five seconds.
    if (p.x > 268 && p.x < 405 && p.y > 12 && p.y < 79) {
      toggleCalibrationData();
    } else {
      // touch may have shifted off the percentage readout since the initial touch
      // meaning this would need to be reset here as it only resets it there is no touch
      calibrationPressStart = 0;
    }
    // check if pressed the moisture sensor raw data Min value
    // the larger value on left only available in calibration mode
    if (p.x > 200 && p.x < 267 && p.y > 12 && p.y < 79 && calibrationMode) {
      msMinValueSet();
    }
    // check if pressed the moisture sensor raw data Max value
    // the smaller value on right only available in calibration mode
    if (p.x > 402 && p.x < 469 && p.y > 12 && p.y < 79 && calibrationMode) {
      msMaxValueSet();
    }
    // check if pressed the on percentage down button
    if (p.x > 200 && p.x < 267 && p.y > 88 && p.y < 155) {
      onPercentageDown();
    }
    // check if pressed the on percentage up button
    if (p.x > 402 && p.x < 469 && p.y > 88 && p.y < 155) {
      onPercentageUp();
    }
    // check if pressed the off percentage down button
    if (p.x > 200 && p.x < 267 && p.y > 165 && p.y < 232) {
      offPercentageDown();
    }
    // check if pressed the off percentage up button
    if (p.x > 402 && p.x < 469 && p.y > 165 && p.y < 232) {
      offPercentageUp();
    }
    // check if pressed the off delay down button
    if (p.x > 200 && p.x < 267 && p.y > 242 && p.y < 309) {
      offDelayDown();
    }
    // check if pressed the off delay up button
    if (p.x > 402 && p.x < 469 && p.y > 242 && p.y < 309) {
      offDelayUp();
    }
  } else {
    //Serial.println("no touch detected");
    // need reset here as there is no touch detected
    calibrationPressStart = 0;
  }
}
void toggleCalibrationData() {
  // this routine is only hit if pressing on the percentage readout.
  if (calibrationPressStart == 0) {
    // set calibrationPressStart equal to current
    // millis(), to initiate the five second delay
    calibrationPressStart = millis();
  }
  // check 5 seconds (5000ms) has passed since the
  // initial touch then show/hide calibration data
  if (millis() >= (calibrationPressStart + 5000)) {
    // reset as the five second delay has ended
    calibrationPressStart = 0;
    if (calibrationMode) {
      hideCalibrationData();
    } else {
      showCalibrationData();
    }
  }
}
void hideCalibrationData() {
  calibrationMode = false;
  // clear moisture sensor Mininum raw data
  tft.setTextSize(2);
  tft.setCursor(220, 38);
  tft.print("    ");
  // clear moisture sensor raw data
  tft.setTextSize(1);
  tft.setCursor(320, 20);
  tft.print("    ");
  // clear moisture sensor Maximum raw data
  tft.setTextSize(2);
  tft.setCursor(414, 38);
  tft.print("    ");
}
void showCalibrationData() {
  calibrationMode = true;
  // show moisture sensor Mininum raw data
  tft.setTextSize(2);
  tft.setCursor(220, 38);
  tft.print(MS_Min);
  // show moisture sensor raw data
  tft.setTextSize(1);
  tft.setCursor(320, 20);
  tft.print(analogRead(5));
  // show moisture sensor Maximum raw data
  tft.setTextSize(2);
  tft.setCursor(414, 38);
  tft.print(MS_Max);
}
void msMinValueSet() {
  MS_Min = analogRead(5);
  // store min value in eeprom
  EEPROM.put(MS_MinMemory, MS_Min);
  updateMSMinValue();
}
void updateMSMinValue() {
  // show moisture sensor Mininum raw data
  tft.setTextSize(2);
  tft.setCursor(220, 38);
  tft.print(MS_Min);
}
void msMaxValueSet() {
  MS_Max = analogRead(5);
  // store max value in eeprom
  EEPROM.put(MS_MaxMemory, MS_Max);
  updateMSMaxValue();
}
void updateMSMaxValue() {  
  // show moisture sensor Maximum raw data
  tft.setTextSize(2);
  tft.setCursor(414, 38);
  tft.print(MS_Max);
}
void onPercentageDown() {
  if (onPercentage > 0) {
    onPercentage -= 1;
    EEPROM.put(onPercentageMemory, onPercentage);
    updateOnPercentage();
  }
}
void onPercentageUp() {
  if (onPercentage < 99) {
    onPercentage += 1;
    EEPROM.put(onPercentageMemory, onPercentage);
    updateOnPercentage();
    if (onPercentage >= offPercentage) {
      offPercentageUp();
    }
  }
}
void updateOnPercentage() {
  tft.setTextSize(3); // set text size
  tft.setCursor(300, 110); // set cursor position
  if (onPercentage == 100) {
    tft.print(String(onPercentage) + "%");
  } else if (onPercentage < 100 && onPercentage > 9) {
    tft.print("0" + String(onPercentage) + "%");
  } else if (onPercentage < 10) {
    tft.print("00" + String(onPercentage) + "%");
  }
}
void offPercentageDown() {
  if (offPercentage > 1) {
    offPercentage -= 1;
    EEPROM.put(offPercentageMemory, offPercentage);
    updateOffPercentage();
    if (offPercentage <= onPercentage) {
      onPercentageDown();
    }
  }
}
void offPercentageUp() {
  if (offPercentage < 100) {
    offPercentage += 1;
    EEPROM.put(offPercentageMemory, offPercentage);
    updateOffPercentage();
  }
}
void updateOffPercentage() {
  // set text size
  tft.setTextSize(3);
  // set cursor position
  tft.setCursor(300, 190);
  if (offPercentage == 100) {
    tft.print(String(offPercentage) + "%");
  } else if (offPercentage < 100 && offPercentage > 9) {
    tft.print("0" + String(offPercentage) + "%");
  } else if (offPercentage < 10) {
    tft.print("00" + String(offPercentage) + "%");
  }
}
void offDelayDown() {
  if (offDelay > 0) {
    offDelay -= 1;
    EEPROM.put(offDelayMemory, offDelay);
    updateOffDelay(offDelay); // update off delay readout with actual 'offDelay' value
  }
}
void offDelayUp() {
  if (offDelay < 999) {
    offDelay += 1;
    EEPROM.put(offDelayMemory, offDelay);
    updateOffDelay(offDelay); // update off delay readout with actual 'offDelay' value
  }
}
void updateOffDelay(int delayTime) {  
  tft.setTextSize(3); // set text size
  tft.setCursor(300, 265); // set cursor position
  if (delayTime > 99) {
    tft.print(String(delayTime) + "s");
  } else if (delayTime < 100 && delayTime > 9) {
    tft.print("0" + String(delayTime) + "s");
  } else if (delayTime < 10) {
    tft.print("00" + String(delayTime) + "s");
  }
}
void checkEvents() {
  if (percentage <= onPercentage && (relayOnStatus == false || offDelayStatus == true)) {
    relayOnEvent();
  } else if (percentage >= offPercentage && offDelay == 0 && relayOnStatus == true) {
    relayOffEvent();
  } else if (percentage >= offPercentage && offDelay > 0 && relayOnStatus == true) {
    relayOffDelayEvent();
  } else {
    resetEvents();
  }
}
void relayOnEvent() {
  msReadsCounter += 1; // increment counter by 1
  if (msReadsCounter > 10) { // check if counter is greater than 10
    relayOn();
  }
}
void relayOn() {
  digitalWrite(10, HIGH); // set relay on
  relayOnStatus = true;
  updateOnStatus();
  updateOffDelay(offDelay); // update off delay readout with actual 'offDelay' value
  resetEvents();
}
void updateOnStatus() {
  // fill circle for on status
  tft.fillCircle(44, 120, 8, GREEN);
  // fill circle for off status
  tft.fillCircle(44, 197, 8, BLACK);
  // fill circle for off delay status
  tft.fillCircle(44, 275, 8, BLACK);
}
void relayOffEvent() {
  msReadsCounter += 1; // increment counter by 1
  if (msReadsCounter > 10) {// check if counter is greater than 10
    relayOff();
  }
}
void relayOff() {
  digitalWrite(10, LOW); // set relay off
  relayOnStatus = false;
  updateOffStatus();
  updateOffDelay(offDelay); // update off delay readout with actual 'offDelay' value
  resetEvents();
}
void updateOffStatus() {
  // fill circle for on status
  tft.fillCircle(44, 120, 8, BLACK);
  // fill circle for off status
  tft.fillCircle(44, 197, 8, RED);
  // fill circle for off delay status
  tft.fillCircle(44, 275, 8, BLACK);
}
void relayOffDelayEvent() {
  if (relayOffDelayStart == 0) {
    relayOffDelayStart = millis();
    updateOffDelayStart = relayOffDelayStart;
    offDelayStatus = true;
    updateOffDelayStatus();
  }
  // update the off delay count down
  updateOffDelayCountDown();
  // check if the off delay should end
  if (millis() >= relayOffDelayStart + (long(offDelay) * 1000)) {
    relayOff();
  }
}
void updateOffDelayStatus() {
  // fill circle for on status
  tft.fillCircle(44, 120, 8, GREEN);
  // fill circle for off status
  tft.fillCircle(44, 197, 8, BLACK);
  // fill circle for off delay status
  tft.fillCircle(44, 275, 8, ORANGE);
}
void updateOffDelayCountDown(){
  // put a 500 milli second update delay here
  if (millis() >= updateOffDelayStart + 500){
    // displays a count down of the delay time 
    updateOffDelay(((relayOffDelayStart + (long(offDelay) * 1000)) - millis()) / 1000);
    // add half a second (500ms) to the updateOffDelayStart for the next update
    updateOffDelayStart += 500;
  }  
}
void resetEvents() {
  msReadsCounter = 0; // reset the counter as there are no events
  offDelayStatus = false; // reset as there is no off delay active
  relayOffDelayStart = 0; // reset to zero as there is no off delay
  updateOffDelayStart = 0; // reset here but should not be needed as it gets set only at the beginning of a off delay event
}
void updateMoisture() {
  // if you let the arduino go at full speed the moisture readout (this routine) would be 
  // updated ten's to hundred's (well maybe not hundred's) of times every second creating a flickery
  // readout. so we create a delay using millis() which when called gives you the milli seconds the arduino
  // has been turned on. the first time this routine is hit msReadDelayStart will equal 0  
  if (msReadDelayStart == 0) {    
    // if msReadDelayStart equals 0 it is set to the current millis().
    msReadDelayStart = millis();
  }
  // so this routine will loop until 500ms have passed then update the reading
  if (millis() >= (msReadDelayStart + 500)) {
    // need reset here to restart the 500ms delay
    msReadDelayStart = 0;
    // convert moisture sensor range into percentage range
    percentage = map(analogRead(5), MS_Min, MS_Max, 0, 100);
    // check if percentage is below 0
    if (percentage < 0) {
      percentage = 0; // set percentage to 0
    }
    // check if percentage is above 100
    if (percentage > 100) {
      percentage = 100; // set percentage to 100
    }
    tft.setTextSize(3); // set text size
    tft.setCursor(300, 35); // set cursor position
    if (percentage == 100) {
      tft.print(String(percentage) + "%");
    } else if (percentage < 100 && percentage > 9) {
      tft.print("0" + String(percentage) + "%");
    } else if (percentage < 10) {
      tft.print("00" + String(percentage) + "%");
    }    
    // displays calibration data if required. I put it here so
    // it only updates every 500ms.
    updateCalibrationData();
  }
}
void updateCalibrationData() {
  if (calibrationMode) {
    // update moisture sensor raw calibration data
    tft.setTextSize(1);
    tft.setCursor(320, 20);
    tft.print(analogRead(5));
  }
}
