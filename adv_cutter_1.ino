//
// NB! This is a file generated from the .4Dino file, changes will be lost
//     the next time the .4Dino file is built
//
// Define LOG_MESSAGES to a serial port to send SPE errors messages to. Do not use the same Serial port as SPE
//#define LOG_MESSAGES Serial

#define RESETLINE     30

#define DisplaySerial Serial1


#include "adv_cutter_1Const.h"

#include "Picaso_Serial_4DLib.h"
#include "Picaso_LedDigitsDisplay.h"
#include "Picaso_PrintDisk.h"
#include "Picaso_Const4D.h"

Picaso_Serial_4DLib Display(&DisplaySerial);

// Uncomment to use ESP8266
//#define ESPRESET 17
//#include <SoftwareSerial.h>
//#define ESPserial SerialS
//SoftwareSerial SerialS(8, 9) ;
// Uncomment next 2 lines to use ESP8266 with ESP8266 library from https://github.com/itead/ITEADLIB_Arduino_WeeESP8266
//#include "ESP8266.h"
//ESP8266 wifi(SerialS,19200);

// routine to handle Serial errors
void mycallback(int ErrCode, unsigned char Errorbyte)
{
#ifdef LOG_MESSAGES
  const char *Error4DText[] = {"OK\0", "Timeout\0", "NAK\0", "Length\0", "Invalid\0"} ;
  LOG_MESSAGES.print(F("Serial 4D Library reports error ")) ;
  LOG_MESSAGES.print(Error4DText[ErrCode]) ;
  if (ErrCode == Err4D_NAK)
  {
    LOG_MESSAGES.print(F(" returned data= ")) ;
    LOG_MESSAGES.println(Errorbyte) ;
  }
  else
    LOG_MESSAGES.println(F("")) ;
  while (1) ; // you can return here, or you can loop
#else
  // Pin 13 has an LED connected on most Arduino boards. Just give it a name
#define led 13
  while (1)
  {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(200);                // wait for a second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(200);                // wait for a second
  }
#endif
}
// end of routine to handle Serial errors

word hndl ;

#define HOME 0        // Home window
#define MENU_SPEED 1  // adjust speed window
#define MENU_LENGTH 2 // adjust cutting length window
#define MENU_QTY 3    // adjust quantity window

#define MENU_RESET_DELAY 60000 // delay time when the menu resets to the home screen in ms

bool system_stop;
char menu_state;
long timer;
word hFont1, hstrings;
int txt_start_left = 8; // left offset of text
int txt_start_height = 8; // top offset of text

void setup()
{
// Ucomment to use the Serial link to the PC for debugging
//  Serial.begin(115200) ;        // serial to USB port
// Note! The next statement will stop the sketch from running until the serial monitor is started
//       If it is not present the monitor will be missing the initial writes
//    while (!Serial) ;             // wait for serial to be established

  pinMode(RESETLINE, OUTPUT);       // Display reset pin
digitalWrite(RESETLINE, 1);       // Reset Display, using shield
  delay(100);                       // wait for it to be recognised
digitalWrite(RESETLINE, 0);       // Release Display Reset, using shield
// Uncomment when using ESP8266
//  pinMode(ESPRESET, OUTPUT);        // ESP reset pin
//  digitalWrite(ESPRESET, 1);        // Reset ESP
//  delay(100);                       // wait for it t
//  digitalWrite(ESPRESET, 0);        // Release ESP reset
  delay(3000) ;                     // give display time to startup

  // now start display as Serial lines should have 'stabilised'
  DisplaySerial.begin(200000) ;     // Hardware serial to Display, same as SPE on display is set to
  Display.TimeLimit4D = 5000 ;      // 5 second timeout on all commands
  Display.Callback4D = mycallback ;

// uncomment if using ESP8266
//  ESPserial.begin(115200) ;         // assume esp set to 115200 baud, it's default setting
                                    // what we need to do is attempt to flip it to 19200
                                    // the maximum baud rate at which software serial actually works
                                    // if we run a program without resetting the ESP it will already be 19200
                                    // and hence the next command will not be understood or executed
//  ESPserial.println("AT+UART_CUR=19200,8,1,0,0\r\n") ;
//  ESPserial.end() ;
//  delay(10) ;                         // Necessary to allow for baud rate changes
//  ESPserial.begin(19200) ;            // start again at a resonable baud rate
  Display.gfx_ScreenMode(PORTRAIT) ; // change manually if orientation change
  Display.putstr("Mounting...\n");
  if (!(Display.file_Mount()))
  {
    while(!(Display.file_Mount()))
    {
      Display.putstr("Drive not mounted...");
      delay(200);
      Display.gfx_Cls();
      delay(200);
    }
  }
  hFont1 = Display.file_LoadImageControl("ADV_CU~1.dnn", "ADV_CU~1.gnn", 1); // Open handle to access uSD fonts, uncomment if required and change nn to font number
  hstrings = Display.file_Open("ADV_CU~1.txf", 'r') ;                            // Open handle to access uSD strings, uncomment if required
  hndl = Display.file_LoadImageControl("ADV_CU~1.dat", "ADV_CU~1.gci", 1);
  // put your setup code here, to run once:

  //Display.gfx_BGcolour(SILVER) ;
  Display.gfx_Cls() ;

  //pinMode(13, OUTPUT);
  Display.touch_Set(TOUCH_ENABLE); // enable the touch screen

  system_stop = true; // system run state, true => STOP system, false => system running
  menu_state = HOME;
  homeWindow(); // display the home window at startup
  timer = millis();

  Display.img_ClearAttributes(hndl, iUserbutton1, I_TOUCH_DISABLE); // speedButton set to enable touch, only need to do this once
  Display.img_ClearAttributes(hndl, iUserbutton2, I_TOUCH_DISABLE); // lengthButton set to enable touch, only need to do this once
  Display.img_ClearAttributes(hndl, iUserbutton3, I_TOUCH_DISABLE); // quantityButton set to enable touch, only need to do this once
  Display.img_ClearAttributes(hndl, iUserbutton4, I_TOUCH_DISABLE); // homeButton set to enable touch, only need to do this once
  Display.img_ClearAttributes(hndl, iWinbutton1, I_TOUCH_DISABLE); // startButton set to enable touch, only need to do this once
  Display.img_ClearAttributes(hndl, iWinbutton2, I_TOUCH_DISABLE); // stopButton set to enable touch, only need to do this once


} // end Setup **do not alter, remove or duplicate this line**

