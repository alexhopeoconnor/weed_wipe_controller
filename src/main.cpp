#include <EEPROM.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// declare touchscreen range raw data limits
#define TS_MINX 190 // stores touchscreen x min value
#define  TS_MAXX 920 // stores touchscreen x max value
#define  TS_MINY 960 // stores touchscreen y min value
#define  TS_MAXY 190 // stores touchscreen y max value

// Define touchscreen colors
#define RED 63488
#define BLUE 31
#define CYAN 2047
#define GREEN 2016
#define WHITE 65535
#define BLEEN 1748
#define BLACK 0
#define ORANGE 64512
#define YELLOW 65504
#define MAGENTA 63519
#define DARK_BLUE 7
#define DARK_GREY 2145
#define LIGHT_BLUE 40443
#define LIGHT_GREY 63390
#define MEDIUM_GREY 50712
#define LIGHT_YELLOW 65519

// Define pins
#define PUMP_PIN 10

// Define behaviour
#define CONTROL_TIME_MIN 5 // seconds
#define CONTROL_TIME_MAX 45 // seconds
#define PUMP_MIN_PERCENT 50 // %
#define PUMP_MAX_PERCENT 100 // %
#define TOUCH_ACTION_DELAY 50 // ms
#define SAVE_NOTIFICATION_DELAY 2500 // ms

// Config definition
#define CONFIG_VERSION "V2"
typedef struct
{
  char version[5];
  unsigned int on_time;
  unsigned int off_time;
  unsigned int pump_on_percentage;
} configuration_type;

// Global configuration instance
configuration_type CONFIGURATION = {
  CONFIG_VERSION,
  5,
  10,
  100
};

// TFT display
MCUFRIEND_kbv tft; // create tft object
TouchScreen ts = TouchScreen(6, A1, A2, 7, 330); //create touchscreen object and initialize it
TSPoint p;

// Global control variables
bool isPumpOn = false;
bool isTouching = false;
bool savedSettings = true;
bool showSavedConfig = false;
unsigned long lastPumpStateSwitch = millis();
unsigned long lastSavedConfig = millis();

void updateOnTime() {
  tft.setTextSize(3); // set text size (1-4)
  tft.setCursor(300, 35); // set cursor position (right(x), down(y))
  tft.print(String(CONFIGURATION.on_time) + " s  "); // print to screen
}

void updateOffTime() {
  tft.setTextSize(3); // set text size
  tft.setCursor(300, 110); // set cursor position
  tft.print(String(CONFIGURATION.off_time) + " s  ");
}

void updatePumpOnPercentage() {
  tft.setTextSize(3); // set text size
  tft.setCursor(300, 191); // set cursor position
  tft.print(String(CONFIGURATION.pump_on_percentage) + " % ");
}

void updatePumpIndicator() {
  if(isPumpOn) {
    tft.fillCircle(40, 198, 20, GREEN);
  } else {
    tft.fillCircle(40, 198, 20, RED);
  }
}

void updateSaveButton() {
  // Manage previous saved state
  static bool prevSaved = false;

  if(prevSaved != savedSettings) {
    tft.setTextSize(3); // set text size (1-4)
    tft.setCursor(350, 265); // set cursor position (right(x), down(y))
    if(!savedSettings) {
      tft.print("Save!"); // print to screen
    } else {
      tft.print("Saved"); // print to screen
    }
  }
  prevSaved = savedSettings;
}

void updateSavedNotification() {
  // Define notification state management
  static bool prevShowing = false;

  // Time the notification out
  if(prevShowing && ((lastSavedConfig + SAVE_NOTIFICATION_DELAY) <= millis() || !savedSettings)) {
    showSavedConfig = false;
  }

  // Update the notification
  if(prevShowing != showSavedConfig) {
    tft.setTextColor(GREEN, BLACK);
    tft.setTextSize(3); // set text size
    tft.setCursor(100, 262); // set cursor position
    if(showSavedConfig) {
      tft.print("Saved!");
    } else {
      tft.print("      ");
    }
    tft.setTextColor(WHITE, BLACK);
  }
  prevShowing = showSavedConfig;
}

