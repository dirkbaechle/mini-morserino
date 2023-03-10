/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018-2020  Willi Kraml, OE1WKL                                                                                 ***
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

#include <Preferences.h>   // ESP 32 library for storing things in non-volatile storage
#include "MorseOutput.h"
#include "MorsePreferences.h"
#include "abbrev.h"
#include "english_words.h"
#include "ClickButton.h"   // button control library

using namespace MorsePreferences;

Preferences pref;               // use the Preferences library for storing and retrieving objects

  // the preferences variable and their defaults

  uint8_t MorsePreferences::version_major = VERSION_MAJOR;
  uint8_t MorsePreferences::version_minor = VERSION_MINOR;
  uint8_t MorsePreferences::sidetoneFreq = 11;               // side tone frequency                               1 - 15
  uint8_t MorsePreferences::sidetoneVolume = 16;              // side tone volume, as a value between 0 and 19   0 -19
  boolean MorsePreferences::didah = false;                    // paddle polarity                                  bool
  uint8_t MorsePreferences::keyermode = 2;                    // Iambic keyer mode: see the #defines in morsedefs.h
  uint8_t MorsePreferences::interCharSpace = 3;               // trainer: in dit lengths                          3 - 24
  boolean MorsePreferences::reversePolarity = false;             // has now a different meaning: true when we need to reverse the polarity of the ext paddle
  uint8_t MorsePreferences::ACSlength = 0;                    // in ACS: we extend the pause between charcaters to the equal length of how many dots 
                                              // (2, 3 or 4 are meaningful, 0 means off) 0, 2-4
  boolean MorsePreferences::encoderClicks = true;             // all: should rotating the encoder generate a click?
  uint8_t MorsePreferences::randomLength = 3;                 // trainer: how many random chars in one group -    1 -  5
  uint8_t MorsePreferences::randomOption = 0;                 // trainer: from which pool are we generating random characters?  0 - 9
  uint8_t MorsePreferences::callLength = 0;                   // trainer: max length of call signs generated (0 = unlimited)    0, 3 - 6
  uint8_t MorsePreferences::abbrevLength = 0;                 // trainer: max length of abbreviations generated (0 = unlimited) 0, 2 - 6
  uint8_t MorsePreferences::wordLength = 0;                   // trainer: max length of english words generated (0 = unlimited) 0, 2 - 6
  uint8_t MorsePreferences::trainerDisplay = DISPLAY_BY_CHAR; // trainer: how we display what the trainer generates: nothing, by character, or by word  0 - 2
  uint8_t MorsePreferences::curtisBTiming = 45;               // keyer: timing for enhanced Curtis mode: dah                    0 - 100
  uint8_t MorsePreferences::curtisBDotTiming = 75 ;           // keyer: timing for enhanced Curtis mode: dit                    0 - 100
  uint8_t MorsePreferences::interWordSpace = 7;               // trainer: normal interword spacing in lengths of dit,           6 - 45 ; default = norm = 7

  uint8_t MorsePreferences::echoRepeats = 3;                  // how often will echo trainer repeat an erroniously entered word? 0 - 7, 7=forever, default = 3
  uint8_t MorsePreferences::echoDisplay = 1;                  //  1 = CODE_ONLY 2 = DISP_ONLY 3 = CODE_AND_DISP
  boolean MorsePreferences::wordDoubler = false;              // in CW trainer mode only, repeat each word
  uint8_t MorsePreferences::echoToneShift = 1;                // 0 = no shift, 1 = up, 2 = down (a half tone)                   0 - 2
  boolean MorsePreferences::echoConf = true;                  // true if echo trainer confirms audibly too, not just visually
  uint8_t MorsePreferences::keyTrainerMode = 1;               // key a transmitter in generator and player mode?
                                              //  0: "Never";  1: "CW Keyer only";  2: "Keyer&Generator"; 3: Keyer, generator and Wifi / Internet RX
  uint8_t MorsePreferences::wifiTrainerMode = 0;              // transmit via WiFi in generator and player mode?
                                              //  0: "No";  1: "WiFi"
  boolean MorsePreferences::speedAdapt = false;               //  true: in echo modes, increase speed when OK, reduce when not ok     
  uint8_t MorsePreferences::latency = 5;                      //  time span after currently sent element during which paddles are not checked; in 1/8th of dit length; stored as 1 -  8  
  uint8_t MorsePreferences::randomFile = 0;                   // if 0, play file word by word; if 255, skip random number of words (0 - 255) between reads   
  
  boolean MorsePreferences::extAudioOnDecode = false;         // send decoded audio also to external audio  I/O port
  uint8_t MorsePreferences::timeOut = 1;                      // time-out value: 4 = no timeout, 1 = 5 min, 2 = 10 min, 3 = 15 min
  boolean MorsePreferences::quickStart = false;               // should we start the last executed command immediately?
  boolean MorsePreferences::autoStopMode = false;                 // If to stop after each word in generator modes
  uint8_t MorsePreferences::maxSequence = 0;                  // max # of words generated before the Morserino pauses

  uint8_t MorsePreferences::serialOut = 7;                    // shall we output characters on USB serial? 0 = none (but DEBUG/ERR) 1 = keyer, 2 = decoder, 3 = both, 7 = everything

  ///// stored in preferences, but not adjustable through preferences menu:
  uint8_t MorsePreferences::responsePause = 5;                // in echoTrainer mode, how long do we wait for response? in interWordSpaces; 2-12, default 5
  uint8_t MorsePreferences::wpm = 15;                         // keyer speed in words per minute                  5 - 60
  uint8_t MorsePreferences::menuPtr = 1;                      // current position of menu
  String  MorsePreferences::wlanSSID = "";                    // SSID for connecting to the Internet
  String  MorsePreferences::wlanPassword = "";                // password for connecting to WiFi router
  String  MorsePreferences::wlanTRXPeer = "";                 // peer Morserino for WiFI TRX
  String  MorsePreferences::wlanSSID1 = "";                    // SSID for connecting to the Internet
  String  MorsePreferences::wlanPassword1 = "";                // password for connecting to WiFi router
  String  MorsePreferences::wlanTRXPeer1 = "";                 // peer Morserino for WiFI TRX
  String  MorsePreferences::wlanSSID2 = "";                    // SSID for connecting to the Internet
  String  MorsePreferences::wlanPassword2 = "";                // password for connecting to WiFi router
  String  MorsePreferences::wlanTRXPeer2 = "";                 // peer Morserino for WiFI TRX
  String  MorsePreferences::wlanSSID3 = "";                    // SSID for connecting to the Internet
  String  MorsePreferences::wlanPassword3 = "";                // password for connecting to WiFi router
  String  MorsePreferences::wlanTRXPeer3 = "";                 // peer Morserino for WiFI TRX

  uint32_t MorsePreferences::fileWordPointer = 0;             // remember how far we have read the file in player mode / reset when loading new file         
  uint8_t MorsePreferences::promptPause = 2;                  // in echoTrainer mode, length of pause before we send next word; multiplied by interWordSpace
  uint8_t MorsePreferences::tLeft = 20;                       // threshold for left paddle
  uint8_t MorsePreferences::tRight = 20;                      // threshold for right paddle

  uint8_t MorsePreferences::snapShots = 0;                    // keep track which snapshots are being used ( 0 .. 7, called 1 to 8)

  uint8_t MorsePreferences::oledBrightness = 255;

//////// end of variables stored in preferences

//// temporary buffer for conversions, local to this file
char numBuffer[16];


/// variables for managing snapshots
uint8_t memories[8];
uint8_t memCounter;
uint8_t memPtr = 0;


//// for adjusting preferences

#define SizeOfArray(x)       (sizeof(x) / sizeof(x[0]))