void loop()
{
  // put your main code here, to run repeatedly:

  byte state ;
  short x, y;
  while(1) {
    state = Display.touch_Get(TOUCH_STATUS); // get touchscreen status
    //-----------------------------------------------------------------------------------------
    if(state == TOUCH_PRESSED) { // if there's a press, or it's moving
      x = Display.touch_Get(TOUCH_GETX);
      y = Display.touch_Get(TOUCH_GETY);
      timer = millis();  // reset the dispaly timeout timer
    }

    //-----------------------------------------------------------------------------------------
    if((menu_state == MENU_SPEED) && (state == TOUCH_MOVING)) {  // if it's moving
      x = Display.touch_Get(TOUCH_GETX);
      y = Display.touch_Get(TOUCH_GETY);
      timer = millis();
      //if (n == iKnob1) moveiknob() ;
    }

    //-----------------------------------------------------------------------------------------
    if(state == TOUCH_RELEASED) {                     // if there's a release
      if ((x >= 0) && (x <= 240) && (y >= 250) && (y <= 320)){     // Width=188 Height= 50
        if (system_stop){
          // Start running the apllication
          system_stop = false; // change system state as it is now active
        } else {
          // Stop the system
          system_stop = true;
        }
        //menu_state = HOME;
        setDisplay();

      } else if((x >= 10) && (x <= 70) && (y >= 172) && (y <= 232)){
        if (menu_state == HOME) {
          // Speed menu button pressed
          menu_state = MENU_SPEED;
        } else {
          menu_state = HOME;
        }
        setDisplay();
      } else if(menu_state == HOME) {
       // only access the menus from the home state
        if((x >= 90) && (x <= 150) && (y >= 172) && (y <= 232)){
          // length menu button pressed
          menu_state = MENU_LENGTH;
          setDisplay();
        }
        else if((x >= 170) && (x <= 230) && (y >= 172) && (y <= 232)){
          // length menu button pressed
          menu_state = MENU_QTY;
          setDisplay();
        }
      }
    }

    if((menu_state != HOME) && (millis() - timer >= MENU_RESET_DELAY)){
      menu_state = HOME;
      setDisplay();
      //timer = millis();
    }
  }
}

void printToScreen(char* str, short x_pos, short y_pos) {
  Display.txt_Width(2);
  Display.txt_Height(2);
  Display.txt_BGcolour(SILVER);
  Display.gfx_MoveTo(x_pos,y_pos) ;
  Display.putstr(str) ;
}

void stopStartButtonDispaly() {

  if (system_stop){
    // display the start button
    //Display.gfx_Button(1, 10, 250, GREEN, WHITE, FONT1, 6, 6, "Start") ; // Button_Start Width=188 Height=49
    Display.img_SetWord(hndl, iWinbutton1, IMAGE_INDEX, 0); // startButton where state is 0 for up, 1 for down, 2 for 'on' up and 3 for 'on' down
    Display.img_Show(hndl,iWinbutton1) ;  // startButton
  } else {
    //Display.gfx_Button(1, 10, 250, RED, WHITE, FONT1, 6, 6, "Stop ") ; // Button_Start Width=188 Height=49
    Display.img_SetWord(hndl, iWinbutton2, IMAGE_INDEX, 1); // stopButton where state is 0 for up, 1 for down, 2 for 'on' up and 3 for 'on' down
    Display.img_Show(hndl,iWinbutton2) ;  // stopButton
  }
}

