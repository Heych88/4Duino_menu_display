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
#include "XYposToDegree.h"
#include "Picaso_KBRoutines.h"
#include "Picaso_Const4D.h"

Picaso_Serial_4DLib Display(&DisplaySerial);

#include <string.h>

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
#define MENU_DELAY 4  // state to delay the home menu reset

#define MENU_RESET_DELAY 60000 // delay time when the menu resets to the home screen in ms

bool system_stop;
char menu_state;
long timer;
word hFont1, hstrings;
int txt_start_x = 8; // left offset of text
int txt_start_y = 8; // top offset of text
int total_qty = 1;  // total number of plastic labels to cut
int current_qty = 0; // current count of how many labels have been cut
float current_speed = 0;
float desired_speed = 0; // target running speed
float cut_length =0; // length to cut each plastic piece

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
  Display.img_ClearAttributes(hndl, iWinbutton1, I_TOUCH_DISABLE); // startButton set to enable touch, only need to do this once
  Display.img_ClearAttributes(hndl, iWinbutton2, I_TOUCH_DISABLE); // stopButton set to enable touch, only need to do this once

} // end Setup **do not alter, remove or duplicate this line**

int n;
void loop()
{
  // put your main code here, to run repeatedly:

  byte state ;
  short x, y;
  while(1) {
    state = Display.touch_Get(TOUCH_STATUS); // get touchscreen status
    n = Display.img_Touched(hndl,-1) ;

    //-----------------------------------------------------------------------------------------
    if(state == TOUCH_PRESSED) { // if there's a press, or it's moving
      x = Display.touch_Get(TOUCH_GETX);
      y = Display.touch_Get(TOUCH_GETY);
      timer = millis();  // reset the dispaly timeout timer

      // This is for the keydown event
      if ((n >= iKeyboard1) && (n <= iKeyboard1+oKeyboard1[KbButtons])){
        kbDown(Display, hndl, iKeyboard1, oKeyboard1, iKeyboard1keystrokes, n-iKeyboard1, KbHandler) ;  // Keyboard1
        //kbDown(Display, hndl, ikeyboard1, okeyboard1, ikeyboard1keystrokes, n-ikeyboard1, KbHandler) ;
      }
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
        stopStartButtonDispaly();
      } else if (oKeyboard1[KbDown] != -1) {
        // This is for the keyup event
        kbUp(Display, hndl, iKeyboard1, oKeyboard1) ;  // Keyboard1
     } else if (menu_state == HOME) {
        if((x >= 10) && (x <= 70) && (y >= 172) && (y <= 232)){
          // Speed menu button pressed
          menu_state = MENU_SPEED;
          setDisplay();
        } //else {
          //menu_state = HOME;
        //}

       // only access the menus from the home state
        else if((x >= 90) && (x <= 150) && (y >= 172) && (y <= 232)){
          // length menu button pressed
          menu_state = MENU_LENGTH;
          setDisplay();
        }
        else if((x >= 170) && (x <= 230) && (y >= 172) && (y <= 232)){
          // length menu button pressed
          menu_state = MENU_QTY;
          setDisplay();
        }
      } else if (menu_state == MENU_DELAY) {
        menu_state = HOME; // button is released now reset to the home state
        delay(200);
        Display.gfx_Cls();   // clear screen
        setDisplay();
        // disable and clear the kewboard1
        for (uint8_t i = iKeyboard1+1; i <= iKeyboard1+oKeyboard1[KbButtons]; i++){
          Display.img_SetAttributes(hndl, i, I_TOUCH_DISABLE); // Keyboard1 set to enable touch, only need to do this once  next
        }
      }
    }
    if((menu_state != HOME) && (millis() - timer >= MENU_RESET_DELAY)){
      menu_state = MENU_DELAY;
      setDisplay();
      //timer = millis();
    }
  }
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
  //Display.img_SetWord(hndl, iUserbutton4, IMAGE_INDEX, 0); // homeButton where state is 0 for up and 1 for down, or 2 total states
  //Display.img_Show(hndl,iUserbutton4) ;  // homeButton
}

void printToScreen(char* str, short x_pos=txt_start_x, short y_pos=txt_start_y) {
  //Display.txt_BGcolour(SILVER);
  Display.txt_FontID(FONT3);
  Display.txt_Width(2) ;
  Display.txt_Height(2) ;
  Display.txt_FGcolour(WHITE) ;
  Display.txt_BGcolour(BLACK) ;
  Display.gfx_MoveTo(x_pos, y_pos) ;
  Display.putstr(str) ;      // quantity
  Display.txt_Width(1) ;
  Display.txt_Height(1) ;
}