const String MorsePreferences::prefOption[] = { "Encoder Click", "Tone Pitch Hz", "External Pol.", "Paddle Polar.", 
                              "Keyer Mode   ", "CurtisB DahT%", "CurtisB DitT%", "AutoChar Spce", 
                              "Tone Shift   ", "InterWord Spc", "InterChar Spc", "Random Groups", 
                              "Length Rnd Gr", "Length Calls ", "Length Abbrev", "Length Words ", 
                              "CW Gen Displ ", "Each Word 2x ", "Echo Prompt  ", "Echo Repeats ", "Confrm. Tone ", 
                              "Key ext TX   ", "Generator Tx ", "Bandwidth    ", "Adaptv. Speed", 
                              "Latency      ", "Randomize File", "Decoded on I/O",
                              "Time Out     ", "Quick Start  ", "Stop/Next/Rep", "Max # of Words", "Serial Output", 
                              "RECALLSnapshot", "STORE Snapshot",
                              "Calibrate Batt", "Hardware Conf"};                   
  prefPos MorsePreferences::keyerOptions[] =      {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS, 
                                    posKeyTrainerMode, posTimeOut, posQuickStart, posSerialOut };
  prefPos MorsePreferences::generatorOptions[] =  {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posRandomOption, 
                                    posRandomLength, posCallLength, posAbbrevLength, posWordLength, posMaxSequence, posAutoStop, 
                                    posTrainerDisplay, posWordDoubler, posKeyTrainerMode, posWifiTrainerMode, posTimeOut, posQuickStart, posSerialOut };
 prefPos MorsePreferences::playerOptions[] =     {posClicks, posPitch, posExtPaddles, posInterWordSpace, posInterCharSpace, posMaxSequence, posAutoStop, posTrainerDisplay, 
                                     posRandomFile, posWordDoubler, posKeyTrainerMode, posWifiTrainerMode, posTimeOut, posQuickStart,  posSerialOut };
 prefPos MorsePreferences::echoPlayerOptions[] = {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posInterWordSpace, posInterCharSpace, posMaxSequence, posRandomFile, posEchoRepeats,  posEchoDisplay, posEchoConf, posTimeOut, 
                                    posQuickStart, posSerialOut};
 prefPos MorsePreferences::echoTrainerOptions[]= {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomOption, 
                                    posRandomLength, posCallLength, posAbbrevLength, posWordLength, posMaxSequence, posEchoRepeats,  posEchoDisplay, posEchoConf, posSpeedAdapt, posTimeOut, 
                                    posQuickStart, posSerialOut };
 prefPos MorsePreferences::wifiTrxOptions[] =    {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posTrainerDisplay, posKeyTrainerMode, posExtAudioOnDecode, posTimeOut, posQuickStart, posSerialOut };
 prefPos MorsePreferences::extTrxOptions[] =     {posClicks, posPitch, posExtPaddles, posPolarity, posLatency, posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS,
                                    posEchoToneShift, posExtAudioOnDecode, posTimeOut, posQuickStart, posSerialOut };
 prefPos MorsePreferences::decoderOptions[] =    {posClicks, posPitch, posCurtisMode, posExtAudioOnDecode, posTimeOut, posQuickStart, posSerialOut };

 prefPos MorsePreferences::allOptions[] =        { posClicks, posPitch, posExtPaddles, posPolarity, posLatency,
                                    posCurtisMode, posCurtisBDahTiming, posCurtisBDotTiming, posACS, 
                                    posEchoToneShift, posInterWordSpace, posInterCharSpace, posRandomOption, 
                                    posRandomLength, posCallLength, posAbbrevLength, posWordLength, posMaxSequence, posAutoStop, 
                                    posTrainerDisplay, posRandomFile, posWordDoubler, posEchoRepeats, posEchoDisplay, posEchoConf, 
                                    posKeyTrainerMode, posWifiTrainerMode, posSpeedAdapt, 
                                    posExtAudioOnDecode, posTimeOut, posQuickStart, posSerialOut};

prefPos *MorsePreferences::currentOptions = MorsePreferences::allOptions;

int MorsePreferences::keyerOptionsSize = SizeOfArray(MorsePreferences::keyerOptions);
int MorsePreferences::generatorOptionsSize = SizeOfArray(MorsePreferences::generatorOptions);
int MorsePreferences::playerOptionsSize = SizeOfArray(MorsePreferences::playerOptions);
int MorsePreferences::echoPlayerOptionsSize = SizeOfArray(MorsePreferences::echoPlayerOptions);
int MorsePreferences::echoTrainerOptionsSize = SizeOfArray(MorsePreferences::echoTrainerOptions);
int MorsePreferences::wifiTrxOptionsSize = SizeOfArray(MorsePreferences::wifiTrxOptions);
int MorsePreferences::extTrxOptionsSize = SizeOfArray(MorsePreferences::extTrxOptions);
int MorsePreferences::decoderOptionsSize = SizeOfArray(MorsePreferences::decoderOptions);
int MorsePreferences::allOptionsSize = SizeOfArray(MorsePreferences::allOptions);


int currentOptionSize;

////// setup preferences ///////

             
boolean MorsePreferences::setupPreferences(uint8_t atMenu) {
  // enum morserinoMode {morseKeyer, wifiTrx, morseGenerator, echoTrainer, shutDown, morseDecoder, invalid };
  static int oldPos = 1;
  int t;

  int ptrIndex, ptrMax;
  prefPos posPtr;
 
  ptrMax = currentOptionSize;

  ///// we should check here if the old ptr (oldIndex) is contained in the current preferences collection (currentOptions)
  ptrIndex = 1;
  for (int i = 0; i < ptrMax; ++i) {
      if (currentOptions[i] == oldPos) {
          ptrIndex = i;
          break;
      }
  }
  posPtr = currentOptions[ptrIndex];  
  keyOut(false, true, 0, 0);                // turn the LED off, unkey transmitter, or whatever; just in case....
  keyOut(false,false, 0, 0);  
  displayKeyerPreferencesMenu(posPtr);
  MorseOutput::printOnScroll(2, REGULAR, 0,  " ");

  while (true) {                            // we wait for single click = selection or long click = exit - or single or long click or RED button
        Buttons::modeButton.Update();
        switch (Buttons::modeButton.clicks) {            // button was clicked
          case 1:     // change the option corresponding to pos
                      if (adjustKeyerPreference(posPtr))
                         goto exitFromHere;
                      break;
          case -1:    //////// long press indicates we are done with setting preferences - check if we need to store some of the preferences
          exitFromHere: return false;
                        break;
          }

          Buttons:: volButton.Update();                 // RED button
          switch (Buttons:: volButton.clicks) {         // was clicked
            case 1:     // recall snapshot
                        if (MorsePreferences::recallSnapshot())
                          writePreferences("morserino");
                        //delay(100);
                        return true;
                        break;
            case 2:     MorseOutput::decreaseBrightness();
                        displayKeyerPreferencesMenu(posPtr);
                        break;
            case -1:    //store snapshot
                        
                        if (MorsePreferences::storeSnapshot(atMenu))
                          writePreferences("morserino");
                        while(Buttons:: volButton.clicks)
                          Buttons:: volButton.Update();
                        return false;
                        break;
          }

          
          //// display the value of the preference in question

         if ((t=checkEncoder())) {
            MorseOutput::pwmClick(MorsePreferences::sidetoneVolume);         /// click 
            ptrIndex = (ptrIndex +ptrMax + t) % ptrMax;
            //DEBUG("ptrIndex: " + String(ptrIndex));
            //DEBUG("ptrMax: " + String(ptrMax));
            posPtr = currentOptions[ptrIndex];
            //oldIndex = ptrIndex;                                                              // remember menu position
            oldPos = posPtr;
            
            displayKeyerPreferencesMenu(posPtr);
            //MorseOutput::printOnScroll(1, BOLD, 0, ">");
            MorseOutput::printOnScroll(2, REGULAR, 0, " ");

            Heltec.display -> display();                                                        // update the display   
         }    // end if (encoderPos)
         checkShutDown(false);         // check for time out
  } // end while - we leave as soon as the button has been pressed long
}   // end function setupKeyerPreferences()


//////// Display the preferences menu - we display the following preferences

