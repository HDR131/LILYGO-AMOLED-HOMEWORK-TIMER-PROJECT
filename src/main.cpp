#define LILYGO_TDISPLAY_AMOLED_SERIES

#include "AceButton.h"
#include "esp_arduino_version.h"
#include <LilyGo_AMOLED.h> 
#include <TFT_eSPI.h>
#include "true_color.h"
#include <LV_Helper.h>

// include images to push to sprite
#include "tomato_img.h"
#include "tomato_Large_img.h"
#include "stopwatch_img.h"
#include "break_img.h"
#include "zzz_img.h"

////////////////////////////// TFT & amoled Initalizations //////////////////////////////
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
TFT_eSprite tmtsml = TFT_eSprite(&tft);
TFT_eSprite tmtlrg = TFT_eSprite(&tft);
TFT_eSprite stpwtch = TFT_eSprite(&tft);
TFT_eSprite brklrg = TFT_eSprite(&tft);
TFT_eSprite zzzsml = TFT_eSprite(&tft);

#define color1 0xDA65 // dark orange color 
#define color2 0xDB29 // light orange color 
#define color3 0xFFDD // off white 
#define color4 0x0841 // black
#define color5 0xB182 // dark orange color 
#define color6 0x52F7 // blue-violet color 
#define color7 0x4A94 // dark blue-violet color 
#define color8 0x73DA // bright blue-violet color

LilyGo_Class amoled;

#define WIDTH amoled.height()
#define HEIGHT amoled.width()

////////////////////////////// Button Initalization ////////////////////////////////////

using namespace ace_button;\
const int BUTTON_PIN = 21; 
AceButton button(BUTTON_PIN); 

////////////////////////////// Timer Initalization /////////////////////////////////////
hw_timer_t *Timer0 = NULL; 
extern volatile int seconds = 25 * 60; // 25 minute timer count in seconds 
hw_timer_t *Timer1 = NULL;
///////////// Timer Values 
extern volatile int img_pos1 = 50; // large tomato image starting position 
extern volatile bool upflag = false; // large tomato image direction flag 


extern volatile int img_pos2 = 13; // small left tomato image starting position 
extern volatile bool tmt1flag = false;  // small left tomato direction flag 

extern volatile int img_pos3 = 18; // small right tomato starting position 
extern volatile bool tmt2flag = true; // small right tomato direction flag 
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;


//////////////////////////////////// Timer Counter ////////////////////////////////////
void IRAM_ATTR timer0Interrupt() {
  portENTER_CRITICAL_ISR(&timerMux0);
  seconds --;
  portEXIT_CRITICAL_ISR(&timerMux0);
}

