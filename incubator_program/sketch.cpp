#include <DallasTemperature.h>// temperature sensors library.
#include <OneWire.h>// library to communication with temperature sensors.
#include <EEPROM.h>

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

#define TenMin 60 //ten second cycles in ten minutes.
#define Day 143 //ten minute cycles in a day.
#define EEPROM_TenMinCount 0;
#define EEPROM_TenSecCycleCount 1;

struct temp{
    float Mean;
    float sd;
};

temp tempArray[150] = {{36.23,1.93},	{37.6,1.74},	{38.92,1.77},	{40.26,2.14},	{41.23,2.48},	{42.43,2.63},	{43.96,2.57},	{45.26,2.57},	{46.27,2.87},	{47.52,2.89},	{48.28,3.31},	{49.3,3.01},	{50.52,2.82},	{51.21,3.41},	{52.36,2.76},	{53.09,3.32},	{54.35,3.37},	{55.24,3.12},	{55.82,3.57},	{56.74,3.45},	{57.16,3.76},	{58.1,3.5},	{58.71,3.34},	{59.22,3.35},	{59.77,3.61},	{60.13,3.74},	{60.4,3.61},	{60.8,3.52},	{60.81,3.55},	{60.94,3.6},	{60.86,3.89},	{60.48,4.47},	{60.86,3.69},	{60.23,4.22},	{60.49,3.74},	{60.26,3.7},	{59.41,4.77},	{58.89,4.98},	{58.7,5.06},	{58.02,4.6},	{57.69,4.15},	{57.1,3.92},	{56.66,3.74},	{56.3,2.62},	{55.33,3.12},	{54.6,2.32},	{53.89,2.27},	{52.86,2.42},	{51.92,2.18},	{50.94,2.11},	{49.96,2.09},	{48.8,2.2},	{47.73,2.06},	{46.54,2.06},	{45.33,2.07},	{44.1,2.08},	{42.84,2.06},	{41.56,2.06},	{40.3,2.05},	{39.08,1.96},	{37.84,1.77},	{36.75,1.66},	{35.77,1.58},	{34.93,1.54},	{34.18,1.48},	{33.5,1.43},	{32.89,1.36},	{32.36,1.3},	{31.89,1.25},	{31.43,1.23},	{31.01,1.2},	{30.62,1.18},	{30.27,1.14},	{29.94,1.11},	{29.64,1.09},	{29.36,1.09},	{29.09,1.08},	{28.82,1.05},	{28.55,1.03},	{28.31,1.01},	{28.09,1.01},	{27.89,1},	{27.68,1},	{27.49,0.99},	{27.31,1},	{27.1,1},	{26.89,1},	{26.69,1.03},	{26.51,1.04},	{26.32,1.05},	{26.15,1.08},	{25.97,1.09},	{25.8,1.11},	{25.63,1.11},	{25.49,1.17},	{25.33,1.2},	{25.2,1.23},	{25.09,1.25},	{24.99,1.24},	{24.87,1.26},	{24.75,1.27},	{24.65,1.31},	{24.59,1.35},	{24.51,1.41},	{24.43,1.47},	{24.37,1.51},	{24.29,1.52},	{24.2,1.55},	{24.12,1.59},	{24.06,1.56},	{24.01,1.51},	{23.95,1.52},	{23.88,1.53},	{23.85,1.56},	{23.84,1.57},	{23.77,1.6},	{23.74,1.65},	{23.76,1.72},	{23.72,1.79},	{23.63,1.83},	{23.53,1.8},	{23.45,1.78},	{23.35,1.74},	{23.3,1.75},	{23.26,1.8},	{23.26,1.86},	{23.26,1.89},	{23.24,1.9},	{23.23,1.86},	{23.29,1.84},	{23.42,1.78},	{23.66,1.7},	{23.98,1.64},	{24.44,1.59},	{24.93,1.5},	{25.74,1.28},	{26.63,1.13},	{27.56,1.16},	{28.7,1.17},	{29.74,1.29},	{30.96,1.36},	{32.11,1.46},	{33.45,1.66},	{34.89,1.77}
};

int TenSecCycleCount;// count of 10 second long cycle.
int TenMinCount;// count of 10 minute long cycle.


#include <Arduino.h>
void setup()
{
    Serial.begin(9600);

    pinMode(H_ControlPin,OUTPUT);
    pinMode(C_ControlPin,OUTPUT);
    pinMode(H_TestPin,OUTPUT);
    pinMode(C_TestPin,OUTPUT);

    /**
    updating cycle counts from EEPROM in case of power failure.
    to begin at TenMinCount at 0 and TenSecCycleCount at 1
    (readings at the beginning of day cycle at 09:00:00)
    the system need to be turned on then off and back on in under 30 seconds.
    */
    int additionToCount = 3;
    TenMinCount = EEPROM.read(EEPROM_TenMinCount);
    TenSecCycleCount = EEPROM.read(EEPROM_TenSecCycleCount) + additionToCount;
    if(TenSecCycleCount > Min){
        TenMinCount++;
        TenSecCycleCount -= Min;
        if(TenMinCount > Day)
            TenMinCount = 0;
    }
    EEPROM.write(EEPROM_TenMinCount,0);
    EEPROM.write(EEPROM_TenSecCycleCount,1);

    //white for 30 seconds for
    delay(3*10000);

    //temperature sensors setup
    sensors.begin();
    sensors.setResolution(SensorControl,11);
    sensors.setResolution(SensorTest,11);
}

void loop()
{
    for(; TenMinCount < Day; TenMinCount++){//day cycle.
        update_TenMinEEPROM(TenMinCount);
        for(; TenSecCycleCount < min){//ten minute cycle.
            update_TenSecEEPROM(TenSecCycleCount);
            float controlTemp, testTemp;
            getTemperature();

        }
    }
}

//functions to update EEPROM
void update_TenMinEEPROM(int TenMinCount){EEPROM.write(EEPROM_TenMinCount,TenMinCount);}
void update_TenSecEEPROM(int TenSecCycleCount){EEPROM.write(EEPROM_TenSecCycleCount,TenSecCycleCount);}
