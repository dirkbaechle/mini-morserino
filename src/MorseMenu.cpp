/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018-2020  Willi Kraml, OE1WKL                                                                            ***
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

#include "MorseMenu.h"
#include "MorseOutput.h"
#include "MorseDecoder.h"

using namespace MorseMenu;

//////// variables and constants for the modus menu


const uint8_t menuN = 28;     // no of menu items +1

const String menuText [menuN] = {
  "",
  "CW Keyer", // 1
  
  "CW Generator", // 2
    "Random",
    "CW Abbrevs",
    "English Words",
    "Call Signs",
    "Mixed",
    "File Player",

  "Echo Trainer", // 9
    "Random",
    "CW Abbrevs",
    "English Words",
    "Call Signs",
    "Mixed",
    "File Player",
  
  "Transceiver",    // 16
    "WiFi Trx",
    "iCW/Ext Trx",
  
  "CW Decoder",     // 19

  "WiFi Functions", // 20
    "Disp MAC Addr",
    "Config WiFi",
    "Check WiFi",
    "Upload File",
    "Update Firmw", // 25
    "Wifi Select", // 26
  
  "Go To Sleep" } ; // 27

enum navi {naviLevel, naviLeft, naviRight, naviUp, naviDown };
       

const uint8_t menuNav [menuN] [5] = {                   // { level, left, right, up, down}
  { 0,0,0,0,0},                                         // 0 = _dummy                
  {0,_goToSleep,_gen,_dummy,0},                         // 1 _keyer
  {0,_keyer,_echo,_dummy,_genRand},                     // 2 _gen
    {1,_genPlayer,_genAbb,_gen,0},                        // 3 _genRand
    {1,_genRand,_genWords,_gen,0},                        // 4 _genAbb
    {1,_genAbb,_genCalls,_gen,0},                         // 5 _genWords
    {1,_genWords,_genMixed,_gen,0},                       // 6 _genCalls
    {1,_genCalls,_genPlayer,_gen,0},                      // 7 _genMixed
    {1,_genMixed,_genRand,_gen,0},                        // 8 _genPlayer
  {0,_gen,_trx,_dummy,_echoRand},                       // 9 _echo
    {1,_echoPlayer,_echoAbb,_echo,0},                     // 10 _echoRand
    {1,_echoRand,_echoWords,_echo,0},                     // 11 _echoAbb
    {1,_echoAbb,_echoCalls,_echo,0},                      // 12 _echoWords
    {1,_echoWords,_echoMixed,_echo,0},                    // 13 _echoCalls
    {1,_echoCalls,_echoPlayer,_echo,0},                   // 14 _echoMixed
    {1,_echoMixed,_echoRand,_echo,0},                     // 15 _echoPlayer
  {0,_echo,_decode,_dummy,_trxWifi},                    // 16 _trx
    {1,_dummy,_trxIcw,_trx,0},                          // 17 _trxWifi
    {1,_trxWifi,_dummy,_trx,0},                          // 18 _trxIcw
  {0,_trx,_wifi,_dummy,0},                              // 19 _decode
  {0,_decode,_goToSleep,_dummy,_wifi_mac},              // 20 _wifi
    {1,_wifi_select,_wifi_config,_wifi,0},                // 21 _wifi_mac
    {1,_wifi_mac,_wifi_check,_wifi,0},                    // 22 _wifi_config
    {1,_wifi_config,_wifi_upload,_wifi,0},                // 23 _wifi_check
    {1,_wifi_check,_wifi_update,_wifi,0},                 // 24 _wifi_upload
    {1,_wifi_upload,_wifi_select,_wifi,0},                // 25 _wifi_update
    {1,_wifi_update,_wifi_mac,_wifi,0},                   // 26 _wifi_select
  {0,_wifi,_keyer,_dummy,0}                             // 27 _goToSleep

};

//boolean quickStart;                                     // should we execute menu item immediately?

////// The MENU