void initTFT() {
  tft.reset();
  tft.begin(tft.readID());  //ILI9488  320x480  ID=0x9488  // set tft rotation to landscape
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  tft.setTextSize(2); // set text size (1-4)
  tft.setTextColor(WHITE, BLACK); // set text color (foreground, background)

  // border rectangle around entire screen
  tft.drawRect(0, 0, tft.width(), tft.height(), WHITE); // draw rectangle (right(x), down(y), width, height, color)

  // On Time
  tft.fillRoundRect(11, 11, 458, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(11, 11, 458, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  tft.setCursor(95, 37); // set cursor position (right(x), down(y))
  tft.print("On Time"); // print to screen
  updateOnTime();
  tft.setCursor(227, 35); // set cursor position (right(x), down(y))
  tft.print("<"); // print to screen
  tft.setCursor(432, 35); // set cursor position (right(x), down(y))
  tft.print(">"); // print to screen

  // Off Time
  tft.fillRoundRect(11, 88, 458, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(11, 88, 458, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  tft.setTextSize(2); // set text size (1-4)
  tft.setCursor(95, 114); // set cursor position (right(x), down(y))
  tft.print("Off Time"); // print to screen
  tft.setCursor(227, 114); // set cursor position (right(x), down(y))
  tft.print("<"); // print to screen
  updateOffTime();
  tft.setTextSize(2); // set text size (1-4)
  tft.setCursor(432, 114); // set cursor position (right(x), down(y))
  tft.print(">"); // print to screen

  // Pump on percentage
  tft.fillRoundRect(11, 165, 458, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(11, 165, 458, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  tft.setTextSize(2); // set text size (1-4)
  tft.setCursor(95, 191); // set cursor position (right(x), down(y))
  tft.print("Pump Rate"); // print to screen
  tft.setCursor(227, 191); // set cursor position (right(x), down(y))
  tft.print("<"); // print to screen
  updatePumpOnPercentage();
  tft.setTextSize(2); // set text size (1-4)
  tft.setCursor(432, 191); // set cursor position (right(x), down(y))
  tft.print(">"); // print to screen

  // Pump indicator
  updatePumpIndicator();

  // Save config button
  tft.fillRoundRect(320, 242, 150, 67, 33, BLACK); // draw rectangle (right(x), down(y), width, height, color)
  tft.drawRoundRect(320, 242, 150, 67, 33, WHITE); // draw rectangle (right(x), down(y), width, height, color)
  updateSaveButton();
}

int loadConfig() 
{
  // Check the version, load (overwrite) the local configuration struct
  if (EEPROM.read(0) == CONFIG_VERSION[0] &&
      EEPROM.read(1) == CONFIG_VERSION[1] &&
      EEPROM.read(2) == CONFIG_VERSION[2]) {
    for (unsigned int i = 0; i < sizeof(CONFIGURATION); i++) {
      *((char*)&CONFIGURATION + i) = EEPROM.read(i);
    }
    return 1;
  }
  return 0;
}

void saveConfig() 
{
  // save the CONFIGURATION in to EEPROM
  for (unsigned int i = 0; i < sizeof(CONFIGURATION); i++) {
    EEPROM.write(i, *((char*)&CONFIGURATION + i));
  }

  // Show the save notification
  savedSettings = true;
  lastSavedConfig = millis();
  showSavedConfig = true;
  updateSavedNotification();
}

void setup() {
  // Load config
  EEPROM.begin();
  if(!loadConfig()) {
    saveConfig();
  }

  // Initialize pump control
  pinMode(PUMP_PIN, OUTPUT);
  analogWrite(PUMP_PIN, 0);

  // Initialize TFT
  initTFT();
}

void getTouchPoint() {
  // set touchscreen point p as current point
  p = ts.getPoint();
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);

  // used to swap x/y coordinates due to screen rotation
  int x = p.y;
  int y = p.x;
  p.x = map(x, TS_MINX, TS_MAXX, tft.width(), 0);
  p.y = map(y, TS_MINY, TS_MAXY, tft.height(), 0);
}

void performTouch() {
  // Touch management variables
  static bool wasTouching = false;
  static unsigned long lastAction = millis();

  // check if the screen is being touched
  if (p.z > ts.pressureThreshhold) {
    isTouching = true;
    if((lastAction + TOUCH_ACTION_DELAY) <= millis()) {
      if(p.x > 200 && p.x < 267) {
        if (p.y > 12 && p.y < 79) { // Decrease on time
          if(CONFIGURATION.on_time > CONTROL_TIME_MIN) {
            CONFIGURATION.on_time -= 1;
            updateOnTime();
            savedSettings = false;
          }
        } else  if (p.y > 91 && p.y < 158) { // Decrease off time
          if(CONFIGURATION.off_time > CONTROL_TIME_MIN) {
            CONFIGURATION.off_time -= 1;
            updateOffTime();
            savedSettings = false;
          }
        } else  if (p.y > 168 && p.y < 235) { // Decrease pump on percentage
          if(CONFIGURATION.pump_on_percentage > PUMP_MIN_PERCENT) {
            CONFIGURATION.pump_on_percentage -= 1;
            updatePumpOnPercentage();
            savedSettings = false;
          }
        } 
      } else if(p.x > 402 && p.x < 469) {
        if (p.y > 12 && p.y < 79) { // Increase on time
          if(CONFIGURATION.on_time < CONTROL_TIME_MAX) {
            CONFIGURATION.on_time += 1;
            updateOnTime();
            savedSettings = false;
          }
        } else if (p.y > 91 && p.y < 158) { // Increase off time
          if(CONFIGURATION.off_time < CONTROL_TIME_MAX) {
            CONFIGURATION.off_time += 1;
            updateOffTime();
            savedSettings = false;
          }
        } else if (p.y > 168 && p.y < 235) { // Increase pump on percentage
          if(CONFIGURATION.pump_on_percentage < PUMP_MAX_PERCENT) {
            CONFIGURATION.pump_on_percentage += 1;
            updatePumpOnPercentage();
            savedSettings = false;
          }
        } else if(p.x > 320 && p.x < 470 && p.y > 242 && p. y < 309) { // Save config
          if(!wasTouching && !savedSettings) {
            saveConfig();
          }
        }
      } 
      lastAction = millis();
    }
  } else {
    isTouching = false;
  }
  wasTouching = isTouching;
}

void operatePump() {
  // Manage previous pump state
  static bool prevPumpOn = !isPumpOn;
  static unsigned int prevPumpPercent;

  // Calculate next state change delay in ms, trigger pump state switch
  unsigned long nextStateChange = (isPumpOn ? CONFIGURATION.on_time : CONFIGURATION.off_time) * 1000;
  if((lastPumpStateSwitch + nextStateChange) <= millis()) {
    isPumpOn = !isPumpOn;
    lastPumpStateSwitch = millis();
  }

  // Change the pump state
  if(prevPumpOn != isPumpOn) {
    if(isPumpOn) {
      // Switch pump on at specified rate
      unsigned int pumpDuty = 255 * (CONFIGURATION.pump_on_percentage / 100.0);
      analogWrite(PUMP_PIN, pumpDuty);
    } else {
      // Turn the pump off
      analogWrite(PUMP_PIN, 0);
    }
    updatePumpIndicator();
  }

  // Update the duty cycle
  if(prevPumpPercent != CONFIGURATION.pump_on_percentage && isPumpOn) {
    unsigned int pumpDuty = 255 * (CONFIGURATION.pump_on_percentage / 100.0);
    analogWrite(PUMP_PIN, pumpDuty);
  }
  prevPumpOn = isPumpOn;
  prevPumpPercent = CONFIGURATION.pump_on_percentage;
}

void loop() {
  // gets the touch screen point
  getTouchPoint();

  // Performs touch interaction
  performTouch();

  // GUI saved state
  updateSaveButton();
  updateSavedNotification();

  // Operate the pump
  operatePump();
}