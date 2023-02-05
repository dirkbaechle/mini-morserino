/******************************************************************************************************************************
 *  Software for the Morserino-32 (M32) multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module   ***
 *  Copyright (C) 2018-2021  Willi Kraml, OE1WKL                                                                            ***
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************************************************************/

#ifndef MORSEDEFS_H
#define MORSEDEFS_H

#include "Arduino.h"
#include "ClickButton.h"   // button control library
#include "WiFi.h"
#include <WiFiUdp.h>       // UDP lib for WiFi TRX
#include <AsyncUDP.h>      // UDP lib for WiFi TRX
#include <Preferences.h>   // ESP 32 library for storing things in non-volatile storage
#include <WebServer.h>     // simple web sever
#include <ESPmDNS.h>       // DNS functionality
#include <Update.h>        // update "over the air" (OTA) functionality
#include "FS.h"
#include "SPIFFS.h"

/////// Program Name & Version

const String PROJECTNAME = "mini-Morserino";

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 1

#define BETA false

#define IGNORE_SERIALOUT false
// if IGNORE_SERIALOUT is true, alle DEBUG messages are on serial out, even when Serial Out is active outputting characters from Keyer, Decoder etc

/////// protocol version for Lora - for the time being this is B01
/////// the first version of the CW over LoRA protocol; future versions will be B02, B03, B00 (reserved for future use)

#define CW_TRX_PROTO_VERSION B01


namespace Buttons
{

    // define the buttons for the clickbutton library
    extern ClickButton modeButton;
    extern ClickButton volButton;

    //void setup();
    //void click();
    //void audioLevelAdjust();

}


///////////////////////////////      H A R D W A R E      ///////////////////////////////////////////
//// Here are the definitions for the various hardware-related I/O pins of the ESP32
/////////////////////////////////////////////////////////////////////////////////////////////////////


const int PinResetOled=-1;
const int PinSDA=5;
const int PinSCL=4;


/// where is the encoder?
const int PinCLK=26;                   // Used for generating interrupts using CLK signal - needs external pullup resisitor! 
const int PinDT=25;                    // Used for reading DT signal  - needs external pullup resisitor! 

/// encoder switch (BLACK knob)
const int modeButtonPin = 36;

/// 2nd switch button - toggles between Speed control and Volume control (RED button)
const int volButtonPin = 15;

// external paddle
const int paddleRight = 2;
const int paddleLeft = 14;

//// with the following we define which pins are used as output for the two pwm channels
//// HF output (with varying duticycle and fixed frequency) and LF output (with varying frequency and fixed dutycycle of 50%)
/// are being added with a 2-transistor AND gate to create a tone frequency with variable frequency and volume

const int LF_Pin = 39;    // for the lower (= NF) frequency generation
const int HF_Pin = 16;    // for the HF PWM generation


// SENS_FACTOR is used for auto-calibrating sensitivity of touch paddles (somewhere between 2.0 and 2.5)
#define SENS_FACTOR 2.22

//#define BAND    0x433E6  //you can set band here directly,e.g. 868E6,915E6


///////////////////////////////////////// END OF HARDWARE DEFS ////////////////////////////////////////////////////////////////////


// defines for keyer modi
//

#define    IAMBICA      1          // Curtis Mode A
#define    IAMBICB      2          // Curtis Mode B (with enhanced Curtis timing, set as parameter
#define    ULTIMATIC    3          // Ultimatic mode
#define    NONSQUEEZE   4          // Non-squeeze mode of dual-lever paddles - simulate a single-lever paddle
#define    STRAIGHTKEY  5          // use of a straight key (for echo training etc) - not really a "keyer" mode


enum DISPLAY_TYPE               // how we display in trainer mode
  {
      NO_DISPLAY, DISPLAY_BY_CHAR, DISPLAY_BY_WORD
  };

enum random_OPTIONS             // what we generate
  {
      OPT_ALL, OPT_ALPHA, OPT_NUM, OPT_PUNCT, OPT_PRO, OPT_ALNUM, OPT_NUMPUNCT, OPT_PUNCTPRO, OPT_ALNUMPUNCT, OPT_NUMPUNCTPRO
  };
enum PROMPT_TYPE                // how we prompt in echo trainer mode
  {
      NO_PROMPT, CODE_ONLY, DISP_ONLY, CODE_AND_DISP
  };

enum GEN_TYPE                   // the things we can generate in generator mode
  { 
      RANDOMS, ABBREVS, WORDS, CALLS, MIXED, PLAYER
  };              
//
//enum AutoStopModes 
//  {
//      off, stop1, stop2
//  }; 

enum DECODER_STATES             // state machine for decoding CW
  {
      LOW_, HIGH_, INTERELEMENT_, INTERCHAR_
  };

enum encoderMode                // define modes for state machine of the various modi the encoder can be in
  {
      speedSettingMode, volumeSettingMode, scrollMode 
  }; 

enum morserinoMode              // the states the morserino can be in - selected intop level menu
  {
      morseKeyer, loraTrx, morseTrx, morseGenerator, echoTrainer, morseDecoder, shutDown, measureNF, invalid
  };

enum menuNo 
  {   _dummy, _keyer, _gen, _genRand, _genAbb, _genWords, _genCalls, _genMixed, _genPlayer,
        _echo, _echoRand, _echoAbb, _echoWords, _echoCalls, _echoMixed, _echoPlayer,
        _trx, _trxLora, _trxIcw, _decode, _goToSleep 
  };

enum echoStates 
  {   
      START_ECHO, SEND_WORD, REPEAT_WORD, GET_ANSWER, COMPLETE_ANSWER, EVAL_ANSWER 
  };

enum KEYERSTATES 
  {
      IDLE_STATE, DIT, DAH, KEY_START, KEYED, INTER_ELEMENT 
  };

  

extern void DEBUG (String s);


// we need this for some strange reason: the min definition breaks with WiFi
#define _min(a,b) ((a)<(b)?(a):(b))
#define _max(a,b) ((a)>(b)?(a):(b))

#endif
