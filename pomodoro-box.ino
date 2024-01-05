#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

uint32_t studyDuration = 5;//length of study time (units: seconds)
uint32_t relaxDuration = 5;// length of relax time (units: seconds)
uint32_t currDuration = studyDuration; // length of time of the current mode we are in
float additionalRelax = 0;
uint32_t currTime = 0;//time elapsed (resets when switch modes)
float relaxToStudyRatio = relaxDuration/studyDuration;
bool isStudy = true; //true if we are in study mode
bool refreshDisplay= true; //true we we need to update the display
bool phoneDetected = true;

uint8_t dispHour;
uint8_t dispMin;
uint8_t dispSec;


hw_timer_t *My_timer = NULL;
void IRAM_ATTR onTimer(){
  currTime += 1;
  //if we having finished are studying session, but we are still going
  //add the appropriate amount of time to the next relax session to keep the
  //ratio the same
  if(currTime > currDuration && isStudy){
    additionalRelax += relaxToStudyRatio;
  }
  refreshDisplay = true;
}


void setup(){
  Serial.begin(115200);
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  //begin timer interrupt that is triggered every second
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer); //Just Enable
}

void loop(){
  //switch modes if the time is past the amount we have set for that mode
  //including any additional relax time, but only accounting that in relax mode
  if(currTime > currDuration + (isStudy ? 0 : additionalRelax) && ((!isStudy && phoneDetected) || isStudy)){
    isStudy = !isStudy;
    currDuration = isStudy ? studyDuration : relaxDuration;
    if(isStudy){
      additionalRelax = 0;
    }
    currTime = 0;
    refreshDisplay = true;
  }

  if (refreshDisplay){
    // set cursor to first column, first row
    lcd.setCursor(0, 0);

    if (!isStudy && currTime > currDuration + additionalRelax && !phoneDetected){
      lcd.print("Put Back Phone");
    }
    lcd.clear();
    int timeDiff = currDuration + (isStudy ? 0 : additionalRelax) - currTime;
    formatTime(abs(timeDiff), dispHour, dispMin, dispSec);
    String headerStr = isStudy ? "Study: " : "Relax: ";
    if (timeDiff < 0){
      headerStr += "+";
    }
    String hourStr = (String) dispHour;
    String minutesStr = (dispMin < 10 ? "0": "" )+  (String) dispMin; 
    String secondsStr = (dispSec < 10 ? "0": "" )+  (String) dispSec; 
    
    // print message
    lcd.print(headerStr + (dispHour != 0 ? hourStr + ":": "") + minutesStr + ":"+secondsStr);

    refreshDisplay=false;


  }
}

void formatTime(uint32_t totalSeconds, uint8_t &hours, uint8_t &minutes, uint8_t &seconds) {
  hours = totalSeconds / 3600;
  totalSeconds %= 3600;
  minutes = totalSeconds / 60;
  seconds = totalSeconds % 60;
}