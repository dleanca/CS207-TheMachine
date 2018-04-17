/*
 * Name: Denis Leanca
 * Class : CS_207 at the University of Regina * 
 * 
 * This is an edit by Denis Leanca of the code from the youtube channel "Must Love Science"
 * The changes implemented are:
 *                          a cheaper LCD was implemented with the library included
 *                          The stripping functionality was added by appending a new state in the existing state machine and the cutting state was modified accordingly
 *                          A sense wire was added that needs to be connected to the cutting claw and ground connected to the spool (or length ) of the wire being used. 
 *                          This way, when the cutting blade cuts through the insulation, it does not go further.
 *  Materials Used:                        
 *                  Arduino Uno,
 *                  10k resistor
 *                  Nema17 stpeper motor
 *                  A4988 driver board
 *                  DC to DC variable power supply converter
 *                  a 1602 LCD character display
 *                  4 push buttons
 *                  proto boards
 *                  jumper wires
 *                  
 *                          
 */

//------------------------------- librarys ----------------------------------
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
#include <Servo.h>



//------------------------------- lcd ----------------------------------

LiquidCrystal lcd(1, 2, 4, 5, 6, 7); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7) 

//------------------------------- stepper ----------------------------------
#define stepPin 12
#define dirPin 13

//------------------------------- servo ----------------------------------
Servo snippers;
#define servo 0
#define openAngle 150
#define closedAngle 100

//------------------------------- input ----------------------------------

#define leftButton 8
#define rightButton 9
#define upButton 10
#define downButton 11

#define senseWire 3

//------------------------------- user settings ----------------------------------
unsigned int wireLength = 0;
unsigned int wireQuantity = 0;
unsigned int stripLength = 0;

//------------------------------- system settings ----------------------------------
int state = 0;
int incrementSpeed = 1;
int previousWireLength = 0;
int previousWireQuantity = 0;
int previousStripLength =0;
//change the mmPerStep thing ?
float mmPerStep = 0.18096;


void setup() {
  
  lcd.begin(16, 2); //LCD columns and rows

  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  pinMode(senseWire, INPUT_PULLUP);

  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);

  snippers.attach(servo);

  snippers.write(openAngle);
    
  delay(1000);
}

void loop() {


 lcd.clear(); // Clears the display 
  if (!digitalRead(rightButton)){
    if(state == 6){
      state = 0;
    }
    else{
      state += 1;
    }
    delay(200);
    lcd.clear();
  }
  
  if (!digitalRead(leftButton) && state > 0 && state < 5){
    state -=1;
    delay(200);
    lcd.clear();
  }


  switch (state){
    case 0:
      homeScreen();
      break;
    case 1:
       chooseWireLength();
       break;
    case 2:
      chooseWireQuantity();
      break;
    case 3:
      chooseStripLength();
      break;
    case 4:
      confirm();
      break;
    case 5:
      currentlyCutting();
      break;
    case 6:
      finishedCutting();
      break;
  }
 
}


void homeScreen(){
  lcd.setCursor(0, 0);
  lcd.print("WIRE CUTTER");
  lcd.setCursor(11, 1);
  lcd.print("NEXT>");
  delay(100);
}


void chooseWireLength(){
  wireLength = changeValue(wireLength);

  //clear LCD if required
  if(previousWireLength != wireLength){
    lcd.clear();
    previousWireLength = wireLength;
  }
 
  //Display information on LCD
  lcd.setCursor(0, 0);
  lcd.print("WIRE_L:" + (String)wireLength + "mm");
  displayNavigation();
}

void chooseWireQuantity(){
  wireQuantity = changeValue(wireQuantity);

  //clear LCD if required
  if(previousWireQuantity != wireQuantity){
    lcd.clear();
    previousWireQuantity = wireQuantity;
  }
 
  //Display information on LCD
  lcd.setCursor(0, 0);
  lcd.print("WIRE_Q:" + (String)wireQuantity);
  displayNavigation();
}

