


#include "FreeRTOS_SAMD21.h"

#include <Adafruit_MCP4728.h>
/*
LoadFile ../.build/FireHawk_II_devel.ino.elf
*/

/*
  Proportional Valve Test
  test of Parker 
  
*/
#define PICO_NO_HARDWARE 1

#include <Wire.h>
#include <arduino.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>
#include <rp2040_pio.h>
#include <TCA9548A-SOLDERED.h>
#include "RTClib.h"
#include "Driver_ppsystemsCO2.h"
#include "Driver_selectorValves.h"
#include "Driver_StatusLED.h"
#include "Driver_ProportionalValve.h"
#include "Driver_mprls.h"
#include "Control.h"
#include "Command.h"
#include "DataLogger.h"
#include "globals.h"



//**************************************************************************
// Type Defines and Constants
//**************************************************************************

#define  ERROR_LED_PIN  13 //Led Pin: Typical Arduino Board
//#define  ERROR_LED_PIN  2 //Led Pin: samd21 xplained board

#define ERROR_LED_LIGHTUP_STATE  HIGH // the state that makes the led light up on your board, either low or high


#define CURRENT_SENSOR_I2C 0x40
#define FLOW_SENSOR_I2C 0x07
#define RELAY_ON_PIN 5
#define RELAY_OFF_PIN 6
#define VALVE_CONTROL_PIN 10 
#define STATUS_LED_PIN 11

#define STEP_TEST 1



//**************************************************************************
// Can use these function for RTOS delays
// Takes into account processor speed
// Use these instead of delay(...) in rtos tasks
//**************************************************************************
void myDelayUs(int us)
{
  vTaskDelay( us / portTICK_PERIOD_US );  
}

void myDelayMs(int ms)
{
  vTaskDelay( (ms * 1000) / portTICK_PERIOD_US );  
}

void myDelayMsUntil(TickType_t *previousWakeTime, int ms)
{
  vTaskDelayUntil( previousWakeTime, (ms * 1000) / portTICK_PERIOD_US );  
}

#define SORBENT_VALVE_CHANNEL MCP4728_CHANNEL_A
#define AEROSOL_VALVE_CHANNEL MCP4728_CHANNEL_B
#define CARBON_VALVE_CHANNEL MCP4728_CHANNEL_C
#define SORBENT_FLOW_MUX_CHANNEL 0
#define AEROSOL_FLOW_MUX_CHANNEL 1
#define CARBON_FLOW_MUX_CHANNEL 2
#define FLOW_SENSOR_I2C 0x07
#define PSENSOR1_MUX_CHANNEL 0x04
#define PSENSOR2_MUX_CHANNEL 0x05

// Global variables
struct Readings readings;
struct Settings settings;

RTC_PCF8523 rtc;
Adafruit_INA219 ina219;
Adafruit_MCP4728 mcp;
int flowAOPin = A0;                               
DateTime now;
uint32_t msClock = 0;
Driver_ppsystemsCO2 co2;
Driver_selectorValves selectorValves;
Driver_StatusLED led;
DataLogger dataLogger;
Command command;
TCA9548A i2cMux;
PressureSensor p1(&i2cMux, PSENSOR1_MUX_CHANNEL, &readings.pressure1);
PressureSensor p2(&i2cMux, PSENSOR2_MUX_CHANNEL, &readings.pressure2);

Driver_ProportionalValve sorbentValve(
                  &mcp, 
                  &i2cMux, 
                  SORBENT_FLOW_MUX_CHANNEL,
                  FLOW_SENSOR_I2C,
                  SORBENT_VALVE_CHANNEL,
                  &settings.flowSorbentSetPoint,
                  &readings.flowSorbent);
Driver_ProportionalValve aerosolValve(
                  &mcp, 
                  &i2cMux, 
                  AEROSOL_FLOW_MUX_CHANNEL,
                  FLOW_SENSOR_I2C,
                  AEROSOL_VALVE_CHANNEL,
                  &settings.flowAerosolSetPoint,
                  &readings.flowAerosol);
Driver_ProportionalValve carbonValve(
                  &mcp, 
                  &i2cMux, 
                  CARBON_FLOW_MUX_CHANNEL,
                  FLOW_SENSOR_I2C,
                  CARBON_VALVE_CHANNEL,
                  &settings.flowCarbonSetPoint,
                  &readings.flowCarbon);


#define INTERVAL_MS_CLOCK 1
#define INTERVAL_DRIVER_TICK 10
#define INTERVAL_DRIVER_TEST 1000