void MorsePreferences::displayKeyerPreferencesMenu(int pos) {
  MorseOutput::clearDisplay();
  if (pos < posSnapRecall)
    MorseOutput::printOnStatusLine( true, 0,  "Set Preferences: ");
    MorseOutput::printOnStatusLine( true, 0,  "Manage Snapshots:");
  MorseOutput::printOnScroll(1, BOLD, 0, prefOption[pos]);
  
  switch (pos) {
     case  posCurtisMode:  internal::displayCurtisMode();
                          break;
     case  posCurtisBDahTiming:  internal::displayCurtisBTiming();
                          break;
     case  posCurtisBDotTiming:  internal::displayCurtisBDotTiming();
                          break;
     case  posACS:  internal::displayACS();
                          break;
    case  posPolarity:  internal::displayPolarity();
                          break;
    case posLatency:    internal::displayLatency();
                          break;
    case  posExtPaddles:  internal::displayExtPaddles();
                          break;
    case  posPitch:  internal::displayPitch();
                          break;
    case  posClicks:  internal::displayClicks();
                          break;
    case posKeyTrainerMode:  internal::displayKeyTrainerMode();
                          break;
    case  posInterWordSpace:  internal::displayInterWordSpace(); 
                          break; 
    case  posInterCharSpace:  internal::displayInterCharSpace();
                          break;
    case  posRandomOption:  internal::displayRandomOption();
                          break;
    case posRandomLength:  internal::displayRandomLength();
                          break;
    case posCallLength:  internal::displayCallLength();
                          break;
    case posAbbrevLength:  internal::displayAbbrevLength();
                          break;
    case posWordLength:  internal::displayWordLength();
                          break;
    case posTrainerDisplay:  internal::displayTrainerDisplay();
                          break;
    case posEchoDisplay:  internal::displayEchoDisplay();
                          break;
    case posEchoRepeats:  internal::displayEchoRepeats();
                          break;
    case posEchoConf:     internal::displayEchoConf();
                          break;
    case posWordDoubler:  internal::displayWordDoubler();
                          break;
    case posEchoToneShift:internal::displayEchoToneShift();
                          break;
    case posWifiTrainerMode:  internal::displayWifiTrainerMode();
                          break;
    case posSpeedAdapt:   internal::displaySpeedAdapt();
                          break;
    case posRandomFile:   internal::displayRandomFile();
                          break;
    case posExtAudioOnDecode:
                          internal::displayExtAudioOnDecode();
                          break;
    case posTimeOut:      internal::displayTimeOut();
                          break;
    case posQuickStart:   internal::displayQuickStart();
                          break;
    case posAutoStop:     internal::displayAutoStop();
                          break;
     case posSnapRecall: internal::displaySnapRecall();
                          break;
     case posSnapStore: internal::displaySnapStore();
                          break;  
     case posMaxSequence: internal::displayMaxSequence();
                          break; 
     case posSerialOut:   internal::displaySerialOut();
                          break;
  } /// switch (pos)
  Heltec.display -> display();
} // displayKeyerPreferences()


/// now follow all the menu displays


//    IAMBICA      1          // Curtis Mode A
//    IAMBICB      2          // Curtis Mode B (with enhanced Curtis timing, set as parameter
//    ULTIMATIC    3          // Ultimatic mode
//    NONSQUEEZE   4          // Non-squeeze mode of dual-lever paddles - simulate a single-lever paddle
//    STRAIGHTKEY  5          // use of a straight key (for echo training etc) - not really a "keyer" mode

void internal::displayCurtisMode() {
  String keyerModus[] = {"Curtis A    ", 
                         "Curtis B    ", 
                         "Ultimatic   ",
                         "Non-Squeeze ",
                         "Straight Key" };
  MorseOutput::printOnScroll(2, REGULAR, 1, keyerModus[MorsePreferences::keyermode-1]);
}   

void internal::displayCurtisBTiming() {
  // display start timing when paddles are being checked in Curtis B mode during dah: between 0 and 100
  sprintf(numBuffer, "%3i", MorsePreferences::curtisBTiming);
  MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
}

void internal::displayCurtisBDotTiming() {
  // display start timing when paddles are being checked in Curtis B modeduring dit : between 0 and 100
  sprintf(numBuffer, "%3i", MorsePreferences::curtisBDotTiming);
  MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
}

void internal::displayACS() {
  String ACSmode[] = {"Off         ",
                      "Invalid     ",
                      "min. 2 dots ",
                      "min. 3 dots ",
                      "min. 4 dots "};
    MorseOutput::printOnScroll(2, REGULAR, 1, ACSmode[MorsePreferences::ACSlength]);                  
}

void internal::displayPitch() {
  sprintf(numBuffer, "%3i", MorseOutput::notes[MorsePreferences::sidetoneFreq]);
  MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
}

void internal::displayClicks() {
     MorseOutput::printOnScroll(2, REGULAR, 1,  MorsePreferences::encoderClicks ? "On " :
                                                          "Off" ); 
}

void internal::displayExtPaddles() {
     MorseOutput::printOnScroll(2, REGULAR, 1,  MorsePreferences::reversePolarity ? "Reversed    " :
                                                    "Normal      " ); 
}

void internal::displayPolarity() {
      MorseOutput::printOnScroll(2, REGULAR, 1, MorsePreferences::didah ? ".- di-dah  " :
                                                   "-. dah-dit " ); 
}

void internal::displayLatency() {
      sprintf(numBuffer, "%1i/8 of dit", MorsePreferences::latency - 1);
      MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
}
void internal::displayInterWordSpace() {
  // display interword space in ditlengths 
  sprintf(numBuffer, "%2i", MorsePreferences::interWordSpace);
  MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
}

void internal::displayInterCharSpace() {
  // display intercharacter space in ditlengths
  sprintf(numBuffer, "%2i", MorsePreferences::interCharSpace);
  MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
}


void internal::displayRandomOption() {
  String texts[] = {"All Chars   ",
                    "Alpha       ", 
                    "Numerals    ", 
                    "Interpunct. ", 
                    "Pro Signs   ",
                    "Alpha + Num ",
                    "Num+Interp. ",
                    "Interp+ProSn",
                    "Alph+Num+Int",
                    "Num+Int+ProS"
                     };
  MorseOutput::printOnScroll(2, REGULAR, 1, texts[MorsePreferences::randomOption]);
}

void internal::displayRandomLength() {
  // display length of random character groups - 2 - 6
  if (MorsePreferences::randomLength <= 6) 
    sprintf(numBuffer, "%1i     ", MorsePreferences::randomLength);  
  else 
    sprintf(numBuffer, "2 to %1i", MorsePreferences::randomLength -4);
  MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
}


void internal::displayCallLength() {
  // display length of calls - 3 - 6, 0 = all
  if (MorsePreferences::callLength == 0)
       MorseOutput::printOnScroll(2, REGULAR, 1, "Unlimited");
  else {
    sprintf(numBuffer, "max. %1i   ", MorsePreferences::callLength);
    MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
  }
}


void internal::displayAbbrevLength() {
  // display length of abbrev - 2 - 6, 0 = all
  if (MorsePreferences::abbrevLength == 0)
       MorseOutput::printOnScroll(2, REGULAR, 1, "Unlimited");
  else {
    sprintf(numBuffer, "max. %1i    ", MorsePreferences::abbrevLength);
    MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
  }
}


void internal::displayWordLength() {
  // display length of english words - 2 - 6, 0 = all
  if (MorsePreferences::wordLength == 0)
       MorseOutput::printOnScroll(2, REGULAR, 1, "Unlimited");
  else {
    sprintf(numBuffer, "max. %1i     ", MorsePreferences::wordLength);
    MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
  }
}


void internal::displayMaxSequence() {
  // display max # of words; 0 = no limit, 5, 10, 15, 20... 250; 255 = no limit
  if ((MorsePreferences::maxSequence == 0) || (MorsePreferences::maxSequence == 255))
      MorseOutput::printOnScroll(2, REGULAR, 1, "Unlimited");
  else {
      sprintf(numBuffer, "%3i      ", MorsePreferences::maxSequence);
      MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
  }
}


void internal::displayTrainerDisplay() {
  switch (MorsePreferences::trainerDisplay) {
    case  NO_DISPLAY:       MorseOutput::printOnScroll(2, REGULAR, 1, "Display off ");
                            break;
    case  DISPLAY_BY_CHAR:  MorseOutput::printOnScroll(2, REGULAR, 1, "Char by char");
                            break;
    case  DISPLAY_BY_WORD:  MorseOutput::printOnScroll(2, REGULAR, 1, "Word by word");
                            break;
  }
}

