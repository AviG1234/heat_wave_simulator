/*#include <DallasTemperature.h>// termo sensor library
#include <OneWire.h>// termo sensor library
#define ONE_WIRE_BUS 4 //  both temperature sensors are connected to the same 5 pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Sensor1 ={0x28, 0xFF, 0x46, 0x6A, 0xC4, 0x17, 0x04, 0x98};// address for control sensor
DeviceAddress Sensor2 = {0x28, 0xAA, 0x51, 0x6E, 0x18, 0x13, 0x2, 0x3A}; // address for control sensor
DeviceAddress Sensor3 ={0x28, 0xFF, 0x01, 0x66, 0xC4, 0x17, 0x04, 0x32};// address for test sensor
DeviceAddress Sensor4 = {0x28, 0xEA, 0x34, 0x96, 0x1F, 0x13, 0x1, 0x79};// address for  test sensor
*/
#include <EEPROM.h>

unsigned long TenSecCycleCount;
unsigned long add_mills;
int TenMinCount =0;
int no_read_count = 0;
int T_EEPROM;

#define H_ControlPin 8
#define C_ControlPin 9
#define H_TestPin 2 // problem with H_Test rellay so i  swicht pin 7 with 2 and H_test power sors with C_test
#define C_TestPin 7
#define LEDpin 12
#define sensor_VCC 6