static void task_ms_clock(void *pvParameters)
{
  Serial.println("Millisecond clock started");

  while(1)
  {
    msClock++;
    myDelayMs(INTERVAL_MS_CLOCK);
  }
}

bool initialized = false;

static void task_driver_tick(void *pvParameters)
{
  Serial.println("driver heartbeat started");
  while(1)
  {
    if (initialized)
    {
      now = rtc.now();
      co2.tick();
      selectorValves.tick();
      dataLogger.tick();
      sorbentValve.tick();
      aerosolValve.tick();    
      carbonValve.tick();
      led.tick();
      p1.tick();
      p2.tick();
    }
    myDelayMs(INTERVAL_DRIVER_TICK);
  }
}

String co2outstr = "";
String co2instr = "";

bool direct = false; 
bool setFlow = false;

int valveCount = 0;
bool valveState = false;

void initRTC(bool doset = false)
{
  if (! rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (! rtc.initialized() || rtc.lostPower() || doset) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

char flowToSet = 's';
String checkFlowSet = "sac";
bool enablePID = true;

static void task_test_drivers(void *pvParameters)
{
  char inchar;
  settings.flowSorbentSetPoint = 200;
  settings.flowAerosolSetPoint = 0;
  settings.flowCarbonSetPoint = 0;
  //sorbentValve.setManual(2000);
  sorbentValve.enablePID(enablePID);
  aerosolValve.enablePID(enablePID);
  carbonValve.enablePID(enablePID);
  //sorbentValve.setPWMpin(5);
  //aerosolValve.setPWMpin(6);
  //carbonValve.setPWMpin(9);
  initialized = true;
  selectorValves.openSet(0);

  while (1)
  {
    if (Serial.available())
    {
      char cline[128];
      size_t lineLength = Serial.readBytesUntil('\n', cline, 127);
      cline[lineLength-1] = '\0';
      String cmdLine = cline;
      command.checkAndParseCommandLine(cmdLine);
    }
    /**
      inchar = Serial.read(); 
      Serial.print(inchar);
      if (inchar == '|')
      {
        direct = !direct;
        if (direct)
          Serial.println("DIRECT");
        else
          Serial.println("PROCESSING");
        inchar = 0;
      }
      if (inchar == '^')
      {
        setFlow = !setFlow;
        if (setFlow)
          Serial.println("SETFLOW");
        else
          Serial.println("REGULAR");
        inchar = 0;
      }

      if (inchar == 'Z')
      {
        co2.startZero();
        inchar = 0;
      } 
      if (inchar == 'T')
      {
        initRTC(true);
        inchar = 0;
      } 
      if (setFlow)
      {
        if (checkFlowSet.indexOf(String(inchar)) > -1)
        {
          flowToSet = inchar;
          Serial.printf("flow setting %c\n",flowToSet);
          inchar = 0;
          co2outstr = "";
          continue;
        }
      }
      if ((inchar == '\r') || (inchar == '\n')) 
      {
        String sets = "123";
        if ((co2outstr.length() == 1) && (sets.indexOf(co2outstr.charAt(0)) > -1))
        {
          switch (co2outstr.charAt(0))
          {
            case '1':
              printf("Opening set 1\n");
              selectorValves.openSet(0);
              break;
            case '2':
              printf("Opening set 2\n");
              selectorValves.openSet(1);
              break;
            case '3':
              printf("Opening set 1\n");
              selectorValves.openSet(2);
              break;
          }
          co2outstr = "";
        }
        else if (setFlow && (co2outstr.length() > 0))
        {
          int flow = co2outstr.toInt();
          co2outstr = "";
          switch (flowToSet)
          {
            case 's':
              settings.flowSorbentSetPoint = flow;
              //sorbentValve.setManual(flow);
              break;
            case 'a':
              settings.flowAerosolSetPoint = flow;
              //aerosolValve.setManual(flow);
              break;
            case 'c':
              settings.flowCarbonSetPoint = flow;
              //carbonValve.setManual(flow);
              break;
          }
          Serial.printf("Setting flow %c to %d\n", flowToSet, flow);
        }
        else
        {
          Serial.print("Sending '"); Serial.print(co2outstr); Serial.println("'");
          co2.send(co2outstr);
          co2outstr = "";
        }
      }
      else
      {
        if (inchar)
          co2outstr += inchar;
      }
    }
    if (!direct)
    {
      DateTime btime = co2.getMeasurementTime();
      //Serial.printf("state = %c\n",co2.getState());
      /*
      if (co2.getState() == CO2_STATE_MEASURING)
      {
        DateTime mTime;
        float measurement = co2.getMeasurement(mTime);
        Serial.print(mTime.timestamp());
        Serial.print(": ");
        Serial.print(measurement);
        Serial.println (" ppm CO2");

      }
      else if (co2.getState() == CO2_STATE_ZERO)
      {
        Serial.printf("Zeroing, second %d of %d\n", co2.getZeroSec(), co2.getZeroEndSec());
      }
      else if (co2.getState() == CO2_STATE_WARMUP)
      {
        Serial.printf("Warming up, IRGA temperature %.1f\n", co2.getTemperature());
      }
      
    }
    else
    {  
      if (co2.receiveAvailable())
      {
        while(co2.receiveAvailable())
        {
          co2.receive(co2instr);
        }
        Serial.print(co2instr);
        Serial.print("|");
        co2instr = "";
      }
    }
    /*
    valveCount++;
    if (valveCount >= 5)
    {
      valveState = !valveState;
      if (valveState)
        selectorValves.openValve(0,0);
      else
        selectorValves.closeGang(0);
      valveCount = 0;
    }
    */
    switch (co2.getState())
    {
      case CO2_STATE_MEASURING:
        led.setStatus(CONTROL_STATE_SAMPLE_0);
        break;
      case CO2_STATE_WARMUP:
        led.setStatus(CONTROL_STATE_WAIT_TRIGGER);
        break;
      case CO2_STATE_ZERO:
        led.setStatus(CONTROL_STATE_SAMPLE_CO2_ZERO);
        break;
      case CO2_STATE_UNKNOWN:
        led.setStatus(CONTROL_STATE_SENDING_LOG);
        break;
    }
    myDelayMs(INTERVAL_DRIVER_TEST);
  }
}



//**************************************************************************
// RTOS global variables
//**************************************************************************
TaskHandle_t handle_clock_task;
TaskHandle_t handle_driver_tick_task;
TaskHandle_t handle_test_task;

QueueHandle_t handle_command_queue;
QueueHandle_t handle_command_response_queue;
QueueHandle_t handle_data_queue;


void initDrivers()
{
  initRTC();
  i2cMux.begin();
  i2cMux.closeAll();
  mcp.begin();
  mcp.setChannelValue(SORBENT_VALVE_CHANNEL,0);
  mcp.setChannelValue(AEROSOL_VALVE_CHANNEL,0);
  mcp.setChannelValue(CARBON_VALVE_CHANNEL,0);
  now = rtc.now();
  co2.open(19200, &now, 1);
  selectorValves.init();
  selectorValves.setMSClock(&msClock);
  led.init(STATUS_LED_PIN);  
  p1.init();
  p2.init();
}

// the setup routine runs once when you press reset:
void setup() 
{
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  delay(1000); // prevents usb driver crash on startup, do not omit this
  while (!Serial) 
  {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
  Serial.println("USB Serial Initialized");
  // Disable the radio so it does not hold onto the MISO pin
  // and get in the way of the SD card
  // (temporary measure until radio used)
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);

  dataLogger.init();
  initDrivers();
  command.init();
  dataLogger.setCO2Driver(&co2);
  dataLogger.setDateTime(&now);
  dataLogger.beginLogging();

  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);
  vSetErrorSerial(&Serial);

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
  xTaskCreate(task_ms_clock,     "msClock",       128, NULL, tskIDLE_PRIORITY + 3, &handle_clock_task);
  xTaskCreate(task_driver_tick,     "drvrTick",       256, NULL, tskIDLE_PRIORITY + 2, &handle_driver_tick_task);
  xTaskCreate(task_test_drivers, "test", 256, NULL, tskIDLE_PRIORITY + 1, &handle_test_task);

  // Start the RTOS, this function will never return and will schedule the tasks.
  vTaskStartScheduler();

  // error scheduler failed to start
  // should never get here
  while(1)
  {
	  Serial.println("Scheduler Failed! \n");
	  Serial.flush();
	  delay(1000);
  }
}

void loop() 
{
   // Optional commands, can comment/uncomment below
    //Serial.print("."); //print out dots in terminal, we only do this when the RTOS is in the idle state
    //Serial.flush();
    delay(10); //delay is interrupt friendly, unlike vNopDelayMS
}