void internal::displayEchoDisplay() {
  switch (MorsePreferences::echoDisplay) {
    case CODE_ONLY:         MorseOutput::printOnScroll(2, REGULAR, 1, "Sound only  ");
                            break;
    case DISP_ONLY:         MorseOutput::printOnScroll(2, REGULAR, 1, "Display only");
                            break;
    case CODE_AND_DISP:     MorseOutput::printOnScroll(2, REGULAR, 1, "Sound & Disp");
                            break;
    
  }
}
void internal::displayKeyTrainerMode() {
  String option;
  switch(MorsePreferences::keyTrainerMode) {
    case 0: option = "Never        ";
            break;
    case 1: option = "CW Keyer only";
            break;
    case 2: option = "Keyer & Gen. ";
            break;
    case 3: option = "Keyer&Gen.&RX";
            break;
  }
  MorseOutput::printOnScroll(2, REGULAR, 1, option);
}

void internal::displayWifiTrainerMode() {
    String option;
  switch(MorsePreferences::wifiTrainerMode) {
    case 0: option = "Tx OFF       ";
            break;
    case 1: option = "WiFi Tx ON   ";
            break;
  }
  MorseOutput::printOnScroll(2, REGULAR, 1, option);
}

void internal::displayEchoRepeats() {
  if (MorsePreferences::echoRepeats < 7) {
      sprintf(numBuffer, "%i      ", MorsePreferences::echoRepeats);
      MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
  } else
  MorseOutput::printOnScroll(2, REGULAR, 1, "Forever");
}

void internal::displayEchoToneShift() {
  String option;
  switch (MorsePreferences::echoToneShift) {
    case 0: option = "No Tone Shift";
            break;
    case 1: option = "Up 1/2 Tone  ";
            break;
    case 2: option = "Down 1/2 Tone";
  }
  MorseOutput::printOnScroll(2, REGULAR, 1, option);
}

void internal::displayEchoConf() {
   MorseOutput::printOnScroll(2, REGULAR, 1,  MorsePreferences::echoConf ? "On " :
                                              "Off" ); 
}

void internal::displayWordDoubler() {
  MorseOutput::printOnScroll(2, REGULAR, 1,  MorsePreferences::wordDoubler ? "On  " :
                                                "Off " ); 
}

void internal::displayRandomFile() {
    MorseOutput::printOnScroll(2, REGULAR, 1,  MorsePreferences::randomFile ? "On  " :
                                                "Off " ); 
}

void internal::displaySpeedAdapt() {
      MorseOutput::printOnScroll(2, REGULAR, 1, MorsePreferences::speedAdapt ? "ON         " :
                                                  "OFF        " ); 
}

void internal::displayExtAudioOnDecode() {
      MorseOutput::printOnScroll(2, REGULAR, 1,  MorsePreferences::extAudioOnDecode ? "On  " :
                                                "Off " ); 
}


void internal::displayTimeOut() {
    String TOValue;
    
    switch (MorsePreferences::timeOut) {
          case 1: TOValue     = " 5 min    ";
                  break;
          case 2: TOValue     = "10 min    ";
                  break;
          case 3: TOValue     = "15 min    ";
                  break;
          case 4: TOValue     = "No timeout";
                  break;
      }
      MorseOutput::printOnScroll(2, REGULAR, 1, TOValue);
}

void internal::displayQuickStart() {
      MorseOutput::printOnScroll(2, REGULAR, 1, MorsePreferences::quickStart ? "ON         " :
                                                  "OFF        " ); 
}

void internal::displayAutoStop() {
      MorseOutput::printOnScroll(2, REGULAR, 1, MorsePreferences::autoStopMode ? "ON         " :
                                                  "OFF        " ); 
}

void internal::displaySerialOut() {
      String option;
      switch (MorsePreferences::serialOut) {
          case 0: option = "ERRORS only  ";
                  break;
          case 1: option = "Keyer        ";
                  break;
          case 2: option = "Decoder      ";
                  break;
          case 3: option = "Keyer+Decoder";
                  break;
          case 7: option = "Everything   ";
                  break;
      }
      MorseOutput::printOnScroll(2, REGULAR, 1, option);
}

void internal::displaySnapRecall() {
  if (memCounter) {
    if (memPtr == memCounter)
      MorseOutput::printOnScroll(2, REGULAR, 1, "Cancel Recall");
    else {
      sprintf(numBuffer, "Snapshot %d   ", memories[memPtr] +1);
      MorseOutput::printOnScroll(2, REGULAR, 1, numBuffer);
    }
  }
  else
    MorseOutput::printOnScroll(2, REGULAR, 1, "NO SNAPSHOTS"); 
}

void internal::displaySnapStore() {
  uint8_t mask = 1;
  mask = mask << memPtr;
  if (memPtr == 8)
    MorseOutput::printOnScroll(2, REGULAR, 1, "Cancel Store");
  else {
    sprintf(numBuffer, "Snapshot %d  ", memPtr+1);
    MorseOutput::printOnScroll(2, MorsePreferences::snapShots & mask ? BOLD : REGULAR, 1, numBuffer);
  }
}

//// function to addjust the selected preference