void homeButtonDisplay() {
  Display.img_SetWord(hndl, iUserbutton4, IMAGE_INDEX, 0); // homeButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton4) ;  // homeButton
}



void qtyDisplay(char* str, int start_height=8) {
  //printToScreen("Qty:", 30, 20);
  Display.txt_FontID(FONT3);
  Display.txt_Width(2) ;
  Display.txt_Height(2) ;
  Display.txt_FGcolour(WHITE) ;
  Display.txt_BGcolour(BLACK) ;
  Display.gfx_MoveTo(txt_start_left, start_height) ;
  Display.putstr(str) ;      // quantity
  Display.txt_Width(1) ;
  Display.txt_Height(1) ;

  //LedDigitsDisplay(Display, hndl, 61, iLeddigits2+1, 68, 3, 1, 26, 1) ;  // qty_done
  //Display.img_Show(hndl,iStatictext4) ;  // slash_string
  //LedDigitsDisplay(Display, hndl, 289, iLeddigits3+1, 156, 3, 1, 26, 1) ;  // qty_total

}

void speedDisplay(int start_height=92) {
  //printToScreen("speed:", 30, 20);
  Display.txt_FontID(FONT3);
  Display.txt_Width(2) ;
  Display.txt_Height(2) ;
  Display.txt_FGcolour(WHITE) ;
  Display.txt_BGcolour(BLACK) ;
  Display.gfx_MoveTo(txt_start_left, start_height) ;
  Display.putstr("Speed : 24.3%") ;      // speed_label
  Display.txt_Width(1) ;
  Display.txt_Height(1) ;

  //LedDigitsDisplay(Display, hndl, 248, iLeddigits4+1, 156, 3, 2, 26, 1) ;  // speed_digit

}

void lengthDisplay(int start_height=50) {
  Display.txt_FontID(FONT3);
  Display.txt_Width(2) ;
  Display.txt_Height(2) ;
  Display.txt_FGcolour(WHITE) ;
  Display.txt_BGcolour(BLACK) ;
  Display.gfx_MoveTo(txt_start_left, start_height) ;
  Display.putstr("Size : 27.5cm") ;      // length_label
  Display.txt_Width(1) ;
  Display.txt_Height(1) ;

}

void homeWindow() {
  // displaye the home window
  Display.gfx_Cls();   // clear screen

  stopStartButtonDispaly();
  //Display.gfx_Panel(PANEL_RAISED, 10, 172, 60, 60, LIGHTSLATEGRAY) ;  // display speed adjust button
  //Display.gfx_Panel(PANEL_RAISED, 90, 172, 60, 60, LIGHTSLATEGRAY) ;  // display qty button
  //Display.gfx_Panel(PANEL_RAISED, 170, 172, 60, 60, LIGHTSLATEGRAY) ;  // display length button
  Display.img_SetWord(hndl, iUserbutton1, IMAGE_INDEX, 0); // speedButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton1) ;  // speedButton
  Display.img_SetWord(hndl, iUserbutton2, IMAGE_INDEX, 0); // lengthButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton2) ;  // lengthButton
  Display.img_SetWord(hndl, iUserbutton3, IMAGE_INDEX, 0); // quantityButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton3) ;  // quantityButton

  qtyDisplay("Qty : 97/154");
  qtyDisplay("Qty : 97/154", 60);
  //speedDisplay();
  //lengthDisplay();
}

void speedWindow() {
  // displaye the menu window to adjust the motor speed
  Display.gfx_Cls();   // clear screen

  stopStartButtonDispaly();
  //homeButtonDisplay(); // display home button

  // Form2 1.1 generated 7/13/2017 10:44:54 AM
  
  // Form2 1.1 generated 7/13/2017 2:26:23 PM
  LedDigitsDisplay(Display, hndl, 7.1, iLeddigits1+1, 74, 4, 2, 30, 1) ;  // Leddigits1
  LedDigitsDisplay(Display, hndl, 53.2, iiCustomdigits1, 44, 4, 1, 9, 0) ;  // Customdigits1
  Display.img_SetWord(hndl, iUserbutton4, IMAGE_INDEX, 0); // homeButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton4) ;  // homeButton

  speedDisplay();
}

void lengthWindow() {
  // displaye the menu window to adjust the length of the plastic to cut
  Display.gfx_Cls();   // clear screen

  stopStartButtonDispaly();
  homeButtonDisplay(); // display home button

  lengthDisplay();
}

void qtyWindow() {
  // displaye the menu window to adjust the quantity of plastic pieces to cut
  Display.gfx_Cls();   // clear screen

  stopStartButtonDispaly();
  homeButtonDisplay(); // display home button

  //qtyDisplay();
}

void setDisplay() {
  // sets the display based on the current menu state
  switch(menu_state){
    case MENU_SPEED:
      speedWindow();
      break;
    case MENU_LENGTH:
      lengthWindow();
      break;
    case MENU_QTY:
      qtyWindow();
      break;
    default:
      homeWindow();
  }
}

