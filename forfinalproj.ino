
/*
FOR THE COMPLETION OF THE REQUIREMENTS FOR THE SUBJECT CPE330L
FINAL PROJECT PROPOSED BY:
1.) Eldan Rey Dofredo       - 3rd Year BSEE/CEE Student
2.) Edcir Jay Benitez       - 3rd Year BSEE/CEE Student
3.) Lloyd Greg Jaromahum    - 3rd Year BSEE/CEE Student

*/



//===============================LIBRARY INCLUDED FOR THE PROJECT========================================//

#include <Wire.h>                   //LIBRARY USED TO COMMUNICATE WITH THE I2C IN THE LCD
#include <LiquidCrystal_I2C.h>      //LIBRARY TO USE LCD FUNCTIONS
#include <EEPROM.h>
#include "DHT.h"                    //LIBRARY FOR DHT11

//=======================================================================================================//



//===========================DEFINING INPUT AND OUTPUT PINS==============================================//


#define DHTTYPE DHT11         
#define SoilMoistureSensor A0           //DEFINING THAT THE SOIL MOISTURE SENSOR IS CONNECTED TO A0 pin
#define Relay1 9                        //DEFINE THE 1st RELAY INPUT TO pin 9
#define Relay2 10                       //DEFINE THE 2nd RELAY INPUT TO pin 10
#define RedLed3 13                       //DEFINE THE RED LED TO PIN 13
#define DHTPIN 2                        //DHT DATA PIN IS ON PIN 2 ON ARDUINO
#define EnterButton 3                   //DEFINE ENTER BUTTON TO PIN 3
#define EditButton 4                    //DEFINE EDIT BUTTON TO PIN 4
#define UpButton 5                      //DEFINE UP BUTTON TO PIN 5
#define DownButton 6                    //DEFINE DOWN BUTTON TO PIN 6
#define BottomFloatSwitch 7                  //DEFINE THE BOTTOM FLOAT SWITCH TO PIN 7    
#define HighFloatSwtich 8                  //DEFINE THE UPPER FLOAT SWITCH TO PIN 8
#define moistureLowerLimitAddress 1     //The moisture lower limit address in the EEPROM will be set to 1 
#define moistureUpperLimitAddress 3     //The moisture upper limit address in the EEPROM will be set to 3 (INTEGER HAS A SIZE OF 2 BYTES therefore we need 2 address for int data type)
#define vpdLowerLimitAddress 5           //THE lower limit for vpd address in EEPROM is set to 5 (FLOAT has a size of 4 bytes therefore we need 4 address slots to store them)
#define vpdUpperLimitAddress 9          //THE upper limit for vpd address in EEPROM is set to 9 



                                        //EVERY PIN IS DEFINED FOR READABILITY

//=======================================================================================================// 




//================================VARIABLE AND SENSORS INITIALIZATION====================================//


float lowerLimitVPD,upperLimitVPD,temp=0,hum=0;
int moistureUpperLimit,moistureLowerLimit,menuState=1;
char L0[17],L1[17];
LiquidCrystal_I2C lcd(0x27, 16, 2);     //INITIALIZE THE LCD_I2C FOUND ADDRESS @ 0x27 WITH SCANNER WITH 16 Columns and 2 Rows
DHT dht(DHTPIN, DHTTYPE);               //INITIALIZE DHT11 Sensor


//=======================================================================================================//




//=============================READS TEMPERATURE AND HUMIDITY TO GET VPD==================================//
float getVPD(){
  hum=dht.readHumidity();                                               //READS THE HUMIDITY USING DHT 11
  temp=dht.readTemperature();                                           //READS THE TEMPERATURE USING DHT11
  float vpsat = 0.6108*exp(((17.27*temp)/(temp+237.3)));                //CALCULATE THE VAPOR PRESSURE (VP) OF THE AIR AT CERTAI TEMPERATURE AND AT SATURATED HUMIDIY
  return(vpsat*((100-hum)/100));                                        //RETURNS THE VPD WHICH IS THE DIFFERENCE OF VP_SATURATED TO VP_ACTUAL THE UNIT IS kilopascal (kPA)
  }
//=======================================================================================================//




//========================GETS THE SOIL MOISTURE FROM SENSOR=============================================//
int getSoilMoisture(){
  int sumSoilMoisture=0;
  for(int i=0;i<16;i++){
    sumSoilMoisture+=analogRead(SoilMoistureSensor);            //AS THE SOIL MOISTURE SENSOR HAS A LOT OF NOISE IN ITS DATA
    _delay_ms(10);                                              //GETTING THE AVERAGE OF 16 READINGS WILL GIVE US LESS NOISE
    }                                                           //THIS WILL GIVE US MORE STABLE DATA FROM THE SENSOR
  return(sumSoilMoisture/16);                                 
  }