//////////////////////////////// Move Sprite Interrupt ////////////////////////////////
void IRAM_ATTR timer1Interrupt() {
  portENTER_CRITICAL_ISR(&timerMux1);
  
  // move large tomato condtional 
  if (upflag == false) { // if flag false add 1 to img position (move downward on screen)
    img_pos1++;
    if (img_pos1 == 120) { // make lower bound, if position hits 120 reverse direction
      upflag = true;
    }
  }
  else { // if flag true subtract 1 from position (move upward on screen)
    img_pos1--;
    if (img_pos1 == 65){ // make upper bound, if position hits 65 reverse direction
      upflag = false;
    }
  }

  // move tomato left of Work Period, operates the same as above function 
  if (tmt1flag == false) {
    img_pos2--;
    if (img_pos2 < 10) {
      tmt1flag = true;
    }
  }
  else {
    img_pos2++;
    if (img_pos2 > 30) {
      tmt1flag = false;
    }
  }

    // move rtomato right of Work Period, operates the same as above fucntion 
    if (tmt2flag == false) {
    img_pos3--;;
    if (img_pos3 < 10) {
      tmt2flag = true;
    }
  }
  else {
    img_pos3++;
    if (img_pos3 > 30) {
      tmt2flag = false;
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux1);
}


////////////////////////// Begin Both Timers Init Function //////////////////////////
void Timer_INIT() {

  // Timer 1 initaliztion 
  Timer0 = timerBegin(0,80,true);
  timerAttachInterrupt(Timer0, timer0Interrupt, true);
  int ticks_in_second = 1000000;
  timerAlarmWrite(Timer0, ticks_in_second, true);

  //timer 2 initalization 
  Timer1 = timerBegin(1,80,true);
  timerAttachInterrupt(Timer1, timer1Interrupt, true);
  int ticks_per_pixel_adj = 100000;
  timerAlarmWrite(Timer1, ticks_per_pixel_adj, true);
  
}

//////////////////////////// Work Timer Screen Function ////////////////////////////
void DrawTimerWork() {

  // create the small tomato sprite 
  tmtsml.createSprite(32,32);
  tmtsml.setSwapBytes(false);
  tmtsml.pushImage(0, 0, 32, 32, tomato_img); 

  // create large tomato sprite 
  tmtlrg.createSprite(126,126);
  tmtlrg.setSwapBytes(false);
  tmtlrg.pushImage(0, 0, 126, 126, tomato_Large_img);

  // create stopwatch sprite 
  stpwtch.createSprite(64,64);
  stpwtch.setSwapBytes(true);
  stpwtch.pushImage(0, 0, 64, 64, stopwatch_img);

  // redeclare starting seconds variable for 25 minute countown
  seconds = 25 * 60; 

  // Begin Timers, timer restart and enable 
  timerRestart(Timer0);
  timerRestart(Timer1);
  timerAlarmEnable(Timer0);
  timerAlarmEnable(Timer1);

  // work period countdown timer loop 
  while (seconds >= 0) {

    // draw bacground rectangle and two spheres 
    spr.fillRect(0, 0, WIDTH, HEIGHT, color1);
    spr.fillCircle(350, 135, 215, color5);
    spr.fillCircle(0, 20, 72, color2);
    
    // create the work period lable string/sprite
    spr.setTextColor(color3);
    String wrktxt = "...Work Period...";
    spr.drawString(wrktxt, 200, 25, 4);

    // create timer outline round rectangles 
    spr.fillRoundRect(85, 85, 310, 135, 25, color3);
    spr.fillRoundRect(90, 90, 300, 125, 18, color2);

    // convert second and minute values 
    int s = seconds % 60;
    int m = (seconds % 3600) / 60;
    int s_size = std::to_string(s).length();

    // create timer number string for minutes 
    String m_str = String(m);
    // if statment to keep alignment if single digit minutes 
    if (m_str.length() == 1) {
      m_str = "0" + m_str; 
    }
    // create timer number string for seconds 
    String s_str = String(s); 
    // if statment to keep alignment if single digits seconds 
    if (s_str.length() == 1) {
      s_str = "0" + s_str;
    }
    // create total time string 
    String currTime = m_str + ":" + s_str; 
    spr.setTextColor(color3, color2);
    spr.drawString(currTime, 115, 110, 8);

    // push tomato sprite left of "Work Period" to the sprite class &spr 
    tmtsml.pushToSprite(&spr, 165, img_pos2, TFT_BLACK);

    // push tomato sprite right of "Work Period" to &spr
    tmtsml.pushToSprite(&spr, 382, img_pos3, TFT_BLACK);

    // Push large tomato sprite to &spr
    tmtlrg.pushToSprite(&spr, 400, img_pos1, TFT_BLACK);

    // Push stopwatch sprite to &spr 
    stpwtch.pushToSprite(&spr, 10, 120, TFT_BLACK);

    // push the all drawn sprites to the screen
    amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
  }

  timerAlarmDisable(Timer0);

}

///////////////////////// 5 MinuteBreak Timer Screen Function /////////////////////////

void DrawTimer5Break() {

  // redefine starting seconds varibale for countdown 
  seconds = 5 * 60;

  // create stopwatch sprite 
  stpwtch.createSprite(64,64);
  stpwtch.setSwapBytes(true);
  stpwtch.pushImage(0, 0, 64, 64, stopwatch_img);

  // create large break image sprite 
  brklrg.createSprite(128,128);
  brklrg.setSwapBytes(false);
  brklrg.pushImage(0, 0, 128, 128, break_img);

  // create zzz image sprite 
  zzzsml.createSprite(32, 32);
  zzzsml.setSwapBytes(false);
  zzzsml.pushImage(0, 0, 32, 32, zzz_img);


  // Begin Timers, timer restart and enable 
  timerRestart(Timer0);
  timerRestart(Timer1);
  timerAlarmEnable(Timer0);
  timerAlarmEnable(Timer1);

  // 5 Min Break Period countdown timer loop 
  while (seconds >= 0) {

    // Draw background 
    spr.fillRect(0, 0, WIDTH, HEIGHT, color6);
    spr.fillCircle(20, 100, 95, color7);
    spr.fillCircle(340, 190, 150, color8);

    // create the break period lable string/sprite
    spr.setTextColor(color3);
    String wrktxt = "...Break Period...";
    spr.drawString(wrktxt, 200, 25, 4);

    // create timer outline round rectangles 
    spr.fillRoundRect(85, 85, 310, 135, 25, color3);
    spr.fillRoundRect(90, 90, 300, 125, 18, color7);

    // convert second and minute values 
    int s = seconds % 60;
    int m = (seconds % 3600) / 60;
    int s_size = std::to_string(s).length();

    // create timer number string for minutes 
    String m_str = String(m);
    // if statment to keep alignment if single digit minutes 
    if (m_str.length() == 1) {
      m_str = "0" + m_str; 
    }
    // create timer number string for seconds 
    String s_str = String(s); 
    // if statment to keep alignment if single digits seconds 
    if (s_str.length() == 1) {
      s_str = "0" + s_str;
    }
    // create total time string 
    String currTime = m_str + ":" + s_str; 
    spr.setTextColor(color3, color7);
    spr.drawString(currTime, 115, 110, 8);

    // Push stopwatch sprite to &spr 
    stpwtch.pushToSprite(&spr, 10, 120, TFT_BLACK);

    // Push large break sprite to &spr
    brklrg.pushToSprite(&spr, 400, img_pos1, TFT_BLACK);

    // push zzz sprite left of "Break Period" to the sprite class &spr 
    zzzsml.pushToSprite(&spr, 165, img_pos2, TFT_BLACK);

    // push zzz sprite right of "Break Period" to &spr
    zzzsml.pushToSprite(&spr, 390, img_pos3, TFT_BLACK);

    // push all drawn sprites to the screen
    amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
  }

  timerAlarmDisable(Timer0);
}

////////////////////////////////////////////////////////////////////////////////////////

///////////////////////// 15 MinuteBreak Timer Screen Function /////////////////////////

void DrawTimer15Break() {

  // redefine starting seconds varibale for countdown 
  seconds = 15 * 60;

  // create stopwatch sprite 
  stpwtch.createSprite(64,64);
  stpwtch.setSwapBytes(true);
  stpwtch.pushImage(0, 0, 64, 64, stopwatch_img);

  // create large break image sprite 
  brklrg.createSprite(128,128);
  brklrg.setSwapBytes(false);
  brklrg.pushImage(0, 0, 128, 128, break_img);

  // create zzz image sprite 
  zzzsml.createSprite(32, 32);
  zzzsml.setSwapBytes(false);
  zzzsml.pushImage(0, 0, 32, 32, zzz_img);


  // Begin Timers, timer restart and enable 
  timerRestart(Timer0);
  timerRestart(Timer1);
  timerAlarmEnable(Timer0);
  timerAlarmEnable(Timer1);

  // 5 Min Break Period countdown timer loop 
  while (seconds >= 0) {
    
    // Draw background 
    spr.fillRect(0, 0, WIDTH, HEIGHT, color6);
    spr.fillCircle(20, 100, 95, color7);
    spr.fillCircle(340, 190, 150, color8);

    // create the break period lable string/sprite
    spr.setTextColor(color3);
    String wrktxt = "...Break Period...";
    spr.drawString(wrktxt, 200, 25, 4);

    // create timer outline round rectangles 
    spr.fillRoundRect(85, 85, 310, 135, 25, color3);
    spr.fillRoundRect(90, 90, 300, 125, 18, color7);

    // convert second and minute values 
    int s = seconds % 60;
    int m = (seconds % 3600) / 60;
    int s_size = std::to_string(s).length();

    // create timer number string for minutes 
    String m_str = String(m);
    // if statment to keep alignment if single digit minutes 
    if (m_str.length() == 1) {
      m_str = "0" + m_str; 
    }
    // create timer number string for seconds 
    String s_str = String(s); 
    // if statment to keep alignment if single digits seconds 
    if (s_str.length() == 1) {
      s_str = "0" + s_str;
    }
    // create total time string 
    String currTime = m_str + ":" + s_str; 
    spr.setTextColor(color3, color7);
    spr.drawString(currTime, 115, 110, 8);

    // Push stopwatch sprite to &spr 
    stpwtch.pushToSprite(&spr, 10, 120, TFT_BLACK);

    // Push large break sprite to &spr
    brklrg.pushToSprite(&spr, 400, img_pos1, TFT_BLACK);

    // push zzz sprite left of "Break Period" to the sprite class &spr 
    zzzsml.pushToSprite(&spr, 165, img_pos2, TFT_BLACK);

    // push zzz sprite right of "Break Period" to &spr
    zzzsml.pushToSprite(&spr, 390, img_pos3, TFT_BLACK);

    // push all drawn sprites to the screen
    amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
  }

  timerAlarmDisable(Timer0);
}

////////////////////////////// Button Event //////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  // initalizations 
  amoled.beginAMOLED_191(); // begin screen
  spr.createSprite(WIDTH, HEIGHT); // create whole screen sprite 
  spr.setSwapBytes(1);
  unsigned int seed = analogReadMilliVolts(PIN_D0); // get a random seed 
  srand(seed); // set random seed 
  Timer_INIT(); // prepare timers 
}

///////////////////////////////////////////////////////////////////////////////////////

void loop() {
  // put your main code here, to run repeatedly:
  // I could make this look more refined with switch statements, but this looks fine and is readable? 
  DrawTimerWork();
  DrawTimer5Break();
  DrawTimerWork();
  DrawTimer5Break();
  DrawTimerWork();
  DrawTimer5Break();
  DrawTimerWork();
  DrawTimer15Break();
}

// put function definitions here:
