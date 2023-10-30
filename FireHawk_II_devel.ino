#include "FreeRTOS_SAMD21.h"

#include <Adafruit_MCP4728.h>

/*
  Proportional Valve Test
  test of Parker 
  
*/

#include <Wire.h>
#include <Adafruit_INA219.h>
#include "RTClib.h"
#include "Driver_ppsystemsCO2.h"
#include "Driver_selectorValves.h"
#include "DataLogger.h"


RTC_PCF8523 rtc;
Adafruit_INA219 ina219;
Adafruit_MCP4728 mcp;

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

// Global variables
int flowAOPin = A0;                               
DateTime now;
uint32_t msClock = 0;
Driver_ppsystemsCO2 co2;
Driver_selectorValves selectorValves;
//DataLogger dataLogger;

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

static void task_driver_tick(void *pvParameters)
{
  Serial.println("driver heartbeat started");
  while(1)
  {
    now = rtc.now();
    co2.tick();
    selectorValves.tick();
    //dataLogger.tick();
    myDelayMs(INTERVAL_DRIVER_TICK);
  }
}

String co2outstr = "";
String co2instr = "";

bool direct = false; 

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

static void task_test_drivers(void *pvParameters)
{
  char inchar;
  while (1)
  {
  
    //Serial.println("Hello");
    //delay(5000);
    //co2.send("this");


    while (Serial.available())
    {
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
      if ((inchar == '\r') || (inchar == '\n')) 
      {
        Serial.print("Sending '"); Serial.print(co2outstr); Serial.println("'");
        co2.send(co2outstr);
        co2outstr = "";
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
    myDelayMs(INTERVAL_DRIVER_TEST);
  }
}



//**************************************************************************
// global variables
//**************************************************************************
TaskHandle_t handle_clock_task;
TaskHandle_t handle_driver_tick_task;
TaskHandle_t handle_test_task;

void initDrivers()
{
  initRTC();
  now = rtc.now();
  co2.open(19200, &now, 1);
  selectorValves.init();
  selectorValves.setMSClock(&msClock);  
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
  initDrivers();
  //dataLogger.setCO2Driver(&co2);
  //dataLogger.setDateTime(&now);
  //dataLogger.beginLogging();

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
    Serial.print("."); //print out dots in terminal, we only do this when the RTOS is in the idle state
    Serial.flush();
    delay(100); //delay is interrupt friendly, unlike vNopDelayMS
}
