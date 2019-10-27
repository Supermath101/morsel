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

/* Created by Derek Yerger, October 26, 2019 */

/* Dots are 1, dashes are 0, with leading 1. For example 8 = 1000 = --- */

#define SEMAPHORE_MAX    116  /* When to quit */

// for i in `seq 0 127`; do echo -n "0, // $i"; echo "obase=2;$i" | bc | sed -e "s/1/ /" -e "s/0/-/g" -e "s/1/./g"; done

const char refTable[SEMAPHORE_MAX] = { 0, /* With leading 1 */
  0,
  't', // 2  -
  'e', // 3  .
  'm', // 4  --
  'n', // 5  -.
  'a', // 6  .-
  'i', // 7  ..
  'o', // 8  ---
  'g', // 9  --.
  'k', // 10 -.-
  'd', // 11 -..
  'w', // 12 .--
  'r', // 13 .-.
  'u', // 14 ..-
  's', // 15 ...
  0,   // 16 ----
  0,   // 17 ---.
  'q', // 18 --.-
  'z', // 19 --..
  'y', // 20 -.--
  'c', // 21 -.-.
  'x', // 22 -..-
  'b', // 23 -...
  'j', // 24 .---
  'p', // 25 .--.
  '\r', // 26 .-.- newline
  'l', // 27 .-..
  0,   // 28 ..--
  'f', // 29 ..-.
  'v', // 30 ...-
  'h', // 31 ....
  '0', // 32 -----
  '9', // 33 ----.
  0,   // 34 ---.-
  '8', // 35 ---..
  0,   // 36 --.--
  0,   // 37 --.-.
  0,   // 38 --..-
  '7', // 39 --...
  0,   // 40 -.---
  '(', // 41 -.--.
  0,   // 42 -.-.-
  0,   // 43 -.-..
  0,   // 44 -..--
  '/',   // 45 -..-.
  '=', // 46 -...-
  '6', // 47 -....
  '1', // 48 .----
  0,   // 49 .---.
  0,   // 50 .--.-
  0,   // 51 .--..
  0,   // 52 .-.--
  '+',   // 53 .-.-.
  0,   // 54 .-..-
  '&', // 55 .-...
  '2', // 56 ..---
  0,   // 57 ..--.
  0,   // 58 ..-.-
  0,   // 59 ..-..
  '3', // 60 ...--
  0,   // 61 ...-.
  '4', // 62 ....-
  '5', // 63 .....
  0, // 64 ------
  0, // 65 -----.
  0, // 66 ----.-
  0, // 67 ----..
  0, // 68 ---.--
  0, // 69 ---.-.
  0, // 70 ---..-
  ':', // 71 ---...
  0, // 72 --.---
  0, // 73 --.--.
  0, // 74 --.-.-
  0, // 75 --.-..
  ',', // 76 --..--
  0, // 77 --..-.
  0, // 78 --...-
  0, // 79 --....
  0, // 80 -.----
  0, // 81 -.---.
  ')', // 82 -.--.-
  0, // 83 -.--..
  '!', // 84 -.-.--
  0, // 85 -.-.-.
  0, // 86 -.-..-
  0, // 87 -.-...
  0, // 88 -..---
  0, // 89 -..--.
  0, // 90 -..-.-
  0, // 91 -..-..
  0, // 92 -...--
  0, // 93 -...-.
  '-', // 94 -....-
  0, // 95 -.....
  0, // 96 .-----
  '\'', // 97 .----.
  0, // 98 .---.-
  0, // 99 .---..
  0, // 100 .--.--
  '@', // 101 .--.-.
  0, // 102 .--..-
  0, // 103 .--...
  0, // 104 .-.---
  0, // 105 .-.--.
  '.', // 106 .-.-.-
  0, // 107 .-.-..
  0, // 108 .-..--
  '"', // 109 .-..-.
  0, // 110 .-...-
  0, // 111 .-....
  0, // 112 ..----
  0, // 113 ..---.
  0, // 114 ..--.-
  '?', // 115 ..--..

};