boolean MorsePreferences::adjustKeyerPreference(prefPos pos) {        /// rotating the encoder changes the value, click returns to preferences menu
     //MorseOutput::printOnScroll(1, REGULAR, 0, " ");       /// returns true when a long button press ended it, and false when there was a short click
     MorseOutput::printOnScroll(2, INVERSE_BOLD, 0, ">");
    uint8_t seq;
    int t;
    while (true) {                            // we wait for single click = selection or long click = exit
        pinMode(modeButtonPin, INPUT);

        Buttons::modeButton.Update();
        switch (Buttons::modeButton.clicks) {
          case -1 : //delay(200);
                    return true;
                    break;
          case  1 : //MorseOutput::printOnScroll(1, BOLD, 0,  ">");
                    MorseOutput::printOnScroll(2, REGULAR, 0,  " ");
                    return false;
        }
        if (pos == posSnapRecall) {         // here we can delete a memory....
          Buttons:: volButton.Update();
          if (Buttons:: volButton.clicks) {
            if (memCounter)
              clearMemory(memPtr);
            return true;
          }
        }
        if ((t=checkEncoder())) {
            MorseOutput::pwmClick(MorsePreferences::sidetoneVolume);         /// click 
            switch (pos) {
                case  posCurtisMode : MorsePreferences::keyermode = (MorsePreferences::keyermode + t);                        // set the curtis mode
                                      MorsePreferences::keyermode = constrain(MorsePreferences::keyermode, 1, 5);
                                      internal::displayCurtisMode();                                    // display curtis mode
                                      break;
                case  posCurtisBDahTiming : MorsePreferences::curtisBTiming += (t * 5);                          // Curtis B timing dah (enhanced Curtis mode)
                                      MorsePreferences::curtisBTiming = constrain(MorsePreferences::curtisBTiming, 0, 100);
                                      internal::displayCurtisBTiming();
                                      break;
                case  posCurtisBDotTiming : MorsePreferences::curtisBDotTiming += (t * 5);                   // Curtis B timing dit (enhanced Curtis mode)
                                      MorsePreferences::curtisBDotTiming = constrain(MorsePreferences::curtisBDotTiming, 0, 100);
                                      internal::displayCurtisBDotTiming();
                                      break;
                case  posACS : MorsePreferences::ACSlength += (t+1);                       // ACS
                                if (MorsePreferences::ACSlength == 2)
                                  MorsePreferences::ACSlength +=  t;
                                MorsePreferences::ACSlength = constrain(MorsePreferences::ACSlength-1, 0, 4);
                                internal::displayACS();
                                break;
                case  posPitch : MorsePreferences::sidetoneFreq += t;                             // sidetone pitch
                                MorsePreferences::sidetoneFreq = constrain(MorsePreferences::sidetoneFreq, 1, 15)  ;
                                internal::displayPitch();
                                break;
                case  posClicks : MorsePreferences::encoderClicks = !MorsePreferences::encoderClicks;
                                internal::displayClicks();
                                break;
                case  posExtPaddles : MorsePreferences::reversePolarity = !MorsePreferences::reversePolarity;                           // ext paddle on/off
                                internal::displayExtPaddles();
                                break;
                case  posPolarity: MorsePreferences::didah = !MorsePreferences::didah;                                            // polarity
                                internal::displayPolarity();
                                break;
                case posLatency:  MorsePreferences::latency += t;
                                  MorsePreferences::latency = constrain(MorsePreferences::latency, 1, 8);
                                  internal::displayLatency();
                                  break;
                case  posKeyTrainerMode:  MorsePreferences::keyTrainerMode += (t+1);                     // Key TRX: 0=never, 1= keyer only, 2 = keyer & trainer, 3 keyer&trainer&RX
                                MorsePreferences::keyTrainerMode = constrain(MorsePreferences::keyTrainerMode-1, 0, 3);
                                internal::displayKeyTrainerMode();
                                break; 
                case  posInterWordSpace : MorsePreferences::interWordSpace += t;                         // interword space in lengths of dit
                                MorsePreferences::interWordSpace = constrain(MorsePreferences::interWordSpace, 6, 45);            // has to be between 6 and 45 dits
                                internal::displayInterWordSpace();
                                updateTimings();
                                break;
                case  posInterCharSpace : MorsePreferences::interCharSpace = constrain(MorsePreferences::interCharSpace + t, 3, 24);  // set Interchar space - 3 - 24 dits
                                internal::displayInterCharSpace();                                           
                                updateTimings();
                                break;
                case  posRandomOption : MorsePreferences::randomOption = (MorsePreferences::randomOption + t + 10) % 10;     // which char set for random chars?
                                internal::displayRandomOption();
                                break;
                case  posRandomLength: MorsePreferences::randomLength += t;                                 // length of random char group: 2-6
                                MorsePreferences::randomLength = constrain(MorsePreferences::randomLength, 1, 10);                   // 7-10 for rnd length 2 to 3-6
                                internal::displayRandomLength();
                                break;
                case  posCallLength: if (MorsePreferences::callLength)                                             // length of calls: 0, or 3-6
                                        MorsePreferences::callLength -= 2;                                        // temorarily make it 0-4
                                MorsePreferences::callLength = constrain(MorsePreferences::callLength + t, 0, 4);
                                if (MorsePreferences::callLength)                                             // length of calls: 0, or 3-6
                                    MorsePreferences::callLength += 2;                                        // expand again if not 0
                                                           
                                internal::displayCallLength();
                                break;
                case  posAbbrevLength: MorsePreferences::abbrevLength += (t+1);                                 // length of abbreviations: 0, or 2-6
                                if (MorsePreferences::abbrevLength == 2)                                      // get rid of 1
                                  MorsePreferences::abbrevLength +=  t;
                                MorsePreferences::abbrevLength = constrain(MorsePreferences::abbrevLength-1, 0, 6);
                                internal::displayAbbrevLength();
                                break;
                case  posWordLength: MorsePreferences::wordLength += (t+1);                                   // length of English words: 0, or 2-6
                                if (MorsePreferences::wordLength == 2)                                        // get rid of 1
                                  MorsePreferences::wordLength +=  t;
                                MorsePreferences::wordLength = constrain(MorsePreferences::wordLength-1, 0, 6);
                                internal::displayWordLength();
                                break;
                case  posSerialOut:
                                MorsePreferences::serialOut = (MorsePreferences::serialOut + t + 8) % 8;                      // what to output on USB serial
                                if (MorsePreferences::serialOut == 6)
                                    MorsePreferences::serialOut = 3;
                                else if (MorsePreferences::serialOut == 4)
                                    MorsePreferences::serialOut = 7;
                                
                                internal::displaySerialOut();
                                break;
                case  posMaxSequence: 
                                switch(MorsePreferences::maxSequence) {
                                  case 0:
                                      if (t == -1) 
                                        MorsePreferences::maxSequence = 250;
                                      else
                                        MorsePreferences::maxSequence = 5;
                                      break;
                                   case 250:
                                      if (t == -1) 
                                        MorsePreferences::maxSequence = 245;
                                      else
                                        MorsePreferences::maxSequence = 0;
                                      break;
                                   default:
                                      MorsePreferences::maxSequence += 5*t;
                                      break;
                                }
                                internal::displayMaxSequence();
                                break;
                case  posTrainerDisplay: MorsePreferences::trainerDisplay = (MorsePreferences::trainerDisplay + t + 3) % 3;   // display options for trainer: 0-2
                                internal::displayTrainerDisplay();
                                break;
                case  posEchoDisplay: MorsePreferences::echoDisplay += t;
                                MorsePreferences::echoDisplay = constrain(MorsePreferences::echoDisplay, 1, 3);             // what prompt for echo trainer mode
                                internal::displayEchoDisplay();
                                break;
                case  posEchoRepeats: MorsePreferences::echoRepeats += (t+1);                                 // no of echo repeats: 0-6, 7=forever
                                MorsePreferences::echoRepeats = constrain(MorsePreferences::echoRepeats-1, 0, 7);
                                internal::displayEchoRepeats();
                                break;
                case  posEchoToneShift: MorsePreferences::echoToneShift += (t+1);                             // echo tone shift can be 0, 1 (up) or 2 (down)
                                MorsePreferences::echoToneShift = constrain(MorsePreferences::echoToneShift-1, 0, 2);
                                internal::displayEchoToneShift();
                                break;
                case  posWordDoubler: MorsePreferences::wordDoubler = !MorsePreferences::wordDoubler;
                                if (MorsePreferences::wordDoubler) {
                                  MorsePreferences::autoStopMode = false;
                                  cleanStartSettings();
                                }
                                internal::displayWordDoubler();
                                break;
                case posAutoStop: MorsePreferences::autoStopMode = !MorsePreferences::autoStopMode;
                                if (MorsePreferences::autoStopMode)
                                  MorsePreferences::wordDoubler = false;
                                cleanStartSettings();
                                internal::displayAutoStop();
                                break;                
                case  posRandomFile: 
                                if (MorsePreferences::randomFile)
                                  MorsePreferences::randomFile = 0;
                                else
                                  MorsePreferences::randomFile = 255;
                                 internal::displayRandomFile();
                                 break;
                case  posEchoConf:  MorsePreferences::echoConf = !MorsePreferences::echoConf;
                                internal::displayEchoConf();
                                break;
                case  posWifiTrainerMode: MorsePreferences::wifiTrainerMode += (t+1);                   // transmit wifi in generator and player mode; can be 0 (no) or 1 (WiFi)
                                MorsePreferences::wifiTrainerMode = (MorsePreferences::wifiTrainerMode % 2);
                                internal::displayWifiTrainerMode();
                                break; 
                case  posSpeedAdapt: MorsePreferences::speedAdapt = !MorsePreferences::speedAdapt;
                                internal::displaySpeedAdapt();
                                break; 
                case posExtAudioOnDecode:
                                  MorsePreferences::extAudioOnDecode = !MorsePreferences::extAudioOnDecode;
                                  internal::displayExtAudioOnDecode();
                                  break;
                case posTimeOut:  MorsePreferences::timeOut += (t+1);
                                  MorsePreferences::timeOut = constrain(MorsePreferences::timeOut-1, 1, 4);
                                  internal::displayTimeOut();
                                  break;
                case posQuickStart: MorsePreferences::quickStart = !MorsePreferences::quickStart;
                                  internal::displayQuickStart();
                                  break;
                case posSnapRecall: 
                                  if (memCounter) {
                                      memPtr = (memPtr +t + memCounter + 1) % (memCounter+1);
                                      //memPtr += (t+1);
                                      //memPtr = constrain(memPtr-1, 0, memCounter);
                                  }
                                  internal::displaySnapRecall();
                                  break;
                case posSnapStore: 
                                  memPtr = (memPtr + t + 9) % 9;
                                  internal::displaySnapStore();
                                  break;
            }   // end switch(pos)                  
            Heltec.display -> display();                                                      // update the display   

         }      // end if(encoderPos)
         checkShutDown(false);         // check for time out
    }    // end while(true) 
}   // end of function

/////////////// READING and WRITING parameters from / into Non Volatile Storage, using ESP32 preferences

