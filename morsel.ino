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

#define  SWITCH_PIN      3

#define  INVERT_SWITCH   1     /* Needed when using internal pull-up */

#define  BLIP_DELTA      250   /* ms length difference to qualify for reading */

#define  BLIP_SLICE      1     /* 1/x amount over dash that should be considered
                                * a pause. 1 = Slow day. 4 = Expert */

#define  BLIP_SPACE      2     /* Multiplier, how many pauses are a space */

#define  MAGIC           22    /* To detect if sketch has run before */

#define  DELBOND_TIME    10000 /* Hold the switch for 10s to delete pairing
                                * info */

#define  SWITCH_MAX      2000  /* Don't exceed this value for longest press
                                * when calculating dit vs dah */

#define  BLIP_MAX        10    /* Max number of unresolved semaphores */

int blipTime[BLIP_MAX];        /* The last number of semaphores' "on" time */
byte blipWrite = 0;            /* Pointer to blipTime write position */
byte blipRead = 0;             /* Pointer to blipTime last unresolved position */

byte lastState = 0, lastSpace = 0;
unsigned long lastChange = 0;
unsigned int lastPause = 0;

/* Function prototypes */
void parseBtCmd(char* hidSequence);
char getKey(unsigned int pause);

/* ===== Main setup, loop, supporting functions ===== */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup() {
  ble.begin(0);
  if (EEPROM.read(0) != MAGIC) { /* First run */
    ble.factoryReset();
    ble.sendCommandCheckOK("AT+GAPDEVNAME=morsel");
    ble.sendCommandCheckOK("AT+HWMODELED=DISABLE");
    EEPROM.update(0, MAGIC);
  }
  ble.echo(false);
  ble.sendCommandCheckOK("AT+BleHIDEn=On");
  ble.sendCommandCheckOK("AT+BleKeyboardEn=On");
  ble.sendCommandCheckOK("AT+GAPCONNECTABLE=1");
  ble.sendCommandCheckOK("AT+GAPSTARTADV");
  ble.reset();
  digitalWrite(SWITCH_PIN, INVERT_SWITCH); /* Activate internal pull-up
                                            * resistor if inverting */
}

void loop() {
  byte r;
  if ((r = digitalRead(SWITCH_PIN) ^ INVERT_SWITCH) != lastState) {
    delay(DEBOUNCE);
    lastState ^= 1;
    if (r) { /* Switch pressed */
      lastPause = (unsigned int) (millis() - lastChange);
    } else { /* Switch released */
      blipTime[blipWrite] = (unsigned int) (millis() - lastChange);
      blipWrite = (blipWrite + 1) % BLIP_MAX;
      if (blipTime[blipWrite] > DELBOND_TIME) {
        /* Delete bonding info */
        ble.sendCommandCheckOK("AT+GAPDELBONDS");
      }
    }
    lastChange = millis();
  }
  char key = getKey((unsigned int) (millis() - lastChange));
  if (key) {
    ble.print("AT+BleKeyboard=");
    if (key == '\r') ble.println("\\r");
    else ble.println(key);
    ble.waitForOK();
  }
}

char getKey(unsigned int pause) { /* Search for patterns */
  int minLen = 0x7FFF;
  int maxLen = 0;
  int reference = 0;
  byte blipPtr = 0;
  int semaphore = 1;

  /* Detect dit vs dah */
  blipPtr = blipRead;
  while (1) {
    if (blipTime[blipPtr] < minLen) minLen = blipTime[blipPtr];
    if (blipTime[blipPtr] > maxLen) maxLen = blipTime[blipPtr];
    
    blipPtr = (blipPtr + 1) % BLIP_MAX;
    if (blipPtr == blipRead) break;
  }

  if (maxLen > SWITCH_MAX) maxLen = SWITCH_MAX;

  /* Minimum difference between a short and long press needed to match */
  if (maxLen - minLen > BLIP_DELTA) 
    reference = minLen + (maxLen - minLen) / 2;
    
  if (!reference) return 0; /* Exit if we can't make heads or tails */

  /* Exit if there hasn't been a significant pause */
  if (pause < (maxLen + (maxLen - minLen) / BLIP_SLICE)) return 0;

  if (!lastSpace && pause > (maxLen + (maxLen - minLen) / BLIP_SLICE) * BLIP_SPACE) {
    lastSpace = 1;
    return ' ';
  }
  
  if (blipRead == blipWrite) return 0; /* Exit if there are no new presses */

  /* Find semaphores */
  blipPtr = blipRead;
  while (blipPtr != blipWrite) {
    semaphore <<= 1;
    semaphore |= (blipTime[blipPtr] < reference); /* Dots are 1, dashes are 0 */

    if (refTable[semaphore] && ((blipPtr+1)%BLIP_MAX) == blipWrite) {
      blipRead = blipWrite;
      lastSpace = (refTable[semaphore] == '\r');
      return refTable[semaphore];
    }

    if (semaphore > SEMAPHORE_MAX) {
      blipRead = (blipRead + 1) % BLIP_MAX;
      blipPtr = blipRead;
    }

    blipPtr = (blipPtr + 1) % BLIP_MAX;

  }

  blipRead = blipWrite;

}
