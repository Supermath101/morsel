/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>. */

/* Created by Derek Yerger, October 26, 2019
 *
 * The following sketch is for an Adafruit Feather 32u4 Bluefruit LE
 *
 * It interprets morse code entered on a switch and outputs the letters as BLE
 * HID Keyboard commands. An attempt is made to interpret dits vs dahs based
 * on the length of each input received.
 *
 * To delete Bluetooth bonding information, hold the switch for 10 seconds. */

#include <EEPROM.h>
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"
#include "morse.h"

#define  DEBOUNCE        20    /* Milliseconds to debounce switch */

const byte switchPin[2] = { 2, 3 }; /* Pins for dit, dah */

#define  INVERT_SWITCH   1     /* Needed when using internal pull-up */

#define  MAGIC           24    /* To detect if sketch has run before */

#define  MENUFIXED       3     /* Where to start counting tunable entries */

const char *tunablesDesc[] = { "Time between letters (ms)",
                               "Time before inserting space (ms)"};

int tunables[] = { 1000, 2000 };

int *letterTime = &tunables[0];
int *spaceTime = &tunables[1];

int semaphore = 1;             /* Current letter */
int programming = 1;
bool serstatus = 0;

byte lastState[2], lastSpace = 0;
unsigned long lastChange = 0;

/* Function prototypes */
void parseBtCmd(char* hidSequence);
char getKey(unsigned int pause);
void printMenu();
void purge();
int prompt();

void saveValues() { /* Write all tunables to EEPROM */
  int adx = 2;
  int sp;
  for (sp = 0; sp < (sizeof(tunables)/sizeof(int)); sp++) {
    EEPROM.update(adx, tunables[sp] >> 8);
    EEPROM.update(adx+1, tunables[sp]); adx += 2;
  }
}

/* ===== Main setup, loop, supporting functions ===== */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(60000);
  ble.begin(0);
  int adx = 0;
  int val = (EEPROM.read(adx) << 8) + EEPROM.read(adx+1);
  if (val != MAGIC) {
    Serial.println("First run on this chip. Values will be initialized.");
    EEPROM.update(adx, MAGIC >> 8); EEPROM.update(adx+1, MAGIC);
    saveValues();
    ble.factoryReset();
    ble.sendCommandCheckOK("AT+GAPDEVNAME=morsel");
    ble.sendCommandCheckOK("AT+HWMODELED=DISABLE");
  } else {
    int sp;
    adx += 2;
    for (sp = 0; sp < (sizeof(tunables)/sizeof(int)); sp++) {
      tunables[sp] = (EEPROM.read(adx) << 8) + EEPROM.read(adx+1);
      adx += 2;
    }
    Serial.println("Stored values have been loaded.");
  }
  ble.echo(false);
  ble.sendCommandCheckOK("AT+BleHIDEn=On");
  ble.sendCommandCheckOK("AT+BleKeyboardEn=On");
  ble.sendCommandCheckOK("AT+GAPCONNECTABLE=1");
  ble.sendCommandCheckOK("AT+GAPSTARTADV");
  ble.reset();
  for (int s = 0; s < 2; s++)
    digitalWrite(switchPin[s], INVERT_SWITCH); /* Activate internal pull-up
                                                 * resistor if inverting */
}

void loop() {
  if (Serial && !serstatus) {
    serstatus = 1;
    programming = 99;
  } else if (!Serial && serstatus) {
    serstatus = 0;
    programming = -99;
  }
  byte r, s;
  for (s = 0; s < 2; s++) {
    if ((r = digitalRead(switchPin[s]) ^ INVERT_SWITCH) != lastState[s]) {
      delay(DEBOUNCE);
      lastChange = millis();
      lastState[s] ^= 1;
      if (r) { /* Switch pressed */
        semaphore <<= 1;
        semaphore |= s; /* Dits are 1, dahs are 0 */
      }
    }
  }

  if (!lastState[0] && !lastState[1]) {
    char key = getKey((unsigned int) (millis() - lastChange));
    if (key) {
      ble.print("AT+BleKeyboard=");
      if (key == '\r') ble.println("\\r");
      else ble.println(key);
      ble.waitForOK();
      Serial.print(key);
      semaphore = 1;
    }
  }

  if (programming >= 0) { /* Menu mode */
    switch (programming) {
      case 99:
        purge();
        printMenu();
        programming = -99;
        break;
        
      case 1:
        saveValues();
        Serial.print("\033[2J\033[0;0H");
        programming = -1;
        break;
        
      case 2:
        /* Delete bonding info */
        ble.sendCommandCheckOK("AT+GAPDELBONDS");
        Serial.println("Bond deleted.");
        delay(1000);
        programming = 99;
        break;
        
      default:
        purge();
        Serial.print("\r\n Enter new value for '");
        Serial.print(tunablesDesc[programming-MENUFIXED]);
        Serial.print("': ");
        tunables[programming-MENUFIXED] = prompt();
        programming = 99;
        break;      
    }
  }
  if (programming == -99 && Serial.available() > 0) {
    int val = Serial.read() - 48;
    if ((val < 1) || (val > (MENUFIXED+sizeof(tunables)/sizeof(int)))) {
      Serial.println("\r\n Invalid choice.");
      delay(1000);
      printMenu();
    } else programming = val;
  }
  if (programming == -1 && Serial.available() > 0) {
    programming = 99;
  }
}

char getKey(unsigned int pause) { /* Search for patterns */
  
  /* Exit if there hasn't been a significant pause */
  if (pause < *letterTime) return 0;

  if (!lastSpace && pause > *spaceTime) {
    lastSpace = 1;
    return ' ';
  }
  
  /* Find semaphores */
  if (semaphore > SEMAPHORE_MAX) { /* Invalid */
    semaphore = 1;
    return 0;
  }

  if (refTable[semaphore]) { /* Found match */
    lastSpace = (refTable[semaphore] == '\r');
    return refTable[semaphore];
  }

  return 0;
}

void printMenu() {
  Serial.println("\033[2J\033[0;0H");
  Serial.println(" \033[32;1mmorsel\033[34m\r\n BLE Morse code converter\033[0m\r\n");
  Serial.println(" \033[1m1.\033[0m Save values and show output\r\n");
  Serial.println(" \033[1m2.\033[0m Delete Bluetooth bond\r\n");
  Serial.println(" \033[1mTunable items:\033[0m"); 
  int ch = MENUFIXED;
  int sp;
  for (sp = 0; sp < (sizeof(tunables)/sizeof(int)); sp++) {
    Serial.print(" \033[1m");
    Serial.write((ch++) + 48);
    Serial.print(".\033[0m ");
    Serial.print(tunablesDesc[sp]);
    Serial.print(" = ");
    Serial.println(tunables[sp]);
  }
  Serial.print("\n Enter a choice: \r\n\r\n");
}

void purge() {
  while (Serial.available() > 0)
    int val = Serial.read();
}

int prompt() {
  purge();
  Serial.print("\r\n Enter a new value: ");
  return Serial.parseInt();
}