void MorsePreferences::readPreferences(String repository) {
  unsigned int l = 15;
  char repName[l];
  uint8_t temp;
  uint32_t tempInt;  

  boolean morserino = false;
  
  if (repository == "morserino")
    morserino = true;

  repository.toCharArray(repName, l);
  // DEBUG("Reading from repository: " + String(repName));
  // read preferences from non-volatile storage
  // if version cannot be read, we have a new ESP32 and need to write the preferences first

  if (morserino) 
    pref.begin(repName, false);                // open namespace in read/write mode
  else
    pref.begin(repName, true);                 // read only in all other cases
 
  /// new code for reading preferences values - we check if we have a value, and if yes, we use it; if no, we use and write a default value

    if (morserino) {
      if ((temp = pref.getUChar("version_major")) != MorsePreferences::version_major)
         pref.putUChar("version_major", MorsePreferences::version_major);
      if ((temp = pref.getUChar("version_minor")) != MorsePreferences::version_minor)
         pref.putUChar("version_minor", MorsePreferences::version_minor);
 

      if (temp = pref.getUChar("snapShots")) {
          MorsePreferences::snapShots = temp;
          updateMemory(temp);
      }  // end: we have snapshots
      
      MorsePreferences::fileWordPointer = pref.getUInt("fileWordPtr"); // do not read fileWordPointer from other snapshots! we never write anything there!
    }  // endif morserino

    if ((temp = pref.getUChar("sidetoneFreq")))
       MorsePreferences::sidetoneFreq = temp;
    else if (morserino)
       pref.putUChar("sidetoneFreq", MorsePreferences::sidetoneFreq);

    if ((temp = pref.getUChar("wpm")))
       MorsePreferences::wpm = temp;
    else if (morserino)
       pref.putUChar("wpm", MorsePreferences::wpm);

    if ((temp = pref.getUChar("sidetoneVolume",255)) != 255)
       MorsePreferences::sidetoneVolume = temp;
    else if (morserino)
       pref.putUChar("sidetoneVolume", MorsePreferences::sidetoneVolume);

    if ((temp = pref.getUChar("keyermode")))
       MorsePreferences::keyermode = temp;
    else if (morserino)
       pref.putUChar("keyermode", MorsePreferences::keyermode);

    if ((temp = pref.getUChar("farnsworthMode")))
       MorsePreferences::interCharSpace = temp;
    else if (morserino)
       pref.putUChar("farnsworthMode", MorsePreferences::interCharSpace);

    if ((temp = pref.getUChar("ACSlength",255)) != 255)
       MorsePreferences::ACSlength = temp;
    else if (morserino)
       pref.putUChar("ACSlength", MorsePreferences::ACSlength);

    if ((temp = pref.getUChar("keyTrainerMode", 255)) != 255)
       MorsePreferences::keyTrainerMode = temp;
    else if (morserino)
       pref.putUChar("keyTrainerMode", MorsePreferences::keyTrainerMode);
 
    if ((temp = pref.getUChar("randomLength")))
       MorsePreferences::randomLength = temp;
    else if (morserino)
       pref.putUChar("randomLength", MorsePreferences::randomLength);

    if ((temp = pref.getUChar("randomOption", 255)) != 255)
       MorsePreferences::randomOption = temp;
    else if (morserino)
       pref.putUChar("randomOption", MorsePreferences::randomOption);

    if ((temp = pref.getUChar("callLength", 255)) != 255)
       MorsePreferences::callLength = temp;
    else if (morserino)
       pref.putUChar("callLength", MorsePreferences::callLength);
       
    if ((temp = pref.getUChar("abbrevLength", 255)) != 255)
       MorsePreferences::abbrevLength = temp;
    else if (morserino)
       pref.putUChar("abbrevLength", MorsePreferences::abbrevLength);

    if ((temp = pref.getUChar("wordLength", 255)) != 255)
       MorsePreferences::wordLength = temp;
    else if (morserino)
       pref.putUChar("wordLength", MorsePreferences::wordLength);

    if ((temp = pref.getUChar("serialOut", 255)) != 255)
      MorsePreferences::serialOut = temp;
    else if (morserino)
      pref.putUChar("serialOut", MorsePreferences::serialOut);

    if ((temp = pref.getUChar("trainerDisplay", 255)) != 255)
       MorsePreferences::trainerDisplay = temp;
    else if (morserino)
       pref.putUChar("trainerDisplay", MorsePreferences::trainerDisplay);

    if ((temp = pref.getUChar("echoDisplay", 255)) != 255)
       MorsePreferences::echoDisplay = temp;
    else if (morserino)
       pref.putUChar("echoDisplay", MorsePreferences::echoDisplay);

    if ((temp = pref.getUChar("curtisBTiming", 255)) != 255)
       MorsePreferences::curtisBTiming = temp;
    else if (morserino)
       pref.putUChar("curtisBTiming", MorsePreferences::curtisBTiming);

    if ((temp = pref.getUChar("curtisBDotT", 255)) != 255)
       MorsePreferences::curtisBDotTiming = temp;
    else if (morserino)
       pref.putUChar("curtisBDotT", MorsePreferences::curtisBDotTiming);

    if ((temp = pref.getUChar("interWordSpace")))
       MorsePreferences::interWordSpace = temp;
    else if (morserino)
       pref.putUChar("interWordSpace", MorsePreferences::interWordSpace);

    if ((temp = pref.getUChar("echoRepeats", 255)) != 255)
       MorsePreferences::echoRepeats = temp;
    else if (morserino)
       pref.putUChar("echoRepeats", MorsePreferences::echoRepeats);

    if ((temp = pref.getUChar("echoToneShift", 255)) != 255)
       MorsePreferences::echoToneShift = temp;
    else if (morserino)
       pref.putUChar("echoToneShift", MorsePreferences::echoToneShift);
    
    
    if ((temp = pref.getUChar("wifiTrainerMode")))
        MorsePreferences::wifiTrainerMode = temp;
    else if (morserino)
        pref.putUChar("wifiTrainerMode", MorsePreferences::wifiTrainerMode);

    if ((temp = pref.getUChar("latency")))
        MorsePreferences::latency = temp;
    else if (morserino)
        pref.putUChar("latency", MorsePreferences::latency);
    
    if ((temp = pref.getUChar("randomFile")))
        MorsePreferences::randomFile = temp;
        
    if ((temp = pref.getUChar("lastExecuted")))
       MorsePreferences::menuPtr = temp;
    //DEBUG("read: MorsePreferences::menuPtr = " + String(MorsePreferences::menuPtr));

    if ((temp = pref.getUChar("brightness")))
       MorsePreferences::oledBrightness = temp;

    if ((temp = pref.getUChar("timeOut")))
       MorsePreferences::timeOut = temp;
    else if (morserino)
       pref.putUChar("timeOut", MorsePreferences::timeOut);

    MorsePreferences::maxSequence = pref.getUChar("maxSequence");
        
    MorsePreferences::didah = pref.getBool("didah", true);
    MorsePreferences::reversePolarity = pref.getBool("useExtPaddle");
    MorsePreferences::encoderClicks = pref.getBool("encoderClicks", true);
    MorsePreferences::echoConf = pref.getBool("echoConf", true);
    MorsePreferences::wordDoubler = pref.getBool("wordDoubler");
    MorsePreferences::speedAdapt  = pref.getBool("speedAdapt");
    MorsePreferences::extAudioOnDecode = pref.getBool("extAudioOnDecode");

    MorsePreferences::wlanSSID = pref.getString("wlanSSID");
    MorsePreferences::wlanPassword = pref.getString("wlanPassword");
    MorsePreferences::wlanTRXPeer = pref.getString("wlanTRXPeer", "");

    MorsePreferences::wlanSSID1 = pref.getString("wlanSSID1");
    MorsePreferences::wlanPassword1 = pref.getString("wlanPassword1");
    MorsePreferences::wlanTRXPeer1 = pref.getString("wlanTRXPeer1", "");
    MorsePreferences::wlanSSID2 = pref.getString("wlanSSID2");
    MorsePreferences::wlanPassword2 = pref.getString("wlanPassword2");
    MorsePreferences::wlanTRXPeer2 = pref.getString("wlanTRXPeer2", "");
    MorsePreferences::wlanSSID3 = pref.getString("wlanSSID3");
    MorsePreferences::wlanPassword3 = pref.getString("wlanPassword3");
    MorsePreferences::wlanTRXPeer3 = pref.getString("wlanTRXPeer3", "");

    MorsePreferences::quickStart = pref.getBool("quickStart");
    MorsePreferences::autoStopMode  = pref.getBool("autoStop");

   pref.end();
   updateTimings();
}

