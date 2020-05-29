#include <Arduino.h>
#include <TM1637Display.h>
#include "TinyWireM.h"

// TM1637 module connection pins
#define CLK 3
#define DIO 4

// RTC module I2C addresses
#define DS3231_I2C_ADDRESS    0x68
#define AT24C32_I2C_ADDRESS   0x57
#define EEPROM_START_ADDRESS  0x60
#define DEBUG_ADDRESS         0     // relative to EEPROM_START_ADDRESS
#define PEE_HOUR_ADDRESS      1     // relative to EEPROM_START_ADDRESS
#define PEE_MINUTE_ADDRESS    2     // relative to EEPROM_START_ADDRESS
#define POO_HOUR_ADDRESS      3     // relative to EEPROM_START_ADDRESS
#define POO_MINUTE_ADDRESS    4     // relative to EEPROM_START_ADDRESS
#define BRIGHTNESS_ADDRESS    5     // relative to EEPROM_START_ADDRESS

// button readouts from analog P5
#define BTN_LEFT_VAL      558
#define BTN_RIGHT_VAL     614
#define BTN_BOTH_VAL      644
#define BTN_AMPLITUDE     14

#define BOOT_DEBUG_FLAG     0x55
#define SUBMODE_DELAY_MS    2000   // time in ms that it takes to move to next submode if not at last submode

// major modes of operation -> states in statemachine
#define MODE_CLOCK       0
#define MODE_PEE_INFO    1
#define MODE_POO_INFO    2
#define MODE_PEE_SET     3
#define MODE_POO_SET     4
#define MODE_CLOCK_SET   5
#define MODE_TEST        6

// button states and config
#define BTN_STATE_CLEAR     0
#define BTN_STATE_PRESSED   1
#define BTN_STATE_HOLD      2
#define BTN_TIME_DEBOUNCE   50    // how many ms must a button consistently be pressed to be detected as being pressed
#define BTN_TIME_HOLD       1000   // how many ms must a button consistently be pressed to be detected as being hold

// display settings
#define DISPLAY_MIN_BRIGHTNESS  0
#define DISPLAY_MAX_BRIGHTNESS  7

TM1637Display display(CLK, DIO);
uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };

uint8_t displayBrightness = DISPLAY_MAX_BRIGHTNESS;
byte second;
byte minute;
byte hour;
byte dayOfWeek;
byte dayOfMonth;
byte month;
byte year;
byte mode = MODE_CLOCK;
byte submode = 0;
uint32_t submodeStart = 0;
bool lastSubmode = false;

bool buttonLeft = false;
bool buttonRight = false;
uint16_t buttonLastRead = 0;

uint32_t timeStartLeft  = 0;
uint32_t timeStartRight = 0;
uint32_t timeStartBoth  = 0;
uint8_t buttonStateLeft   = BTN_STATE_CLEAR;
uint8_t buttonStateRight  = BTN_STATE_CLEAR;
uint8_t buttonStateBoth   = BTN_STATE_CLEAR;

uint16_t pmAddress = 0;
uint16_t phAddress = 0;


// some constants for displaying info messages as text
const uint8_t SEG_PEE[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G ,          // P
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  0
  };

const uint8_t SEG_POO[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G ,          // P
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  0
  };

const uint8_t SEG_UHR[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // U
  SEG_C | SEG_E | SEG_F | SEG_G,            // h
  SEG_E | SEG_G,                            // r
  0
  };

const uint8_t SEG_DAT[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
  SEG_D | SEG_E | SEG_F | SEG_G,                  // t
  0
  };

const uint8_t SEG_TEST[] = {
  SEG_A | SEG_B | SEG_C ,                          // T
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  SEG_A | SEG_D | SEG_C | SEG_F | SEG_G,           // S
  SEG_A | SEG_B | SEG_C                            // T
  };

const uint8_t SEG_SET[] = {
  SEG_A | SEG_D | SEG_C | SEG_F | SEG_G,           // S
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  SEG_A | SEG_B | SEG_C,                           // T
  0
  };

const uint8_t SEG_LAST[] = {
  SEG_D | SEG_E | SEG_F ,                          // L
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,   // A
  SEG_A | SEG_D | SEG_C | SEG_F | SEG_G,           // S
  SEG_A | SEG_B | SEG_C                            // T
  };

const uint8_t SEG_JAHR[] = {
  SEG_B | SEG_C | SEG_D ,                          // J
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,   // A
  SEG_C | SEG_E | SEG_F | SEG_G,                   // h
  SEG_E | SEG_G                                    // r
  };