//=======================================================================================================//




//============================ARDUINO SETUP FUNCTION=====================================================//
void setup() {
  lcd.begin();                                  //THIS WILL INITIALIZE THE LCD AND START IT
  dht.begin();                                   //INITIALIZE THE DHT
  lcd.backlight();                              //TURNING ON THE BACKLIGHT OF THE LCD
  Serial.begin(9600);
  pinMode(SoilMoistureSensor,INPUT);            //INITIALIZE THE PINMODE OF THE SOIL MOISTURE (A0 pin) and set to INPUT
  for(int i=3;i<=8;i++)pinMode(i,INPUT_PULLUP);
  pinMode(2,INPUT);
  for(int i=11;i<=13;i++)pinMode(i,OUTPUT);
  pinMode(9,OUTPUT);pinMode(10,OUTPUT);
  readVariables();
}
//=======================================================================================================//



//=================================WATER TANK CONTROL====================================================//

void waterTankSystem(){
    bool isTankLow = !digitalRead(BottomFloatSwitch);
    bool isTankFull = digitalRead(HighFloatSwtich);
    if(isTankFull)digitalWrite(Relay1,HIGH);
    else if (isTankLow || !isTankFull){
      digitalWrite(Relay1,LOW);
      }
  }
  
//=======================================================================================================//




//========================IRRIGATION SYSTEM (SOIL MOISTURE AND RELAY)====================================//

void irrigationSystem(){
    int soilMoisture = getSoilMoisture();
    if(soilMoisture<=(moistureUpperLimit+moistureLowerLimit)/2){
      Serial.println("RELAY 2 ON");digitalWrite(Relay2,HIGH);
      }
    else if(soilMoisture>=((moistureUpperLimit+moistureLowerLimit)/2)){
       Serial.println("RELAY 2 OFF");digitalWrite(Relay2,LOW);
      }

    
  }
  
//=======================================================================================================//



//===================================VPD WARNING DETECTION===============================================//

void vpdWarningSystem(){
    float vpd=getVPD();
    if(vpd<=lowerLimitVPD || vpd>=upperLimitVPD){
      digitalWrite(RedLed3,HIGH);
  }
  else {
    digitalWrite(RedLed3,LOW);
    }
  }

//=======================================================================================================//



//==================================Save Values to EEPROM================================================//
void saveCalibrateSMS(int SMSUaddress, int val1, int SMSLaddress, int val2){
  EEPROM.write(SMSUaddress, val1 >> 8);
  EEPROM.write(SMSUaddress + 1, val1 & 0xFF);
  EEPROM.write(SMSLaddress,  val2 >> 8);
  EEPROM.write(SMSLaddress + 1, val2 & 0xFF);
  }
//=======================================================================================================//
//==================================Save Values to EEPROM================================================//
void saveCalibrateVPD(){
  EEPROM.put(vpdLowerLimitAddress, lowerLimitVPD);
  EEPROM.put(vpdUpperLimitAddress, upperLimitVPD);
  }
//=======================================================================================================//



//==========================================Read From EEPROM=============================================//
void readVariables(){
  EEPROM.get(moistureLowerLimitAddress, moistureLowerLimit);
  EEPROM.get(moistureUpperLimitAddress, moistureUpperLimit);
  EEPROM.get(vpdLowerLimitAddress, lowerLimitVPD);
  EEPROM.get(vpdUpperLimitAddress, upperLimitVPD);

//   if the address read is empty or == -1 it will return these default values
  if(moistureLowerLimit<0 || isnan(moistureLowerLimit))moistureLowerLimit=200;
  if(moistureUpperLimit<0|| isnan(moistureUpperLimit) ||moistureUpperLimit>=1000)moistureUpperLimit=400;
  if(lowerLimitVPD<0|| isnan(lowerLimitVPD))lowerLimitVPD=0.4;
  if(upperLimitVPD<0|| isnan(upperLimitVPD))upperLimitVPD=1.6;
  }
//=======================================================================================================//





//==========================================BUTTON PRESSED FUNCTION======================================//
byte buttonPressed(){
  for(int i=3;i<=6;i++)if(!digitalRead(i))return i;
  }
//=======================================================================================================//  






//==========================================PRINT FUNCTIONS=============================================//
void lcdPrint(int col,int row, String stext ){
  char text[17];
  stext.toCharArray(text,17);
    lcd.setCursor(col,row);
    if(row){
      sprintf(L1,"%-16s",text);
      lcd.print(L1);
      }
    else{
      sprintf(L0,"%-16s",text);
      lcd.print(L0);
      }
  }