void MorsePreferences::writePreferences(String repository) {
  unsigned int l = 15;
  char repName[l];
  uint8_t temp;
  uint32_t tempInt;  

  boolean morserino = false;
  
  if (repository == "morserino")
    morserino = true;
//DEBUG("Writing to repository: " + repository);
  repository.toCharArray(repName, l);

  pref.begin(repName, false);                // open namespace in read/write mode
 
    if (MorsePreferences::sidetoneFreq != pref.getUChar("sidetoneFreq"))
        pref.putUChar("sidetoneFreq", MorsePreferences::sidetoneFreq);    
    if (MorsePreferences::didah != pref.getBool("didah"))
        pref.putBool("didah", MorsePreferences::didah);
    if (MorsePreferences::keyermode != pref.getUChar("keyermode"))
        pref.putUChar("keyermode", MorsePreferences::keyermode);
    if (MorsePreferences::interCharSpace != pref.getUChar("farnsworthMode"))
        pref.putUChar("farnsworthMode", MorsePreferences::interCharSpace);
    if (MorsePreferences::reversePolarity != pref.getBool("useExtPaddle"))
        pref.putBool("useExtPaddle", MorsePreferences::reversePolarity);
    if (MorsePreferences::ACSlength != pref.getUChar("ACSlength"))
        pref.putUChar("ACSlength", MorsePreferences::ACSlength);
    if (MorsePreferences::keyTrainerMode != pref.getUChar("keyTrainerMode"))
        pref.putUChar("keyTrainerMode", MorsePreferences::keyTrainerMode);
    if (MorsePreferences::encoderClicks != pref.getBool("encoderClicks"))
        pref.putBool("encoderClicks", MorsePreferences::encoderClicks);   
    if (MorsePreferences::randomLength != pref.getUChar("randomLength"))
        pref.putUChar("randomLength", MorsePreferences::randomLength);
    if (MorsePreferences::randomOption != pref.getUChar("randomOption"))
        pref.putUChar("randomOption", MorsePreferences::randomOption);
    if (MorsePreferences::callLength != pref.getUChar("callLength"))
        pref.putUChar("callLength", MorsePreferences::callLength);
    if (MorsePreferences::abbrevLength != pref.getUChar("abbrevLength")) {
        pref.putUChar("abbrevLength", MorsePreferences::abbrevLength);
    }
    if (MorsePreferences::wordLength != pref.getUChar("wordLength")) {
        pref.putUChar("wordLength", MorsePreferences::wordLength);
    }
    if (MorsePreferences::serialOut != pref.getUChar("serialOut"))
        pref.putUChar("serialOut", MorsePreferences::serialOut);
    if (MorsePreferences::trainerDisplay != pref.getUChar("trainerDisplay"))
        pref.putUChar("trainerDisplay", MorsePreferences::trainerDisplay);
    if (MorsePreferences::echoDisplay != pref.getUChar("echoDisplay"))
        pref.putUChar("echoDisplay", MorsePreferences::echoDisplay);
    if (MorsePreferences::curtisBTiming != pref.getUChar("curtisBTiming"))
        pref.putUChar("curtisBTiming", MorsePreferences::curtisBTiming);
    if (MorsePreferences::curtisBDotTiming != pref.getUChar("curtisBDotT")) 
        pref.putUChar("curtisBDotT", MorsePreferences::curtisBDotTiming);
    if (MorsePreferences::interWordSpace != pref.getUChar("interWordSpace"))
        pref.putUChar("interWordSpace", MorsePreferences::interWordSpace);
    if (MorsePreferences::echoRepeats != pref.getUChar("echoRepeats"))
        pref.putUChar("echoRepeats", MorsePreferences::echoRepeats);
    if (MorsePreferences::echoToneShift != pref.getUChar("echoToneShift"))
        pref.putUChar("echoToneShift", MorsePreferences::echoToneShift);
    if (MorsePreferences::echoConf != pref.getBool("echoConf"))
        pref.putBool("echoConf", MorsePreferences::echoConf);
    
    if (MorsePreferences::wordDoubler != pref.getBool("wordDoubler"))
        pref.putBool("wordDoubler", MorsePreferences::wordDoubler);
    if (MorsePreferences::speedAdapt != pref.getBool("speedAdapt"))
        pref.putBool("speedAdapt", MorsePreferences::speedAdapt);
    if (MorsePreferences::wifiTrainerMode != pref.getUChar("wifiTrainerMode"))
        pref.putUChar("wifiTrainerMode", MorsePreferences::wifiTrainerMode);
    if (MorsePreferences::maxSequence != pref.getUChar("maxSequence")) 
        pref.putUChar("maxSequence", MorsePreferences::maxSequence);
    if (MorsePreferences::latency != pref.getUChar("latency"))
        pref.putUChar("latency", MorsePreferences::latency);
    if (MorsePreferences::randomFile != pref.getUChar("randomFile"))
        pref.putUChar("randomFile", MorsePreferences::randomFile);
    if (MorsePreferences::extAudioOnDecode != pref.getBool("extAudioOnDecode"))
        pref.putBool("extAudioOnDecode", MorsePreferences::extAudioOnDecode);
    if (MorsePreferences::timeOut != pref.getUChar("timeOut"))
        pref.putUChar("timeOut", MorsePreferences::timeOut);
    if (MorsePreferences::quickStart != pref.getBool("quickStart"))
        pref.putBool("quickStart", MorsePreferences::quickStart);
    if (MorsePreferences::autoStopMode != pref.getBool("autoStop"))
        pref.putBool("autoStop", MorsePreferences::autoStopMode);

    if (MorsePreferences::snapShots != pref.getUChar("snapShots"))
        pref.putUChar("snapShots", MorsePreferences::snapShots);

    if (MorsePreferences::wlanSSID != pref.getString("wlanSSID"))
        pref.putString("wlanSSID", MorsePreferences::wlanSSID);
    if (MorsePreferences::wlanPassword != pref.getString("wlanPassword"))
        pref.putString("wlanPassword", MorsePreferences::wlanPassword);
    if (MorsePreferences::wlanTRXPeer != pref.getString("wlanTRXPeer"))
        pref.putString("wlanTRXPeer", MorsePreferences::wlanTRXPeer);

    if (MorsePreferences::wlanSSID1 != pref.getString("wlanSSID1"))
        pref.putString("wlanSSID1", MorsePreferences::wlanSSID1);
    if (MorsePreferences::wlanPassword1 != pref.getString("wlanPassword1"))
        pref.putString("wlanPassword1", MorsePreferences::wlanPassword1);
    if (MorsePreferences::wlanTRXPeer1 != pref.getString("wlanTRXPeer1"))
        pref.putString("wlanTRXPeer1", MorsePreferences::wlanTRXPeer1);

    if (MorsePreferences::wlanSSID2 != pref.getString("wlanSSID2"))
        pref.putString("wlanSSID2", MorsePreferences::wlanSSID2);
    if (MorsePreferences::wlanPassword2 != pref.getString("wlanPassword2"))
        pref.putString("wlanPassword2", MorsePreferences::wlanPassword2);
    if (MorsePreferences::wlanTRXPeer2 != pref.getString("wlanTRXPeer2"))
        pref.putString("wlanTRXPeer2", MorsePreferences::wlanTRXPeer2);

    if (MorsePreferences::wlanSSID3 != pref.getString("wlanSSID3"))
        pref.putString("wlanSSID3", MorsePreferences::wlanSSID3);
    if (MorsePreferences::wlanPassword3 != pref.getString("wlanPassword3"))
        pref.putString("wlanPassword3", MorsePreferences::wlanPassword3);
    if (MorsePreferences::wlanTRXPeer3 != pref.getString("wlanTRXPeer3"))
        pref.putString("wlanTRXPeer3", MorsePreferences::wlanTRXPeer3);

    if (! morserino)  {
        pref.putUChar("lastExecuted", MorsePreferences::menuPtr);   // store last executed command in snapshots

     if (morserino) {
        pref.putUChar("brightness", MorsePreferences::oledBrightness);  // if not snapshots, store current screen brightness
     }
    }


  pref.end();
}