const uint8_t SEG_HELL[] = {
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,           // H
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  SEG_D | SEG_E | SEG_F ,                          // L
  SEG_D | SEG_E | SEG_F                            // L
  };

const uint8_t SEG_BOOT[] = {
  SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,    // b
  SEG_C | SEG_D | SEG_E | SEG_G,            // o
  SEG_C | SEG_D | SEG_E | SEG_G,            // o
  SEG_D | SEG_E | SEG_F | SEG_G             // t
  };


// helper functions
byte highAddressByte(word  address)
{
  return byte(address >> 8);
}

byte lowAddressByte(word address)
{
  return byte(address & 0xff);
}

// write to given address in RTC onboard EEPROM (it seems safe to use addresses after 0x60)
void writeEEPROM(uint16_t address, uint8_t value)
{
  TinyWireM.beginTransmission(AT24C32_I2C_ADDRESS);
  
  TinyWireM.write(highAddressByte(address));      //First Word Address
  TinyWireM.write(lowAddressByte(address));      //Second Word Address
  TinyWireM.write(value);        
  delay(10);
  
  TinyWireM.endTransmission();
  delay(10);  
}

// read from given address in RTC onboard EEPROM 
uint8_t readEEPROM(uint16_t address)
{
  uint8_t value=0;
  TinyWireM.beginTransmission(AT24C32_I2C_ADDRESS);
  TinyWireM.write(highAddressByte(address));      //First Word Address
  TinyWireM.write(lowAddressByte(address));      //Second Word Address
  TinyWireM.endTransmission();
  delay(10);
  
  TinyWireM.requestFrom(AT24C32_I2C_ADDRESS, 1);        
  delay(10);
  value = TinyWireM.read();
  delay(10);
  return value;
}

// helper functions for converting BCD encoded time values from RTC
uint8_t decToBcd(uint8_t val)
{
  uint8_t _val = val;
  return ((_val / 10 * 16) + (_val % 10));
}

uint8_t bcdToDec(uint8_t val)
{
  uint8_t _val = val;
  return ((_val / 16 * 10) + (_val % 16));
}

// sets the RTC to a given time and date
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  TinyWireM.beginTransmission(DS3231_I2C_ADDRESS);
  TinyWireM.write(0); // set next input to start at the seconds register
  TinyWireM.write(decToBcd(second)); // set seconds
  TinyWireM.write(decToBcd(minute)); // set minutes
  TinyWireM.write(decToBcd(hour)); // set hours
  TinyWireM.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  TinyWireM.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  TinyWireM.write(decToBcd(month)); // set month
  TinyWireM.write(decToBcd(year)); // set year (0 to 99)
  TinyWireM.endTransmission();
}

// reads time and date from RTC
void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
  TinyWireM.beginTransmission(DS3231_I2C_ADDRESS);
  TinyWireM.write(0); // set DS3231 register pointer to 00h
  TinyWireM.endTransmission();
  byte datalen = 7;
  TinyWireM.requestFrom(DS3231_I2C_ADDRESS, datalen);             // request seven bytes of data from DS3231 starting from register 00h
  if (TinyWireM.available()) {
    *second = bcdToDec(TinyWireM.read() & 0b01111111);            // 0x00 seconds
    *minute = bcdToDec(TinyWireM.read() & 0b01111111);            // 0x01 minutes
    *hour = bcdToDec(TinyWireM.read() & 0b00111111);              // 0x02 hours
    *dayOfWeek = bcdToDec(TinyWireM.read() & 0b00000111);         // 0x03 "day" in the datasheet
    *dayOfMonth = bcdToDec(TinyWireM.read() & 0b00111111);        // 0x04 "date" in the datasheet
    *month = bcdToDec(TinyWireM.read() & 0b00011111);             // 0x05 month
    *year = bcdToDec(TinyWireM.read());                           // 0x06 year
  } else
  {
    // write failed but we can't tell anyone on the digispark (no serial!)
  }
}

// change to another mode (and reset helpers)
void changeMode(uint8_t targetMode, uint8_t targetSubmode = 0) {
  mode = targetMode;
  submode = targetSubmode;
  lastSubmode = false;
  submodeStart = millis();
}

// copies 4 bytes from the given constant array to the "drawing buffer" data array
void copyText(const uint8_t source[]) {
  data[0] = source[0]; data[1] = source[1]; data[2] = source[2]; data[3] = source[3]; 
}