void homeWindow() {
  // displaye the home window
  
  // Form1 1.1 generated 14-Jul-17 3:38:16 PM
  Display.gfx_Cls();   // clear screen

  stopStartButtonDispaly();

  Display.img_ClearAttributes(hndl, iUserbutton1, I_TOUCH_DISABLE); // speedButton set to enable touch,
  Display.img_ClearAttributes(hndl, iUserbutton2, I_TOUCH_DISABLE); // lengthButton set to enable touch,
  Display.img_ClearAttributes(hndl, iUserbutton3, I_TOUCH_DISABLE); // quantityButton set to enable touch,

  Display.img_SetWord(hndl, iUserbutton1, IMAGE_INDEX, 0); // speedButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton1) ;  // speedButton
  Display.img_SetWord(hndl, iUserbutton2, IMAGE_INDEX, 0); // lengthButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton2) ;  // lengthButton
  Display.img_SetWord(hndl, iUserbutton3, IMAGE_INDEX, 0); // quantityButton where state is 0 for up and 1 for down, or 2 total states
  Display.img_Show(hndl,iUserbutton3) ;  // quantityButton

  //add the quantity values to the display
  char str[50];
  sprintf( str, "Qty : %d/%d", current_qty, total_qty);
  printToScreen(str);  // quantity update
  //add the current motor speed to the display
  sprintf(str, "Speed : %d", current_speed);
  printToScreen(str, txt_start_x, txt_start_x+40);  // quantity update
  //add the set cutting length to the display
  sprintf(str, "size : %d", cut_length);
  printToScreen(str, txt_start_x, txt_start_x+80);  // quantity update
}

void keyFormDisplay() {
  //Display.gfx_Cls();   // clear screen
  //Display.img_SetAttributes(hndl, iForm1, I_TOUCH_DISABLE);
  Display.img_SetAttributes(hndl, iUserbutton1, I_TOUCH_DISABLE); // speedButton set to enable touch, only need to do this once
  Display.img_SetAttributes(hndl, iUserbutton2, I_TOUCH_DISABLE); // lengthButton set to enable touch, only need to do this once
  Display.img_SetAttributes(hndl, iUserbutton3, I_TOUCH_DISABLE); // quantityButton set to enable touch, only need to do this once

  
  // Form2 1.1 generated 14-Jul-17 3:38:16 PM
  Display.gfx_Cls();   // clear screen
  Display.touch_Set(TOUCH_ENABLE);

  stopStartButtonDispaly();

  Display.img_Show(hndl,iKeyboard1) ; // Keyboard1 show initial keyboard
  for (uint8_t i = iKeyboard1+1; i <= iKeyboard1+oKeyboard1[KbButtons]; i++){
      Display.img_ClearAttributes(hndl, i, I_TOUCH_DISABLE); // Keyboard1 set to enable touch, only need to do this once  next
  }
}

void speedWindow() {
  // displaye the menu window to adjust the motor speed
  keyFormDisplay();


  //add the current motor speed to the display
  char str[50];
  sprintf(str, "Speed : %d", current_speed);
  printToScreen(str);  // speed update



}

void lengthWindow() {
  // displaye the menu window to adjust the length of the plastic to cut
  keyFormDisplay();

  //add the set cutting length to the display
  char str[50];
  sprintf(str, "size : %d", cut_length);
  printToScreen(str);  // quantity update

}

void qtyWindow() {
  // displaye the menu window to adjust the quantity of plastic pieces to cut
  keyFormDisplay();

  //add the quantity values to the display
  char str[50];
  sprintf( str, "Total : %d", total_qty);
  printToScreen(str);  // quantity update

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

int row, col;

void KbHandler(int Key)
{
  char str[50];

  if(Key == 11) {
    menu_state = MENU_DELAY;
  } else if(Key == 10) {
    Key = 0;
  }

  switch(menu_state){
    case MENU_SPEED:
      sprintf(str, "Speed : %d", Key);
      printToScreen(str);  // speed update
      break;
    case MENU_LENGTH:
      sprintf(str, "Size : %d", Key);
      printToScreen(str);  // speed update
      break;
    case MENU_QTY:
      sprintf(str, "Total : %d", Key);
      printToScreen(str);  // speed update
      break;
    case MENU_DELAY:
      break;
    //default:    */
  }
}