boolean  MorsePreferences::recallSnapshot() {         // return true if we selected a real recall, false when it was cancelled
    String snapname;
    String text;

    memPtr = 0;
    displayKeyerPreferencesMenu(posSnapRecall);
    if (!adjustKeyerPreference(posSnapRecall)) {
        //DEBUG("recall memPtr: " + String(memPtr));
        text = "Snap " + String(memories[memPtr]+1) + " RECALLD";
        if(memCounter) {
          if (memPtr != memCounter)  {
            snapname = "snap" + String(memories[memPtr]);
            //DEBUG("recall snapname: " + snapname);
            readPreferences(snapname);
            MorseOutput::printOnScroll(2, BOLD, 0, text);
            //DEBUG("after recall - p_menuPtr: " + String(p_menuPtr));
            delay(1000);
            return true;
          }
          return false;
        }
    } return false;
      
}

boolean MorsePreferences::storeSnapshot(uint8_t menu) {        // return true if we selected a real store, false when it was cancelled
    String snapname;
    uint8_t mask = 1;
    String text;

    memPtr = 0;
    displayKeyerPreferencesMenu(posSnapStore);
    adjustKeyerPreference(posSnapStore);
    Buttons:: volButton.Update();
        //DEBUG("store memPtr: " + String(memPtr));
    if (memPtr != 8)  {
        MorsePreferences::menuPtr = menu;     // also store last menu selection
        //DEBUG("menu: " + String(p_menuPtr));
        text = "Snap " + String(memPtr+1) + " STORED ";
        snapname = "snap" + String(memPtr);
        //DEBUG("store snapname: " + snapname);
        //DEBUG("store: p_menuPtr = " + String(p_menuPtr));
        writePreferences(snapname);
        /// insert the correct bit into p_snapShots & update memory variables
        mask = mask << memPtr;
        MorsePreferences::snapShots = MorsePreferences::snapShots | mask;
        //DEBUG("store p_snapShots: " + String(p_snapShots));
        MorseOutput::printOnScroll(2, BOLD, 0, text);
        updateMemory(MorsePreferences::snapShots);
        delay(1000);
        return true;
      }
      return false;
}


void MorsePreferences::updateMemory(uint8_t temp) {
  memCounter = 0;                 // create an array that contains the snapshots (memories) that are in use
        for (int i = 0; i<8; ++i) {
          if (temp & 1) {     // mask rightmost bit
            memories[memCounter] = i;
            ++memCounter;
          }
          temp = temp >> 1;   // shift one position to the right
        }
}


void MorsePreferences::clearMemory(uint8_t ptr) {
  String text = "Snap " + String(memories[ptr]+1) + " CLEARED";

  MorsePreferences::snapShots &= ~(1 << memories[ptr]);     // clear the bit
  MorseOutput::printOnScroll(2, BOLD, 0, text);
  updateMemory(MorsePreferences::snapShots);
  delay(1000);
}


void MorsePreferences::fireCharSeen(boolean wpmOnly)
{
    pref.begin("morserino", false);             // open the namespace as read/write
    pref.putUChar("wpm", MorsePreferences::wpm);
    if (!wpmOnly)
    {
        pref.putUChar("tLeft", MorsePreferences::tLeft);
        pref.putUChar("tRight", MorsePreferences::tRight);
    }
    pref.end();
}

void MorsePreferences::writeWordPointer()
{
    pref.begin("morserino", false);              // open the namespace as read/write
    if ((MorsePreferences::fileWordPointer != pref.getUInt("fileWordPtr")))
    {   // update word pointer if necessary (if we ran player before)
        pref.putUInt("fileWordPtr", MorsePreferences::fileWordPointer);
    }
    pref.end();

}

void MorsePreferences::writeVolume()
{
    pref.begin("morserino", false);                     // open the namespace as read/write
    if (pref.getUChar("sidetoneVolume") != MorsePreferences::sidetoneVolume)
        pref.putUChar("sidetoneVolume", MorsePreferences::sidetoneVolume);  // store the last volume, if it has changed
    pref.end();
}

void MorsePreferences::writeLastExecuted(uint8_t menuPtr)
{
    pref.begin("morserino", false);             // open the namespace as read/write
    pref.putUChar("lastExecuted", menuPtr);   // store last executed command
    pref.end();                                 // close namespace
}

void MorsePreferences::writeWifiInfoMultiple(
  String ssid1, String passwd1, String trxpeer1,
  String ssid2, String passwd2, String trxpeer2,
  String ssid3, String passwd3, String trxpeer3
  )
{
    pref.begin("morserino", false);             // open the namespace as read/write

    if (ssid1 != "")
      MorsePreferences::wlanSSID1 = ssid1;
    if (passwd1 != "")
      MorsePreferences::wlanPassword1 = passwd1;
    //if (trxpeer != "")
      MorsePreferences::wlanTRXPeer1 = trxpeer1;

    if (MorsePreferences::wlanSSID1 != pref.getString("wlanSSID1"))
        pref.putString("wlanSSID1", MorsePreferences::wlanSSID1);
    if (MorsePreferences::wlanPassword1 != pref.getString("wlanPassword1"))
        pref.putString("wlanPassword1", MorsePreferences::wlanPassword1);
    if (MorsePreferences::wlanTRXPeer1 != pref.getString("wlanTRXPeer1"))
        pref.putString("wlanTRXPeer1", MorsePreferences::wlanTRXPeer1);

    if (ssid2 != "")
      MorsePreferences::wlanSSID2 = ssid2;
    if (passwd2 != "")
      MorsePreferences::wlanPassword2 = passwd2;
    //if (trxpeer != "")
      MorsePreferences::wlanTRXPeer2 = trxpeer2;

    if (MorsePreferences::wlanSSID2 != pref.getString("wlanSSID2"))
        pref.putString("wlanSSID2", MorsePreferences::wlanSSID2);
    if (MorsePreferences::wlanPassword2 != pref.getString("wlanPassword2"))
        pref.putString("wlanPassword2", MorsePreferences::wlanPassword2);
    if (MorsePreferences::wlanTRXPeer2 != pref.getString("wlanTRXPeer2"))
        pref.putString("wlanTRXPeer2", MorsePreferences::wlanTRXPeer2);

    if (ssid3 != "")
      MorsePreferences::wlanSSID3 = ssid3;
    if (passwd3 != "")
      MorsePreferences::wlanPassword3 = passwd3;
    //if (trxpeer != "")
      MorsePreferences::wlanTRXPeer3 = trxpeer3;

    if (MorsePreferences::wlanSSID3 != pref.getString("wlanSSID3"))
        pref.putString("wlanSSID3", MorsePreferences::wlanSSID3);
    if (MorsePreferences::wlanPassword3 != pref.getString("wlanPassword3"))
        pref.putString("wlanPassword3", MorsePreferences::wlanPassword3);
    if (MorsePreferences::wlanTRXPeer3 != pref.getString("wlanTRXPeer3"))
        pref.putString("wlanTRXPeer3", MorsePreferences::wlanTRXPeer3);

    pref.end();

    writeWifiInfo(ssid1, passwd1, trxpeer1);
}

void MorsePreferences::writeWifiInfo(String ssid, String passwd, String trxpeer)
{
    if (ssid != "")
      MorsePreferences::wlanSSID = ssid;
    if (passwd != "")
      MorsePreferences::wlanPassword = passwd;
    //if (trxpeer != "")
      MorsePreferences::wlanTRXPeer = trxpeer;
    pref.begin("morserino", false);             // open the namespace as read/write
    if (MorsePreferences::wlanSSID != pref.getString("wlanSSID"))
        pref.putString("wlanSSID", MorsePreferences::wlanSSID);
    if (MorsePreferences::wlanPassword != pref.getString("wlanPassword"))
        pref.putString("wlanPassword", MorsePreferences::wlanPassword);
    if (MorsePreferences::wlanTRXPeer != pref.getString("wlanTRXPeer"))
        pref.putString("wlanTRXPeer", MorsePreferences::wlanTRXPeer);

    pref.end();

}

void MorsePreferences::setCurrentOptions(prefPos *current, int size) {
  MorsePreferences::currentOptions = current;
  currentOptionSize = size;
}