void setup() {
  TinyWireM.begin();                    // initialize I2C lib
  pinMode(5, INPUT);
  display.setBrightness(0x0f);
  //setDS3231time(0, 58, 2, 5, 21, 05, 20); // do this only one time at initial setup if RTC has not been set already
  bool enterTestmode = false;

  uint8_t ram;
  ram = readEEPROM(EEPROM_START_ADDRESS + DEBUG_ADDRESS);
  if (ram == BOOT_DEBUG_FLAG) enterTestmode = true;

  writeEEPROM(EEPROM_START_ADDRESS + DEBUG_ADDRESS, BOOT_DEBUG_FLAG);
  copyText(SEG_BOOT);
  display.setSegments(data);
  for (int i = 0; i<60; i++) {
    delay(50);
    readButtons();
    checkButtons();
    if (buttonStateLeft == BTN_STATE_HOLD) enterTestmode = true;
    if (enterTestmode) {
      data[1] |= (i & 0x01)<<7;
      display.setSegments(data);  
    }
  }
  writeEEPROM(EEPROM_START_ADDRESS + DEBUG_ADDRESS, 0);

  if (enterTestmode) {
    changeMode(MODE_TEST);
  } else {
    ram = readEEPROM(EEPROM_START_ADDRESS + BRIGHTNESS_ADDRESS);
    if ((ram >= DISPLAY_MIN_BRIGHTNESS) && (ram <= DISPLAY_MAX_BRIGHTNESS)) displayBrightness = ram;
    display.setBrightness(displayBrightness);
    writeEEPROM(EEPROM_START_ADDRESS + BRIGHTNESS_ADDRESS, displayBrightness);
    changeMode(MODE_CLOCK);
  }
}

// make a raw analog read and interpret which buttons are pressed
void readButtons() {
  buttonLastRead = analogRead(0); //P5!
  buttonLeft = false;
  buttonRight = false;
  if ((buttonLastRead > BTN_LEFT_VAL - BTN_AMPLITUDE) && (buttonLastRead < BTN_LEFT_VAL + BTN_AMPLITUDE)) buttonLeft = true;
  if ((buttonLastRead > BTN_RIGHT_VAL - BTN_AMPLITUDE) && (buttonLastRead < BTN_RIGHT_VAL + BTN_AMPLITUDE)) buttonRight = true;
  if ((buttonLastRead > BTN_BOTH_VAL - BTN_AMPLITUDE) && (buttonLastRead < BTN_BOTH_VAL + BTN_AMPLITUDE)) {buttonLeft = true; buttonRight = true;}
}

// keep track of which button has been pressed so events of "pressed" and "hold" can be issued for each or both buttons
void checkButtons() {
  uint32_t currentTime = millis();
  if (buttonLeft) {
    if (timeStartLeft == 0) timeStartLeft = currentTime;
    if (currentTime - timeStartLeft > BTN_TIME_HOLD) buttonStateLeft = BTN_STATE_HOLD;
  } else {
    if ((buttonStateLeft != BTN_STATE_HOLD) && (timeStartLeft > 0)  && (currentTime - timeStartLeft > BTN_TIME_DEBOUNCE)) {
      buttonStateLeft = BTN_STATE_PRESSED;
    } else {
      buttonStateLeft = BTN_STATE_CLEAR; 
    }
    timeStartLeft = 0;
  }
  if (buttonRight) {
    if (timeStartRight == 0) timeStartRight = currentTime;
    if (currentTime - timeStartRight > BTN_TIME_HOLD) buttonStateRight = BTN_STATE_HOLD;
  } else {
    if ((buttonStateRight != BTN_STATE_HOLD) && (timeStartRight > 0) && (currentTime - timeStartRight > BTN_TIME_DEBOUNCE)) {
      buttonStateRight = BTN_STATE_PRESSED;
    } else {
      buttonStateRight = BTN_STATE_CLEAR;
    }
    timeStartRight = 0;
  }
  if (buttonLeft && buttonRight) {
    timeStartLeft = 0;
    timeStartRight = 0;
    buttonStateLeft = BTN_STATE_CLEAR;
    buttonStateRight = BTN_STATE_CLEAR;
    if (timeStartBoth == 0) timeStartBoth = currentTime;
    if (currentTime - timeStartBoth > BTN_TIME_HOLD) buttonStateBoth = BTN_STATE_HOLD;
  } else {
    if ((buttonStateBoth != BTN_STATE_HOLD) && (timeStartBoth > 0)  && (currentTime - timeStartBoth > BTN_TIME_DEBOUNCE)) {
      buttonStateBoth = BTN_STATE_PRESSED;
    } else {
      buttonStateBoth = BTN_STATE_CLEAR;
    }
    timeStartBoth = 0;
  }
}

