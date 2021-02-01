#include <math.h>
#include <Wire.h>                                                                 // I2C library
#include <SPI.h>                                                                  // SPI library  
//#include <PID_v1.h>
#include <ctype.h>
#include <string.h>
#include <Adafruit_MCP4725.h>                                                     // DAC IC library


Adafruit_MCP4725 dac_hv;                                                          // Define hv DAC
Adafruit_MCP4725 dac_pres;                                                        // Define pressure DAC


int pressure_set_pin = DAC0;
int voltage_set_pin = DAC1;

int pressure_read_pin = A0;
int voltage_read_pin = A1;
int current_read_pin = A2;
int ambientTemp_read_pin = A3;
int hum_read_pin = A4;


/// SET INPUT PARAMETERRS ///

//Data_in already decoded and ready to use. Arduino Due supports 2 DAC. Max resolution 12bit. Range: 0.55-2.7V.
//Amplification of gain = 3 is used.

void set_pressure(int input_value) {                                              //Set voltage. Maximum pressure value 4Bar.
  int new_pressure = input_value - 100000;                                        //Decode to get the pressure value in kPa
  if (new_pressure > 400) {                                                       //Make sure the maximum pressure does not exceed 4bar
    new_pressure = 400;
  }
  else if (new_pressure < 0) {                                                    //Minimum pressure is 0.
    new_pressure = 0;
  }
  else {
    int new_pressure_to_dac = round(4095* new_pressure * 0.8 / 400);
    dac_pres.setVoltage(new_pressure_to_dac, false);                              //Set analog output pressure.
    //Serial.println(new_pressure_to_dac);
    //Serial.println("ok");
    //analogWrite(pressure_set_pin,4096*(new_pressure/400));                      //Convert the input_value to the pressure scale.
  }
}

void set_voltage(int input_value) {
  int new_voltage = input_value - 200000;                                         //Decode to get the pressuse value in volts.
  if (new_voltage > 25000) {                                                      //Set voltage. Maximum voltage value 25kV.
    new_voltage = 25000;
  }
  else if (new_voltage < 0) {
    new_voltage = 0;
  }
  else {
    int new_voltage_to_dac = 4095 * new_voltage / 19000;                          //Set analog output voltage. Number given in VOLTS.
    dac_hv.setVoltage(new_voltage_to_dac, false);                                 //Convert the input_value to the voltage scale.
    //Serial.println(new_voltage_to_dac);
    //Serial.println("ok");
    //analogWrite(voltage_set_pin,4096*(input_value/25000));
  }
}

/// READ INPUT PARAMETERRS ///
int read_pressure(int data_in = analogRead(A0)) {                                 //Read current pressure. Data_in range is converted from 0-3.3V to 0-4096.
  int result = round(data_in * 330 * 1.58 / 4095);                                        //Convert from analog voltage to kPa value. Use scalar of 1.58 as a correcting factor.
  
  return result;
}

int read_voltage(int data_in = analogRead(A1)) {                                  //Read current voltage. Data_in range is converted from 0-2.5V to 0-4096.
  int result = data_in * 33330 / 3795;                                            //Convert from analog voltage to the actual voltage. Use a value of 3795 (instead of 4095) as a correcting factor.
  return result;
}

int read_current(int data_in = analogRead(A2)) {                                  //Read current voltage. Data_in range is converted from 0-2.5V to 0-4096.
  int result = data_in * 250 / 3102;                                              //Convert from analog voltage to mA.
  return result;
}
void read_ambientTemp() {
}
void read_hum() {
}

void setup() {
  //SERIAL INIT
  Serial.begin(115200);                                                           //Communicate with MATLAB via TTL to USB (COM14)
  //Serial1.begin(115200);
  Serial1.begin(115200);                                                          //Communicate with Python (COM6)
  //DUE ADC RESOLUTION
  Serial2.begin(115200);
  analogReadResolution(12);

  //DUE/DAC RESOLUTION
  analogWriteResolution(12);

  // ADAFRUID IC DAC HV setup
  dac_hv.begin(0x62);                                                             // deafauly ADAFRUIT IC DAC address is 0x62. IF A0 is connected to VCC
  // the address is 0x63
  // ADAFRUIT IC DAC PRESSURE setup
  dac_pres.begin(0x63);

  //PRESSURE OUTPUT INIT
  pinMode(DAC0, OUTPUT);

  //VOLTAGE OUTPUT INIT
  pinMode(DAC1, OUTPUT);

  //PRESSURE INPUT INIT
  pinMode(pressure_read_pin, INPUT);

  //VOLTAGE INPUT INIT
  pinMode(voltage_read_pin, INPUT);

  //CURRENT INPUT INIT
  pinMode(current_read_pin, INPUT);

  //AMBIENT TEMP INPUT INIT
  pinMode(ambientTemp_read_pin, INPUT);

  //HUMIDITY INPUT INIT
  pinMode(hum_read_pin, INPUT);

  dac_hv.setVoltage(1, false);   //HV
  dac_pres.setVoltage(1, false);   //AP
}