void setup(){

  Serial.begin(9600);
  pinMode(H_ControlPin,OUTPUT);
  pinMode(C_ControlPin,OUTPUT);
  pinMode(H_TestPin,OUTPUT);
  pinMode(C_TestPin,OUTPUT);
  pinMode(LEDpin, OUTPUT);
  pinMode(sensor_VCC, OUTPUT);
  digitalWrite(sensor_VCC, LOW);
  digitalWrite(H_ControlPin, HIGH);
  digitalWrite(C_ControlPin, HIGH);
  digitalWrite(H_TestPin, HIGH);
  digitalWrite(C_TestPin, HIGH);

  TenMinCount = EEPROM.read(0);
  T_EEPROM = EEPROM.read(1);
  EEPROM.write(0,0);
  EEPROM.write(1,1);

  TenSecCycleCount = 60*TenMinCount + T_EEPROM;
  add_mills = TenSecCycleCount;
  Serial.println("setup");
  Serial.print("add_mills: ");
  Serial.println(add_mills);
  sensors_setup();
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop(){
    float TempExpectedControl, NextTempExpectedControl, TempExpectedTest,NextTempExpectedTest;
    get_Ex_Temp(TenMinCount,TempExpectedControl,NextTempExpectedControl,TempExpectedTest,NextTempExpectedTest);
    for (int T = T_EEPROM; T<=60; T++){// 10 min cycle loop
        TenSecCycleCount++;
        Serial.print("i:  ");
        Serial.print(TenMinCount);
        Serial.print(",  TenSecCycleCount:  ");
        Serial.print(TenSecCycleCount);
        Serial.print(",  T: ");
        Serial.println(T);
        float T_TempExpControl = get_T_temp(T,TempExpectedControl,NextTempExpectedControl);
        float T_TempExpTest = get_T_temp(T,TempExpectedTest,NextTempExpectedTest);
        float TenpObservedControl, TenpObservedTest;
        //get_Ob_Temp(TenpObservedControl,TenpObservedTest);
      if (millis() > 20000){
      TenpObservedControl = 0;
      TenpObservedTest = 0;
      }
        float T_TempControlDelta = get_Temp_Delta(T_TempExpControl,TenpObservedControl);
        float T_TempTestDelta = get_Temp_Delta(T_TempExpTest,TenpObservedTest);
        unsigned long CycleRealTime = get_Ten_Sec_Cycle_Real_time(TenSecCycleCount);
        if (TenpObservedControl == 0 || TenpObservedTest == 0){
            no_read_count++;
            delay(CycleRealTime);
        }
        else{
            no_read_count = 0;
            run_rellays(CycleRealTime,T_TempControlDelta,T_TempTestDelta);
        }
        if (no_read_count == 3){
            digitalWrite(sensor_VCC, HIGH);
            delay(10000);
            T = T + 1;
            EEPROM.write(0,TenMinCount);
            EEPROM.write(1,T);
            delay(100);
            resetFunc();
        }

    }
    TenMinCount++;
    if (TenMinCount>143){
        resetFunc();
    }
}
/*
void sensors_setup(){
    delay(1000);
    sensors.begin();
    //sensors.isParasitePowerMode();
    sensors.setResolution(Sensor1,11);
    sensors.setResolution(Sensor2,11);
    sensors.setResolution(Sensor3,11);
    sensors.setResolution(Sensor4,11);
}
*/
void get_Ex_Temp(int Count10,float &TempExControl,float &NextTempExControl,float &TempExTest,float &NextTempExTest){ //get temperature values from the recorded data.
    float TempMeanData [150]={36.23,37.6,38.92,40.26,41.23,42.43,43.96,45.26,46.27,47.52,48.28,49.3,50.52,51.21,52.36,53.09,54.35,55.24,55.82,56.74,57.16,58.1,58.71,59.22,59.77,60.13,60.4,60.8,60.81,60.94,60.86,60.48,60.86,60.23,60.49,60.26,59.41,58.89,58.7,58.02,57.69,57.1,56.66,56.3,55.33,54.6,53.89,52.86,51.92,50.94,49.96,48.8,47.73,46.54,45.33,44.1,42.84,41.56,40.3,39.08,37.84,36.75,35.77,34.93,34.18,33.5,32.89,32.36,31.89,31.43,31.01,30.62,30.27,29.94,29.64,29.36,29.09,28.82,28.55,28.31,28.09,27.89,27.68,27.49,27.31,27.1,26.89,26.69,26.51,26.32,26.15,25.97,25.8,25.63,25.49,25.33,25.2,25.09,24.99,24.87,24.75,24.65,24.59,24.51,24.43,24.37,24.29,24.2,24.12,24.06,24.01,23.95,23.88,23.85,23.84,23.77,23.74,23.76,23.72,23.63,23.53,23.45,23.35,23.3,23.26,23.26,23.26,23.24,23.23,23.29,23.42,23.66,23.98,24.44,24.93,25.74,26.63,27.56,28.7,29.74,30.96,32.11,33.45,34.89,
    }; // mean temperature reads Katzrin soil at 0.5 cm 24.7.2018 to 5.9.2018. 10 mints apart.
    float TempSdData [150]={1.93,1.74,1.77,2.14,2.48,2.63,2.57,2.57,2.87,2.89,3.31,3.01,2.82,3.41,2.76,3.32,3.37,3.12,3.57,3.45,3.76,3.5,3.34,3.35,3.61,3.74,3.61,3.52,3.55,3.6,3.89,4.47,3.69,4.22,3.74,3.7,4.77,4.98,5.06,4.6,4.15,3.92,3.74,2.62,3.12,2.32,2.27,2.42,2.18,2.11,2.09,2.2,2.06,2.06,2.07,2.08,2.06,2.06,2.05,1.96,1.77,1.66,1.58,1.54,1.48,1.43,1.36,1.3,1.25,1.23,1.2,1.18,1.14,1.11,1.09,1.09,1.08,1.05,1.03,1.01,1.01,1,1,0.99,1,1,1,1.03,1.04,1.05,1.08,1.09,1.11,1.11,1.17,1.2,1.23,1.25,1.24,1.26,1.27,1.31,1.35,1.41,1.47,1.51,1.52,1.55,1.59,1.56,1.51,1.52,1.53,1.56,1.57,1.6,1.65,1.72,1.79,1.83,1.8,1.78,1.74,1.75,1.8,1.86,1.89,1.9,1.86,1.84,1.78,1.7,1.64,1.59,1.5,1.28,1.13,1.16,1.17,1.29,1.36,1.46,1.66,1.77,
    };// SD temperature reads Katzrin soil at 0.5 cm 24.7.2018 to 5.9.2018. 10 mints apart.

    TempExControl = TempMeanData[Count10];
    TempExTest = TempExControl + TempSdData[Count10]*2;
    if(Count10<143)
    {
        NextTempExControl = TempMeanData[Count10+1];
        NextTempExTest = NextTempExControl + TempSdData[Count10+1]*2;
    }
    else
    {
        NextTempExControl = TempMeanData[0];
        NextTempExTest = NextTempExControl + TempSdData[0]*2;
    }
}

void get_Ob_Temp(float &TenpObservedControl, float &TenpObservedTest){
//read the temperature of test environment and control environment

    float SensorData1, SensorData2, SensorData3, SensorData4;
    bool boolSensorData1 = false , boolSensorData2 = false, boolSensorData3 = false, boolSensorData4 = false;
    bool boolSensor_1_2 = false, boolSensor_3_4 = false;

    int while_loop_count = 0;

    while((boolSensor_1_2 == false || boolSensor_3_4 == false) && while_loop_count < 6){//
        if (while_loop_count > 0)//sensor delay of 0.75 s
        {
            digitalWrite(LEDpin,HIGH);
            delay(750);
            digitalWrite(LEDpin,LOW);
        }
        sensors.requestTemperatures();// Send the command to get temperatures

        if (boolSensorData1 == false){
          SensorData1 = 30;//sensors.getTempC(Sensor1);
            if (SensorData1 > 1 && SensorData1 != 85){
                boolSensorData1 = true;
            }
        }
        if (boolSensorData2 == false){
          SensorData2 = 30;//sensors.getTempC(Sensor2);
            if (SensorData2 > 1 && SensorData2 != 85){
                boolSensorData2 = true;            }
        }
        if (boolSensorData3 == false){
          SensorData3 = 30;//sensors.getTempC(Sensor3);
            if (SensorData3 > 1 && SensorData3 != 85){
                boolSensorData3 = true;
            }
        }
        if (boolSensorData4 == false){
          SensorData4 = 30;//sensors.getTempC(Sensor4);
            if (SensorData4 > 1 && SensorData4 != 85){
                boolSensorData4 = true;
            }
        }

        if (boolSensorData1 == true || boolSensorData2 == true){// temp value for control environment
            boolSensor_1_2 = true;
        }
        if (boolSensorData3 == true || boolSensorData4 == true){// temp value for test environment
            boolSensor_3_4 = true;
        }
        while_loop_count++;
    }

    if (boolSensor_1_2){
       TenpObservedControl = (SensorData1*boolSensorData1 + SensorData2*boolSensorData2)/(boolSensorData1 + boolSensorData2);
    }
    else {
        TenpObservedControl = 0;
    }
    if (boolSensor_3_4){
        TenpObservedTest = (SensorData3*boolSensorData3 + SensorData4*boolSensorData4)/(boolSensorData3 + boolSensorData4);
    }
    else {
        TenpObservedTest = 0;
    }
}

float get_T_temp(float T,float Temp, float NextTemp){
    return (Temp + (T/60)*(NextTemp-Temp));
}

float get_Temp_Delta(float Exp, float Obs){//get difference in temperature between target temperature and real temperature (expected and observed)
    if (Obs == 0){
        return Obs;
    }
    else {
        return Exp - Obs;
    }
}

long get_Ten_Sec_Cycle_Real_time(unsigned long TenSecCycleCount){
    unsigned long CalculatedTime = TenSecCycleCount*10000 ;//each cycle takes 10,000 milliseconds
    long Time = CalculatedTime - (millis());//millis() will overflow after 50 days.
    if (Time < 0){
        Time = 0;
        return Time;
    }
    else{
        return Time;
    }
}

void run_rellays(unsigned long CycleRealTime,float T_TempControlDelta,float T_TempTestDelta){//activate ether heating or cooling of control and test environment
    bool H_control_bool = true, C_control_bool = true, H_test_bool = true, C_test_bool = true;

    float Temp_Minimal_Delta = 0.25;
    float Temp_gradient = 1;

    float work_time_control;
    if (abs(T_TempControlDelta)<=Temp_gradient){
        work_time_control = abs(T_TempControlDelta)*CycleRealTime;//work time is less then cycle time.
    }
    else{
        work_time_control = CycleRealTime;//work time is equal to cycle time.
    }

    float work_time_test;
    if (abs(T_TempTestDelta)<=Temp_gradient){
        work_time_test = abs(T_TempTestDelta)*CycleRealTime;//work time is less then cycle time.
    }
    else{
        work_time_test = CycleRealTime;//work time is equal to cycle time.
    }

    if (abs(T_TempControlDelta)<= Temp_Minimal_Delta){
        work_time_control = 0;
    }
    else if(T_TempControlDelta > 0){
        H_control_bool = false;
    }
    else if(T_TempControlDelta < 0){
        C_control_bool = false;
    }

    if (abs(T_TempTestDelta)<= Temp_Minimal_Delta){
        work_time_test = 0;
    }
    else if(T_TempTestDelta > 0){
        H_test_bool = false;
    }
    else if(T_TempTestDelta < 0){
        C_test_bool = false;
    }

    int rellay_Pin_Array[] = {H_ControlPin,C_ControlPin,H_TestPin,C_TestPin};
    bool bool_control_Array[] = {H_control_bool,C_control_bool,H_test_bool,C_test_bool};

    /*
    Serial.print("work_time_control: ");
    Serial.println(work_time_control);
    Serial.print("work_time_test: ");
    Serial.println(work_time_test);
    Serial.println("{H_control_bool,C_control_bool,H_test_bool,C_test_bool}: ");
    for (int i = 0; i<=3; i++){
    Serial.println(bool_control_Array[i]);
    }
    Serial.println(" ");
    */

    if (work_time_control == work_time_test){//
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(work_time_control);
        for (int i = 0; i<=3; i++){//tern control and test bool relays off
            bool_control_Array[i] = true;
        }
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(CycleRealTime - work_time_control);
    }

    else if (work_time_control > work_time_test){
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(work_time_test);
        for (int i = 2; i<=3; i++){//tern test bool relays off
            bool_control_Array[i] = true;
        }
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(work_time_control - work_time_test);

        for (int i = 0; i<=3; i++){//tern control and test bool relays off
            bool_control_Array[i] = true;
        }
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(CycleRealTime - work_time_control);
    }

    else{//(work_time_control < work_time_test)
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(work_time_control);
        for (int i = 0; i<=1; i++){//tern control bool relays off
            bool_control_Array[i] = true;
        }
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(work_time_test - work_time_control);

        for (int i = 0; i<=3; i++){//tern control and test bool relays off
            bool_control_Array[i] = true;
        }
        for (int i = 0; i<=3; i++){
            digitalWrite(rellay_Pin_Array[i],bool_control_Array[i]);
        }
        delay(CycleRealTime - work_time_test);
    }
}