void resetButtons() {
  timeStartLeft = 0;
  timeStartRight = 0;
  timeStartBoth = 0;
}

void handleMode() {
  switch (mode) { 
    case MODE_CLOCK : {
      readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year); // call is pretty expensive, in battery power modes this should be calles seldomly and the internal timer is used regularly
      break;
    }
    case MODE_PEE_INFO:
    case MODE_PEE_SET : {
      pmAddress = EEPROM_START_ADDRESS + PEE_MINUTE_ADDRESS;
      phAddress = EEPROM_START_ADDRESS + PEE_HOUR_ADDRESS;
      break;
    }
    case MODE_POO_INFO:
    case MODE_POO_SET : {
      pmAddress = EEPROM_START_ADDRESS + POO_MINUTE_ADDRESS;
      phAddress = EEPROM_START_ADDRESS + POO_HOUR_ADDRESS;
      break;
    }
    
  }
  
}

void handleSubmode() {
  uint32_t currentTime = millis();
  if ((!lastSubmode) && (currentTime - submodeStart > SUBMODE_DELAY_MS)) {
    submode++;
    submodeStart = currentTime;
  }
  switch (mode) { 
    case MODE_CLOCK : {
      switch (submode) {
        case 0: {
          copyText(SEG_UHR);
          lastSubmode = false;
          break;
        }
        case 1: { // standard clock
          data[3] = display.encodeDigit(minute % 10);
          data[2] = display.encodeDigit(minute / 10);
          data[1] = display.encodeDigit(hour % 10) | (second << 7);
          data[0] = display.encodeDigit(hour / 10);
          lastSubmode = true;
          if (buttonStateLeft == BTN_STATE_PRESSED) changeMode(MODE_PEE_INFO);
          if (buttonStateRight == BTN_STATE_PRESSED) changeMode(MODE_POO_INFO);
          if (buttonStateLeft == BTN_STATE_HOLD) changeMode(MODE_PEE_SET);
          if (buttonStateRight == BTN_STATE_HOLD) changeMode(MODE_POO_SET);
          if (buttonStateBoth == BTN_STATE_HOLD) changeMode(MODE_CLOCK_SET);
          break;
        }
      }
      break;
    }
    case MODE_TEST : {
      switch (submode) {
        case 0: {
          copyText(SEG_TEST);
          lastSubmode = false;
          break;
        }
        case 1: { // test mode of interpreted buttons
          data[0] = 0;
          data[1] = 0;
          data[2] = 0;
          data[3] = 0;
        
          if (buttonStateLeft == BTN_STATE_PRESSED) data[0] |= SEG_G;
          if (buttonStateRight == BTN_STATE_PRESSED) data[3] |= SEG_G;
          if (buttonStateBoth == BTN_STATE_PRESSED) { data[1] |= SEG_G; data[2] |= SEG_G; }
          if (buttonStateLeft == BTN_STATE_HOLD) data[0] |= SEG_D;
          if (buttonStateRight == BTN_STATE_HOLD) data[3] |= SEG_D;
          if (buttonStateBoth == BTN_STATE_HOLD) { data[1] |= SEG_D; data[2] |= SEG_D; }
         
          if (buttonLeft) data[0] |= SEG_A;
          if (buttonRight) data[3] |= SEG_A;
          if (currentTime - submodeStart > 5000) {
            submodeStart = currentTime;
            submode = 2;
          }
          lastSubmode = true;
          break;
        }
        case 2: { // test mode of raw analog values
          uint16_t val = analogRead(0); //P5!
          data[3] = display.encodeDigit(val % 10);
          data[2] = display.encodeDigit((val / 10) % 10);
          data[1] = display.encodeDigit((val / 100) % 10);
          data[0] = display.encodeDigit((val / 1000) % 10);
          if (currentTime - submodeStart > 5000) {
            submodeStart = currentTime;
            submode = 1;
          }
          lastSubmode = true;
          break;
        }
      }
      break;
    }
    case MODE_PEE_INFO :
    case MODE_POO_INFO : {
      switch (submode) {
        case 0: {
          copyText(SEG_LAST);
          lastSubmode = false;
          break;
        }
        case 1: {
          if (mode==MODE_PEE_INFO) {
            copyText(SEG_PEE);
          } else {
            copyText(SEG_POO);
          }
          break;
        }
        case 2: {
          uint8_t pminute = readEEPROM(pmAddress);
          uint8_t phour = readEEPROM(phAddress);
          data[3] = display.encodeDigit(pminute % 10);
          data[2] = display.encodeDigit(pminute / 10);
          data[1] = display.encodeDigit(phour % 10) | SEG_DP;
          data[0] = display.encodeDigit(phour / 10);
          break;
        }
        case 3:
        case 4: {
          break;
        }
        case 5: {
          changeMode(MODE_CLOCK,1);
          break;
        }
      }
      break;
    }
    case MODE_POO_SET :
    case MODE_PEE_SET : {
      switch (submode) {
        case 0: {
          if (mode==MODE_PEE_SET) {
            copyText(SEG_PEE);
            if (buttonStateLeft != BTN_STATE_HOLD) changeMode(MODE_CLOCK);
          } else {
            copyText(SEG_POO);
            if (buttonStateRight != BTN_STATE_HOLD) changeMode(MODE_CLOCK);
          }
          lastSubmode = false;
          break;
        }
        case 1: {
          copyText(SEG_SET);
          writeEEPROM(pmAddress, minute);
          writeEEPROM(phAddress, hour);
          submode ++;
          break;
        }
        case 2: {
          copyText(SEG_SET);
          lastSubmode = true;
          if ((buttonStateLeft == BTN_STATE_CLEAR) && (buttonStateRight == BTN_STATE_CLEAR)) changeMode(MODE_CLOCK);
          break;
        }
      }
      break;
    }
    case MODE_CLOCK_SET : {
      switch (submode) {
        case 0: {
          copyText(SEG_SET);
          lastSubmode = false;
          break;
        }
        case 1: {
          copyText(SEG_UHR);
          if (buttonStateBoth == BTN_STATE_HOLD) submodeStart = currentTime;
          break;
        }
        case 2: {
          data[3] = display.encodeDigit(minute % 10);
          data[2] = display.encodeDigit(minute / 10);
          data[1] = display.encodeDigit(hour % 10) | SEG_DP;
          data[0] = display.encodeDigit(hour / 10);
          if ((currentTime - submodeStart)%1000 > 500) {
            data[1] = SEG_DP;
            data[0] = 0;
          }
          if ((buttonStateLeft == BTN_STATE_PRESSED) || ((buttonStateLeft == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            hour = (hour + 23) % 24;
            submodeStart = currentTime;
          }
          if ((buttonStateRight == BTN_STATE_PRESSED) || ((buttonStateRight == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            hour = (hour + 1) % 24;
            submodeStart = currentTime;
          }
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submode = 3;
//            resetButtons();
          }
          lastSubmode = true;
          break;
        }
        case 3: {
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submodeStart = currentTime;
          } else {
            submode = 4;
          }
          lastSubmode = false;
          break;
        }
        case 4: {
          data[3] = display.encodeDigit(minute % 10);
          data[2] = display.encodeDigit(minute / 10);
          data[1] = display.encodeDigit(hour % 10) | SEG_DP;
          data[0] = display.encodeDigit(hour / 10);
          if ((currentTime - submodeStart)%1000 > 500) {
            data[3] = 0;
            data[2] = 0;
          }
          if ((buttonStateLeft == BTN_STATE_PRESSED) || ((buttonStateLeft == BTN_STATE_HOLD) && (currentTime - submodeStart > 250))) {
            minute = (minute + 61) % 60;
            submodeStart = currentTime;
          }
          if ((buttonStateRight == BTN_STATE_PRESSED) || ((buttonStateRight == BTN_STATE_HOLD) && (currentTime - submodeStart > 250))) {
            minute = (minute + 1) % 24;
            submodeStart = currentTime;
          }
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submode = 10;
            //submode = 5
          }
          lastSubmode = true;
          break;
        }
/*        case 5: {
          copyText(SEG_DAT);
          if (buttonStateBoth == BTN_STATE_HOLD) submodeStart = currentTime;
          lastSubmode = false;
          break;
        }
        case 6: {
          data[3] = display.encodeDigit(month % 10);
          data[2] = display.encodeDigit(month / 10);
          data[1] = display.encodeDigit(dayOfMonth % 10);
          data[0] = display.encodeDigit(dayOfMonth / 10);
          if ((currentTime - submodeStart)%1000 > 500) {
            data[1] = 0;
            data[0] = 0;
          }
          if ((buttonStateLeft == BTN_STATE_PRESSED) || ((buttonStateLeft == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            dayOfMonth = (dayOfMonth + 29) % 31 + 1;
            submodeStart = currentTime;
          }
          if ((buttonStateRight == BTN_STATE_PRESSED) || ((buttonStateRight == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            dayOfMonth = (dayOfMonth + 31) % 31 + 1;
            submodeStart = currentTime;
          }
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submode = 7;
            resetButtons();
          }
          lastSubmode = true;
          break;
        }
        case 7: {
          data[3] = display.encodeDigit(month % 10);
          data[2] = display.encodeDigit(month / 10);
          data[1] = display.encodeDigit(dayOfMonth % 10);
          data[0] = display.encodeDigit(dayOfMonth / 10);
          if ((currentTime - submodeStart)%1000 > 500) {
            data[3] = 0;
            data[2] = 0;
          }
          if ((buttonStateLeft == BTN_STATE_PRESSED) || ((buttonStateLeft == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            month = (month + 10) % 12 + 1;
            submodeStart = currentTime;
          }
          if ((buttonStateRight == BTN_STATE_PRESSED) || ((buttonStateRight == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            month = (month + 12) % 12 + 1;
            submodeStart = currentTime;
          }
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submode = 8;
          }
          lastSubmode = true;
          break;
        }
        case 8: {
          copyText(SEG_JAHR);
          if (buttonStateBoth == BTN_STATE_HOLD) submodeStart = currentTime;
          lastSubmode = false;
          break;
        }
        case 9: {
          data[3] = display.encodeDigit(year % 10);
          data[2] = display.encodeDigit((year / 10) % 10);
          data[1] = display.encodeDigit((year / 100) % 10);
          data[0] = display.encodeDigit((year / 1000) % 10);
          if ((currentTime - submodeStart)%1000 > 500) {
            data[3] = 0;
            data[2] = 0;
            data[1] = 0;
            data[0] = 0;
          }
          if ((buttonStateLeft == BTN_STATE_PRESSED) || ((buttonStateLeft == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            year = (year + 99) % 100 + 2000;
            submodeStart = currentTime;
          }
          if ((buttonStateRight == BTN_STATE_PRESSED) || ((buttonStateRight == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            year = (year + 1) % 100 + 2000;
            submodeStart = currentTime;
          }
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submode = 10;
            resetButtons();
          }
          lastSubmode = true;
          break;
        }*/
        case 10: {
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submodeStart = currentTime;
          } else {  
            setDS3231time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);      
//            resetButtons();
            submode = 11;
          }
          break;
        }
        case 11: {
          copyText(SEG_HELL);
          if ((buttonStateLeft == BTN_STATE_PRESSED) || ((buttonStateLeft == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            displayBrightness = (displayBrightness - 1 + (DISPLAY_MAX_BRIGHTNESS - DISPLAY_MIN_BRIGHTNESS)) % (DISPLAY_MAX_BRIGHTNESS - DISPLAY_MIN_BRIGHTNESS) + DISPLAY_MIN_BRIGHTNESS;
            submodeStart = currentTime;
          }
          if ((buttonStateRight == BTN_STATE_PRESSED) || ((buttonStateRight == BTN_STATE_HOLD) && (currentTime - submodeStart > 400))) {
            displayBrightness = (displayBrightness + 1) % (DISPLAY_MAX_BRIGHTNESS - DISPLAY_MIN_BRIGHTNESS) + DISPLAY_MIN_BRIGHTNESS;
            submodeStart = currentTime;
          }
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submodeStart = currentTime;
            submode = 12;
          }
          display.setBrightness(displayBrightness);
          lastSubmode = true;
          break;
        }
        case 12: {
          if (buttonStateBoth == BTN_STATE_HOLD) {
            submodeStart = currentTime;
          } else {  
            writeEEPROM(EEPROM_START_ADDRESS + BRIGHTNESS_ADDRESS, displayBrightness);
            changeMode(MODE_CLOCK, 1);
//            resetButtons();
          }
          break;
        }
      }
      break;
    }
  }

}

void loop() {
  readButtons();
  checkButtons();
  handleMode();
  handleSubmode();
  delay(50); // 20Hz Update
  display.setSegments(data);  
}