void MorseMenu::menu_() {
   uint8_t newMenuPtr = MorsePreferences::menuPtr;
   uint8_t disp = 0;
   int t, command;
   
    WiFi.disconnect(true, false);
    active = false;
    cleanStartSettings();
    MorseOutput::clearScroll();                  // clear the buffer

    keyOut(false, true, 0, 0);
    keyOut(false, false, 0, 0);
    encoderState = speedSettingMode;             // always start with this encoderstate (decoder will change it anyway)
    MorsePreferences::setCurrentOptions(MorsePreferences::allOptions, MorsePreferences::allOptionsSize);
    MorsePreferences::writeWordPointer();
    file.close();                               // just in case it is still open....
    MorseOutput::clearDisplay();
    
    while (true) {                          // we wait for a click (= selection)
        if (disp != newMenuPtr) {
          disp = newMenuPtr;
          MorseMenu::menuDisplay(disp);
        }
        if (quickStart) {
            quickStart = false;
            command = 1;
            delay(250);
            MorseOutput::printOnScroll(2, REGULAR, 1, "QUICK START");
            Heltec.display -> display();
            delay(600);
            MorseOutput::clearDisplay();
        }
        else {           
            Buttons::modeButton.Update();
            command = Buttons::modeButton.clicks;
        }

        switch (command) {                                          // actions based on encoder button
          case 2: if (MorsePreferences::setupPreferences(newMenuPtr))                       // all available options when called from top menu
                    newMenuPtr = MorsePreferences::menuPtr;
                  MorseMenu::menuDisplay(newMenuPtr);
                  break;
          case 1: // check if we have a submenu or if we execute the selection
                  //DEBUG("newMP: " + String(newMenuPtr) + " navi: " + String(menuNav[newMenuPtr][naviDown]));
                  if (menuNav[newMenuPtr][naviDown] == 0) {
                      MorsePreferences::menuPtr = newMenuPtr;
                      disp = 0;
                      if (MorsePreferences::menuPtr < _wifi) {                        // remember last executed, unless it is a wifi function or shutdown
                        MorsePreferences::writeLastExecuted(newMenuPtr);
                      }
                      if (MorseMenu::menuExec())
                        return;
                  } else {
                      newMenuPtr = menuNav[newMenuPtr][naviDown];
                  }
                  break;
          case -1:  // we need to go one level up, if possible
                  if (menuNav[newMenuPtr][naviUp] != 0) 
                      newMenuPtr = menuNav[newMenuPtr][naviUp];
          default: break;
        }

       if ((t=checkEncoder())) {
          MorseOutput::pwmClick(MorsePreferences::sidetoneVolume);         /// click
          newMenuPtr =  menuNav [newMenuPtr][(t == -1) ? naviLeft : naviRight];
       }

       Buttons::volButton.Update();
    
       switch (Buttons::volButton.clicks) {
          case -1:  audioLevelAdjust();                         /// for adjusting line-in audio level (at the same time keying tx and sending oudio on line-out
                    MorseOutput::clearDisplay();
                    MorseMenu::menuDisplay(disp);
                    break;
          case 2:   MorseOutput::decreaseBrightness();
                    MorseMenu::menuDisplay(disp);
                    break;
       }
       checkShutDown(false);                  // check for time out   
  } // end while - we leave as soon as the button has been pressed
} // end menu_() 


void MorseMenu::menuDisplay(uint8_t ptr) {
  //DEBUG("Level: " + (String) menuNav [ptr][naviLevel] + " " + menuText[ptr]);
  uint8_t oneUp = menuNav[ptr][naviUp];
  uint8_t twoUp = menuNav[oneUp][naviUp];
  uint8_t oneDown = menuNav[ptr][naviDown];

  MorseOutput::printOnStatusLine( true, 0,  "Select Modus:     ");
  
  // delete previous content
  MorseOutput::clearThreeLines();
  
  /// level 0: top line, possibly ".." on line 1
  /// level 1: higher level on 0, item on 1, possibly ".." on 2
  /// level 2: higher level on 1, highest level on 0, item on 2
  switch (menuNav [ptr][naviLevel]) {
    case 2: MorseOutput::printOnScroll(2, BOLD, 0, menuText[ptr]);
            MorseOutput::printOnScroll(1, REGULAR, 0, menuText[oneUp]);
            MorseOutput::printOnScroll(0, REGULAR, 0, menuText[twoUp]);
            break;
    case 1: if (oneDown)
                MorseOutput::printOnScroll(2, REGULAR, 0, String(".."));
            MorseOutput::printOnScroll(1, BOLD, 0, menuText[ptr]);
            MorseOutput::printOnScroll(0, REGULAR, 0, menuText[oneUp]);
            break;
    case 0: 
            if (oneDown)
                MorseOutput::printOnScroll(1, REGULAR, 0, String(".."));
            MorseOutput::printOnScroll(0, BOLD, 0, menuText[ptr]);
            break;
  }
}


