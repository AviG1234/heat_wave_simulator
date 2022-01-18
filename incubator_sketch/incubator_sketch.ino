#include <DallasTemperature.h>// temperature sensors library.
#include <OneWire.h>// library to communication with temperature sensors.
#include <EEPROM.h>
#include "temptarget.h"

#define ONE_WIRE_BUS 5//temperature sensors are connected to the 5 pin.
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress SensorControl ={0x28, 0xFF, 0x46, 0x6A, 0xC4, 0x17, 0x04, 0x98};// address for control sensor
DeviceAddress SensorTest = {0x28, 0xAA, 0x51, 0x6E, 0x18, 0x13, 0x2, 0x3A};// address for test sensor

//relay control pins. H(heat), C(cold).
#define H_ControlPin 8
#define C_ControlPin 9
#define H_TestPin 6
#define C_TestPin 7
#define LED_BUILTIN 13

#define TEN_MIN 60 //ten second cycles in ten minutes.
#define DAY 143 //ten minute cycles in a day.
#define SECOND 1000 //millisecond in 1 second.

#define EEPROM_TenMinCount 0
#define EEPROM_TenSecCycleCount 1
#define EEPROM_dayCount 2

#define TEMPERATURE_BUFFER 0.5

//setup function delay. to reset the system it must be turned on of and back on
//in under SETUP_DELAY time (the value is in milliseconds).
#define SETUP_DELAY 30000

int TenSecCycleCount;// count of 10 second long cycle.
int TenMinCount;// count of 10 minute long cycle.
int dayCount;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

#include <Arduino.h>
void setup()
{
    Serial.begin(9600);

    pinMode(H_ControlPin,OUTPUT);
    pinMode(C_ControlPin,OUTPUT);
    pinMode(H_TestPin,OUTPUT);
    pinMode(C_TestPin,OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    /**
    updating cycle counts from EEPROM in case of power failure.
    to begin at TenMinCount at 0 and TenSecCycleCount at 1
    (readings at the beginning of day cycle at 09:00:00)
    the system need to be turned on then off and back on in under 30 seconds.
    */
    TenMinCount = EEPROM.read(EEPROM_TenMinCount);
    TenSecCycleCount = EEPROM.read(EEPROM_TenSecCycleCount) + SETUP_DELAY/(10*SECOND);
    dayCount = EEPROM.read(EEPROM_dayCount);
    if(TenSecCycleCount > TEN_MIN){
        TenMinCount++;
        TenSecCycleCount = 0;
        if(TenMinCount > DAY)
            TenMinCount = 0;
    }
    
    //reset EEPROM memory in case of restarting.
    update_TenMinEEPROM(0);
    update_TenSecEEPROM(1);
    update_deyCountEEPROM(0);

    //white for 10 seconds for
    digitalWrite(LED_BUILTIN, HIGH); 
    delay(SETUP_DELAY);
    digitalWrite(LED_BUILTIN, LOW); 

    //temperature sensors setup
    sensors.begin();
    sensors.setResolution(SensorControl,11);
    sensors.setResolution(SensorTest,11);
}

void loop()
{
    for(; TenMinCount < DAY; TenMinCount++){//day cycle.
        update_TenMinEEPROM(TenMinCount);
        for(; TenSecCycleCount < TEN_MIN; TenSecCycleCount++){//ten minute cycle.
            update_TenSecEEPROM(TenSecCycleCount);
            //get the difference between measured temperature and target temperature.
            float tempDelta[1];
            getTemperatureDifference(tempDelta);
            setRelays(tempDelta);
            delay(getCycleTime());//get the remainder time of the ten second cycle.
        }
    }
    update_TenMinEEPROM(0);
    update_TenSecEEPROM(1);
    update_deyCountEEPROM(++dayCount);
    resetFunc();
}



void getTemperatureDifference(float tempReads[]){
    //function read temperature from control and test chambers and
    //compares the reads to the target temperature.
    TempTarget tempTarget;
    if(getTemperature(tempReads)){
        //delta is calculated as measured temperature minus target temperature.
        tempReads[0] -= tempTarget.controlTemp(TenMinCount, TenSecCycleCount);
        tempReads[1] -= tempTarget.testTemp(TenMinCount, TenSecCycleCount);
    }
    else{
        tempReads[0] = 0;
        tempReads[1] = 0;
    }
}

bool getTemperature(float tempReads[]){
    //try read from sensor, if read unsuccessful delay for 1 second and repeat for 6 seconds.
    //if after 6 seconds reading is unsuccessful return null values.
    int errorCode1 = -127, errorCode2 = 85;
    for(int delayRound = 0; delayRound < 6; delayRound++){
        sensors.requestTemperatures();// Send command to get temperatures.
        tempReads[0] = sensors.getTempC(SensorControl); //control chamber temperature
        tempReads[1] = sensors.getTempC(SensorTest);//test chamber temperature
        if((tempReads[0] != errorCode1 || tempReads[0] != errorCode2) ||
        (tempReads[1] != errorCode1 || tempReads[1] != errorCode2)){
            return true;
        }
        delay(SECOND);
    }
    return false;
}

long getCycleTime(){
    long calTime = ((TenMinCount)*TEN_MIN + TenSecCycleCount);//number of ten second cycles sense program beginning.
    calTime *= 10;//number seconds sense program beginning.
    calTime *=1000;//number seconds milliseconds  program beginning.
    calTime += SETUP_DELAY;
    long delayTime = calTime - millis();

    if(delayTime < 0 ) delayTime = 0;
    return delayTime;
}

void setRelays(float tempDelta[]){
    if(tempDelta[0] < -TEMPERATURE_BUFFER)controlHeatRelayON();//heat control chamber.
    else controlHeatOFF();
    if(tempDelta[0] > TEMPERATURE_BUFFER) controlColdON();//cool control chamber.
    else controlColdOFF();

    if(tempDelta[1] < -TEMPERATURE_BUFFER) testHeatON();//heat control chamber.
    else testHeatOFF();
    if(tempDelta[1] > TEMPERATURE_BUFFER) testColdON();//cool control chamber.
    else testColdOFF();
}

//function controlling relays for cooling and heating of the chambers.
void controlHeatRelayON(){digitalWrite(H_ControlPin,LOW);}
void controlHeatOFF(){digitalWrite(H_ControlPin,HIGH);}
void controlColdON(){digitalWrite(C_ControlPin,LOW);}
void controlColdOFF(){digitalWrite(C_ControlPin,HIGH);}
void testHeatON(){digitalWrite(H_TestPin,LOW);}
void testHeatOFF(){digitalWrite(H_TestPin,HIGH);}
void testColdON(){digitalWrite(C_TestPin,LOW);}
void testColdOFF(){digitalWrite(C_TestPin,HIGH);}

//functions to update EEPROM
void update_TenMinEEPROM(int TenMinCount){EEPROM.write(EEPROM_TenMinCount,TenMinCount);}
void update_TenSecEEPROM(int TenSecCycleCount){EEPROM.write(EEPROM_TenSecCycleCount,TenSecCycleCount);}
void update_deyCountEEPROM(int TenSecCycleCount){EEPROM.write(EEPROM_dayCount,dayCount);}