void chooseStripLength(){

  stripLength = changeValue(stripLength);

  //clear LCD if required
  if(previousStripLength != stripLength){
    lcd.clear();
    previousStripLength = stripLength;
  }
 
  //Display information on LCD
  lcd.setCursor(0, 0);
  lcd.print("STRIP_L:" + (String)stripLength + "mm");
  displayNavigation();
}


void confirm(){
  lcd.setCursor(0, 0);
  lcd.print((String)wireLength + "mm x " + (String)wireQuantity + "pcs");
  lcd.setCursor(0, 1);
  lcd.print("<BACK");
  lcd.setCursor(10, 1);
  lcd.print("START>");
  delay(100);
}

void currentlyCutting(){
  lcd.setCursor(0, 0);
  lcd.print((String)0 + "/" + (String)wireQuantity);
  lcd.setCursor(0, 1);
  lcd.print("???s");
  int snipperStep =0;
  int stepsToTake = (int)wireLength/mmPerStep;
  int stripStepsToTake = (int)stripLength/mmPerStep;
  
  for(int i = 0; i < wireQuantity; i++){
    unsigned long timeForOneCycle = millis();

    if(stripLength !=0)
    {
      //subtracting the length of the strip on both sides
      stepsToTake -=stripStepsToTake;
      stepsToTake -=stripStepsToTake;

      //rolls out strip length
      digitalWrite(dirPin,HIGH);
      for(int x = 0; x < stripStepsToTake; x++) 
      {
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500); 
      }

      //this cuts until it touches the wire
      snipperStep =0;
      while((digitalRead(A0) != 0) && ((openAngle-snipperStep) > closedAngle))
      {
        snippers.write(openAngle-snipperStep);
        snipperStep++;
        delay(15);
      }
      snipperStep = 0;
      snippers.write(openAngle);
      delay(600);

      // rolls out the length of the wire
      digitalWrite(dirPin,HIGH);
      for(int x = 0; x < stepsToTake; x++) 
      {
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500); 
      }

      //this cuts until it touches the wire
      snipperStep =0;
      while((digitalRead(A0) != 0) && ((openAngle-snipperStep) > closedAngle))
      {
        snippers.write(openAngle-snipperStep);
        snipperStep++;
        delay(15);
      }
      snipperStep = 0;
      snippers.write(openAngle);
      delay(600);

       //rolls out strip length
      digitalWrite(dirPin,HIGH);
      for(int x = 0; x < stripStepsToTake; x++) 
      {
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500); 
      }

      snippers.write(closedAngle);
      delay(600);
      snippers.write(openAngle);
      delay(600);
    }
    
    else
    {
      
    digitalWrite(dirPin,HIGH);
    for(int x = 0; x < stepsToTake; x++) {
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(500); 
    }
    
    lcd.setCursor(0, 0);
    lcd.print((String)(i+1) + "/" + (String)wireQuantity);
    
    snippers.write(closedAngle);
    delay(600);
    snippers.write(openAngle);
    delay(600);

    }

    
    lcd.setCursor(0, 1);

    unsigned long timeRemaining = ((millis() - timeForOneCycle)*(wireQuantity - (i+1)))/1000;
    lcd.print((String)timeRemaining + "s    ");
    
  }
  wireLength = 0;
  wireQuantity = 0;
  stripLength = 0;
  state = 6;
}

void finishedCutting(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CUTTING COMPLETE");
  lcd.setCursor(11, 1);
  lcd.print("NEXT>");
  delay(100);
}


int changeValue(int currentValue){
  if (!digitalRead(upButton)) {
    currentValue += incrementSpeed;
    incrementSpeed ++;
  }
  if (!digitalRead(downButton)) {
    if(currentValue - incrementSpeed >= 0){
      currentValue -= incrementSpeed;
      incrementSpeed ++;
    }
    else{
      currentValue = 0;
    }
  }
  if (digitalRead(downButton) && digitalRead(upButton)){
    incrementSpeed = 1;
  }
  return currentValue;
}

void displayNavigation(){
  lcd.setCursor(0, 1);
  lcd.print("<BACK");
  lcd.setCursor(11, 1);
  lcd.print("NEXT>");
  delay(100);
}