void lcdPrint(int col,int row, String stext, int val ){
  char text[17];
  stext.toCharArray(text,17);
    lcd.setCursor(col,row);
    if(row){
      sprintf(L1,"%s %-16d",text,val);
      lcd.print(L1);
      }
    else{
      sprintf(L0,"%s %-16d",text,val);
      lcd.print(L0);
      }
  }
void lcdPrint(int col,int row, String stext, float val  ){
  char text[17];
  char float1[6];
  dtostrf(val,1,2,float1);
  stext.toCharArray(text,17);
    lcd.setCursor(col,row);
    if(row){
      sprintf(L1,"%s %s",text,float1);
      lcd.print(L1);
      }
    else{
      sprintf(L0,"%s %s",text,float1);
      lcd.print(L0);
      }
  }  
//=======================================================================================================//


 void lcdClear(){
  lcdPrint(0,0," ");
  lcdPrint(0,1," ");
  }
  


void menuSystem(){
 
  switch(menuState){
    case 1: 
    mainScreen();
    break;
    case 2:
    calibrateSMS();
    break;
    case 3:
    editVPD();
    break;
    }
  
  }


//=======================FUNCTION FOR CALIBRATING SOIL MOISTURE=============================================//
void calibrateSMS(){
  while(1){
    lcdPrint(0,0,"SMULim:",moistureUpperLimit );
    lcdPrint(0,1," ");
    if(buttonPressed()==5){
      moistureUpperLimit++;
      _delay_ms(200);
      }
    else if(buttonPressed()==6){
      moistureUpperLimit--;
      _delay_ms(200);
      }
    if(buttonPressed()==3)break;
  if(buttonPressed()==4){
    menuState=1;
    break;                                                                                                                                                                                                                                                                                                                                        
    }
    }
    _delay_ms(500);
  while(1){
    lcdPrint(0,0,"SMLLim:",moistureLowerLimit );
    lcdPrint(0,1," ");
    if(buttonPressed()==5){
      moistureLowerLimit++;
      _delay_ms(200);
      }
    else if(buttonPressed()==6){
      moistureLowerLimit--;
      _delay_ms(200);
      }
    if(buttonPressed()==3){
      lcdPrint(0,0,"Calibrate saved");_delay_ms(1000);
      Serial.println("saveCalibrate"),
      saveCalibrateSMS(moistureUpperLimitAddress,moistureUpperLimit,moistureLowerLimitAddress,moistureLowerLimit);
      menuState=1;
      break;
      }
  if(buttonPressed()==4){
    menuState=1;
    break;
    }
    }
  }
//=======================================================================================================//





//=======================FUNCTION FOR CALIBRATING VAPOR PRESSURE========================================//
void editVPD(){
  while(1){
    lcdPrint(0,0,"VPDULim:",upperLimitVPD );
    Serial.println(upperLimitVPD),
    lcdPrint(0,1," ");
    if(buttonPressed()==5){
      upperLimitVPD+=0.1;
      _delay_ms(200);
      }
    else if(buttonPressed()==6){
      upperLimitVPD-=0.1;
      _delay_ms(200);
      }
    if(buttonPressed()==3){_delay_ms(200);break;}
  if(buttonPressed()==4){
    menuState=1;
    break;
    }
    }
    _delay_ms(500);
  while(1){
    lcdPrint(0,0,"VPDLLim:",lowerLimitVPD );
    lcdPrint(0,1," ");
    if(buttonPressed()==5){
      lowerLimitVPD+=0.1;
      _delay_ms(200);
      }
    else if(buttonPressed()==6){
      lowerLimitVPD-=0.1;
      _delay_ms(200);
      }
    if(buttonPressed()==3){
      lcdPrint(0,0,"Calibrate saved");
      Serial.println("saveCalibrate");_delay_ms(1000);
      saveCalibrateVPD();
      menuState=1;
      break;}
  if(buttonPressed()==4){
    menuState=1;
    break;
    }
    }
  
  }
//=======================================================================================================//




  
void mainScreen(){
  int sms=getSoilMoisture();
  lcdPrint(0,0,"SM(%):",sms);
  lcdPrint(0,1,"VPD:", getVPD());
  Serial.println(getSoilMoisture());
  Serial.println(dht.readTemperature());
  if(buttonPressed()==3){
    Serial.println("WTF3");menuState=2;_delay_ms(200);
    }
  else if(buttonPressed()==4){
    Serial.println("WTF4");menuState=3;_delay_ms(200);
     
    }

  }
  

void loop() {
waterTankSystem(); 
irrigationSystem();  
vpdWarningSystem();
menuSystem();
}