void loop() {
  //dac_hv.setVoltage(4095,false);
  ///// PYTHON USER INPUT /////
  // This will only be execured if there is incoming request from the user
  //delay(50);
  //Serial1.println(round(analogRead(A0)*1.58*330/4095));
  //Serial1.print(" ");

  if (Serial.available() > 0) {                                                  //Chekc if data is received from Python(user). This allows user
    //Serial1.print("test1");                                                         //to change the parameters.
    int python_data_in = Serial.parseInt();                                      //Read integer
    if (python_data_in > 100000 && python_data_in < 200000) {                     //If true, data_in is new pressure to set. Number given as 100;000 + desired pressure(in kPa).
      //Serial1.println(python_data_in);
      //Serial.println(python_data_in);
      set_pressure(python_data_in);
    }
    else if (python_data_in > 200000 && python_data_in < 300000) {                //If true, data_in is new voltage to set. Number given as 200'000 + desired voltage(in Volts).
      //Serial1.println(python_data_in);
      //Serial.println(python_data_in);
      set_voltage(python_data_in);
    }
    /*
      else if (python_data_in > 300000 && python_data_in < 400000){                 //If true, set this temperature on SMOOTHIEBOARD
      }
      else if (python_data_in > 400000 && python_data_in < 500000){                 //If true, set this collection speed on SMOOTHIEBOARD
      }
      else if (python_data_in > 500000 && python_data_in < 600000){                 //If true, set this collection speed on SMOOTHIEBOARD
      }
    */
  }

  ///// MATLAB program input /////
  // This will only be execured if MATLAB request data or parameter change

  if (Serial1.available() > 0) {
    //Check data requests first
    //String MATLAB_data_in_string = Serial.readStringUntil('\n');                //Check if data is received from MATLAB. This allows MATLAB to
    int MATLAB_data_in_int = Serial1.parseInt();                                  //automaticall change the input parameters
    Serial.println(MATLAB_data_in_int);
    if (MATLAB_data_in_int == 100) {                                              //If pressure is requested,send current pressure value.
      int current_pressure = read_pressure();
      Serial1.println(current_pressure);
      Serial1.println("ok");      
    }
    else if (MATLAB_data_in_int == 200) {                                         //If voltage is requested, send current voltage value.
      int current_voltage = read_voltage();
      // Serial1.println(current_voltage);
      Serial1.println(current_voltage);
      Serial1.println("ok");
    }
    else if (MATLAB_data_in_int == 300) {                                         //If current is requested, send current current value.
      int current_current = read_current();
      Serial1.println(current_current);
      Serial1.println("ok");
    }
    else if (MATLAB_data_in_int == 400) {                                         //If ambient temperature is requested, send current
      int current_amb_temp = 25;                                                  //ambient temperature value.
      Serial1.println(current_amb_temp);
      Serial1.println("ok");
    }
    else if (MATLAB_data_in_int == 500) {                                         //If humidity is requested, send current humidity value.
      int current_hum = 60;
      Serial1.println(current_hum);
      Serial1.println("ok");
    }
    //If no data is requested, check parameters need to be set
    else {
      //Serial1.println("test bad");
      if (MATLAB_data_in_int > 100000 && MATLAB_data_in_int < 200000) {           //Check if the encoded value is pressure.
        set_pressure(MATLAB_data_in_int);                                         //Set the new pressure value.
        //Serial1.println(MATLAB_data_in_int);
      }
      else if (MATLAB_data_in_int > 200000 && MATLAB_data_in_int < 300000) {
        set_voltage(MATLAB_data_in_int);
        //Serial1.println(MATLAB_data_in_int);
      }
    }
  }
}