boolean MorseMenu::menuExec() {                                          // return true if we should  leave menu after execution, true if we should stay in menu

  uint32_t wcount = 0;
//  String peer;
//  const char* peerHost;
  String s;

  keyerState = IDLE_STATE;
  
  switch (MorsePreferences::menuPtr) {
    case  _keyer:  /// keyer
                MorsePreferences::setCurrentOptions(MorsePreferences::keyerOptions, MorsePreferences::keyerOptionsSize);
                morseState = morseKeyer;
                showStartDisplay("Start CW Keyer", "", "", 500);
                clearPaddleLatches();
                if(MorsePreferences::keyermode == STRAIGHTKEY)
                  keyDecoder.setup();
                return true;
                break;
     case _genRand:
     case _genAbb:
     case _genWords:
     case _genCalls:
     case _genMixed:      /// generator
                generatorMode = (GEN_TYPE) (MorsePreferences::menuPtr - 3);                   /// 0 = RANDOMS ... 4 = MIXED, 5 = PLAYER
                MorsePreferences::setCurrentOptions(MorsePreferences::generatorOptions, MorsePreferences::generatorOptionsSize);
                goto startGenerator;
     case _genPlayer:  
                generatorMode = (GEN_TYPE) (MorsePreferences::menuPtr - 3);                   /// 0 = RANDOMS ... 4 = MIXED, 5 = PLAYER
                MorsePreferences::setCurrentOptions(MorsePreferences::playerOptions, MorsePreferences::playerOptionsSize);
                file = SPIFFS.open("/player.txt");                            // open file
                //skip p_fileWordPointer words, as they have been played before
                wcount = MorsePreferences::fileWordPointer;
                MorsePreferences::fileWordPointer = 0;
                skipWords(wcount);
                
     startGenerator:
                startFirst = true;
                firstTime = true;
                morseState = morseGenerator;
                showStartDisplay("Generator     ", "Start/Stop:   ", "Paddle | BLACK", 1250);
                if (MorsePreferences::wifiTrainerMode == 1)
                  if (!setupWifi())
                    return false;
                return true;
                break;
      case  _echoRand:
      case  _echoAbb:
      case  _echoWords:
      case  _echoCalls:
      case  _echoMixed:
                MorsePreferences::setCurrentOptions(MorsePreferences::echoTrainerOptions, MorsePreferences::echoTrainerOptionsSize);

                generatorMode = (GEN_TYPE) (MorsePreferences::menuPtr - 10);                /// 0 = RANDOMS ... 4 = MIXED, 5 = PLAYER
                goto startEcho;
      case  _echoPlayer:    /// echo trainer
                generatorMode = (GEN_TYPE) (MorsePreferences::menuPtr - 10);                /// 0 = RANDOMS ... 4 = MIXED, 5 = PLAYER
                MorsePreferences::setCurrentOptions(MorsePreferences::echoPlayerOptions, MorsePreferences::echoPlayerOptionsSize);
                file = SPIFFS.open("/player.txt");                            // open file
                //skip p_fileWordPointer words, as they have been played before
                wcount = MorsePreferences::fileWordPointer;
                MorsePreferences::fileWordPointer = 0;
                skipWords(wcount);
       startEcho:
                startFirst = true;
                morseState = echoTrainer;
                echoStop = false;
                showStartDisplay("Echo Trainer:", "Start:       ", "Press paddle ", 1250);
                if(MorsePreferences::keyermode == STRAIGHTKEY)
                  keyDecoder.setup();
                return true;
                break;
      case  _trxWifi: // Wifi Transceiver
                generatorMode = RANDOMS;  // to reset potential KOCH_LEARN
                MorsePreferences::setCurrentOptions(MorsePreferences::wifiTrxOptions, MorsePreferences::wifiTrxOptionsSize);
                morseState = wifiTrx;
                MorseOutput::clearDisplay();
                MorseOutput::printOnScroll(0, REGULAR, 0, "Connecting...");

                if (!setupWifi())
                  return false;
                //DEBUG("Peer IP: " + peerIP.toString());
                s = peerIP.toString();
                showStartDisplay("", "Start Wifi Trx", s  == "255.255.255.255" ?"IP Broadcast" : s, 1500);

                MorseWiFi::audp.listen(MORSERINOPORT); // listen on port 7373
                MorseWiFi::audp.onPacket(onWifiReceive);
                clearPaddleLatches();
                //keyTx = false;
                clearText = "";
                return true;
                break;
      case  _trxIcw: /// icw/ext TRX
                MorsePreferences::setCurrentOptions(MorsePreferences::extTrxOptions, MorsePreferences::extTrxOptionsSize);
                morseState = morseTrx;
                MorseOutput::clearDisplay();
                MorseOutput::printOnScroll(1, REGULAR, 0, "Start CW Trx" );
                clearPaddleLatches();
                goto setupDecoder;

      case  _decode: /// decoder
                MorsePreferences::setCurrentOptions(MorsePreferences::decoderOptions, MorsePreferences::decoderOptionsSize);
                morseState = morseDecoder;
                  /// here we will do the init for decoder mode
                encoderState = volumeSettingMode;
                MorseOutput::clearDisplay();
                MorseOutput::printOnScroll(1, REGULAR, 0, "Start Decoder" );
      setupDecoder:
                speedChanged = true;
                delay(650);
                cleanupScreen();
                MorseOutput::drawInputStatus(false);

                displayCWspeed();
                MorseOutput::displayVolume(encoderState == speedSettingMode, MorsePreferences::sidetoneVolume);                                     // sidetone volume
                keyDecoder.setup();
                return true;
                break;
      case  _wifi_mac:
      case  _wifi_config:
      case _wifi_check:
      case _wifi_upload:
      case _wifi_update:
                  MorseWiFi::menuExec((uint8_t) MorsePreferences::menuPtr);
                  break;
      case _wifi_select:
                  MorseWiFi::menuNetSelect();
                  break;
      case  _goToSleep: /// deep sleep
                checkShutDown(true);
      default:  break;
  }
  return false;
}   /// end menuExec()

boolean MorseMenu::setupWifi() {
  String peer;
  const char* peerHost;

//// if not true WiFi has not been configured or is not available, hence return false!
  if (! MorseWiFi::wifiConnect()) {
    delay(1000);    // wait a bit
    return(false);
  }

  if (MorsePreferences::wlanTRXPeer.length() == 0) 
      peer = "255.255.255.255";     // send to local broadcast IP if not set
  else
      peer = MorsePreferences::wlanTRXPeer;
  peerHost = peer.c_str();
  if (!peerIP.fromString(peerHost)) {    // try to interpret the peer as an ip address...
    //DEBUG("hostname: " + String(peerHost));
      int err = WiFi.hostByName(peerHost, peerIP); // ...and resolve peer into ip address if that fails
    //DEBUG("errcode: " + String(err));
      if (err != 1)                       // if that fails too, use broadcast
        peerIP.fromString("255.255.255.255");
  }
  return true;
}


void MorseMenu::cleanupScreen() {
    MorseOutput::clearDisplay();
    updateTopLine();
    MorseOutput::clearScroll();      // clear the buffer}
}

void MorseMenu::showStartDisplay(String l0, String l1, String l2, int pause) {
    MorseOutput::clearDisplay();
    if (l0.length())
        MorseOutput::printOnScroll(0, REGULAR, 0, l0);
    if (l1.length())
        MorseOutput::printOnScroll(1, REGULAR, 0, l1);
    if (l2.length())
        MorseOutput::printOnScroll(2, REGULAR, 0, l2);
    delay(pause);
    cleanupScreen();
}
