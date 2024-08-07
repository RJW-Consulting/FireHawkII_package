
#include <FreeRTOS.h>
#include <croutine.h>
#include <deprecated_definitions.h>
#include <error_hooks.h>
#include <event_groups.h>
#include <FreeRTOS_SAMD51.h>
#include <FreeRTOSConfig.h>
#include <message_buffer.h>
#include <mpu_prototypes.h>
#include <mpu_wrappers.h>
#include <portable.h>
#include <portmacro.h>
#include <projdefs.h>
#include <queue.h>
#include <runTimeStats_hooks.h>
#include <semphr.h>
#include <stack_macros.h>
#include <stream_buffer.h>
#include <task.h>
#include <timers.h>
#include <list.h>



/*
jlink.exe -device ATSAMD51J19 -if SWD -speed 4000 -autoconnect 1
LoadFile ../.build/FireHawk_II_devel.ino.elf
*/

/*
  Proportional Valve Test
  test of Parker 
  
*/
#define PICO_NO_HARDWARE 1

#include <Wire.h>
#include <arduino.h>
#include <Adafruit_MCP4728.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MS8607.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_Sensor.h>
#include <rp2040_pio.h>
#include <TCA9548A-SOLDERED.h>
#include <Adafruit_I2CDevice.h>
#include "RTClib.h"
#include "Driver_ppsystemsCO2.h"
#include "Driver_CO.h"
#include "Driver_selectorValves.h"
#include "Driver_StatusLED.h"
#include "Driver_ProportionalValve.h"
#include "Driver_Pump.h"
#include "Driver_mprls.h"
#include "Driver_PTRH.h"
#include "Driver_Temp_MCP9808.h"
#include "Control.h"
#include "Command.h"
#include "DataLogger.h"
#include "Radio.h"
#include "globals.h"
#include "I2C_Addrs.h"

String versionString = "Firehawk II FW Vers 1.5";

//**************************************************************************
// Type Defines and Constants
//**************************************************************************

#define  ERROR_LED_PIN  13 //Led Pin: Typical Arduino Board

#define ERROR_LED_LIGHTUP_STATE  HIGH // the state that makes the led light up on your board, either low or high


#define RELAY_ON_PIN 5
#define RELAY_OFF_PIN 6
#define VALVE_CONTROL_PIN 10 
#define STATUS_LED_PIN 4

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

#define gas_VALVE_CHANNEL MCP4728_CHANNEL_A
#define oa_VALVE_CHANNEL MCP4728_CHANNEL_C
#define am_VALVE_CHANNEL MCP4728_CHANNEL_B
#define PUMP_CHANNEL MCP4728_CHANNEL_D
#define gas_FLOW_MUX_CHANNEL 0
#define oa_FLOW_MUX_CHANNEL 1
#define am_FLOW_MUX_CHANNEL 2
#define gas_FLOW_ADC_CHANNEL 0x2
#define oa_FLOW_ADC_CHANNEL 0x0
#define am_FLOW_ADC_CHANNEL 0x1
#define PSENSOR1_MUX_CHANNEL 0x04
#define PSENSOR2_MUX_CHANNEL 0x05
#define CO_1_ADC_CHANNEL 0
#define CO_2_ADC_CHANNEL 2
// Global variables
struct Readings readings;
struct Settings settings;
DateTime now;
uint32_t msClock = 0;

// Devices available globally
RTC_PCF8523 rtc;
Adafruit_MS8607 gasPTRH;  // Pressure/Temp/RH sensor in CO sensor hood
Adafruit_INA219 ina219;
Adafruit_MCP4728 mcp;
Adafruit_ADS1115 ads1115_a;
Adafruit_ADS1115 ads1115_b;
Adafruit_MCP9808 mcp9809;
TCA9548A i2cMux;

int flowAOPin = A0;                               

Driver_ppsystemsCO2 co2;
Driver_CO co_1;
Driver_CO co_2;
Driver_selectorValves selectorValves;
Driver_StatusLED led;
Driver_PTRH ptrh;
Driver_Temp_MCP9808 caseTemp;
PressureSensor p1(&i2cMux, PSENSOR1_MUX_CHANNEL, &readings.pressure1);
PressureSensor p2(&i2cMux, PSENSOR2_MUX_CHANNEL, &readings.pressure2);

#define PID_PERIOD 1

//#define USE_HONEYWELL_FLOW 1

Driver_ProportionalValve gasValve(
                  &mcp, 
                  &i2cMux, 
                  gas_FLOW_MUX_CHANNEL,
                  0,
                  &ads1115_b,
                  gas_FLOW_ADC_CHANNEL,
                  gas_VALVE_CHANNEL,
                  &settings.flowGasSetPoint,
                  &readings.flowGas,
                  &settings.gasPIDKp,
                  &settings.gasPIDKi,
                  &settings.gasPIDKd,
                  &settings.gasFlowSlope,
                  &settings.gasFlowIntercept,
                  's',
                  PID_PERIOD
                  );
Driver_ProportionalValve oaValve(
                  &mcp, 
                  &i2cMux, 
                  oa_FLOW_MUX_CHANNEL,
                  0,
                  &ads1115_b,
                  oa_FLOW_ADC_CHANNEL,
                  oa_VALVE_CHANNEL,
                  &settings.flowOaSetPoint,
                  &readings.flowOa,
                  &settings.oaPIDKp,
                  &settings.oaPIDKi,
                  &settings.oaPIDKd,
                  &settings.oaFlowSlope,
                  &settings.oaFlowIntercept,
                  'q',
                  PID_PERIOD
                  );
Driver_ProportionalValve amValve(
                  &mcp, 
                  &i2cMux, 
                  am_FLOW_MUX_CHANNEL,
                  0,
                  &ads1115_b,
                  am_FLOW_ADC_CHANNEL,
                  am_VALVE_CHANNEL,
                  &settings.flowAmSetPoint,
                  &readings.flowAm,
                  &settings.amPIDKp,
                  &settings.amPIDKi,
                  &settings.amPIDKd,
                  &settings.amFlowSlope,
                  &settings.amFlowIntercept,
                  't',
                  PID_PERIOD
                  );

Driver_Pump pump(
                  &mcp, 
                  PUMP_CHANNEL,
                  &settings.samplePumpOn,
                  &settings.samplePumpSpeed);


// Major system components available globally
DataLogger dataLogger;
Command command;
Radio radio;

#define INTERVAL_MS_CLOCK 1
#define INTERVAL_DRIVER_TICK 10
#define INTERVAL_DRIVER_TEST 1000
#define INTERVAL_LED_TICK 100

#define BUTTON_PIN 5
#define BUTTON_INTERRUPT 0

bool button_pushed = false;
uint32_t button_pushed_time;
bool button_down = false;

bool button_is_down()
{
  if (digitalRead(BUTTON_PIN) == LOW)
    button_down = true;
  else
    button_down = false;
  return button_down;
}

void button_ISR()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    button_pushed = true;
    button_pushed_time = now.unixtime();
  }
}

void button_setup()
{
  button_pushed = false;
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // This approach is not recommended, but is necessary since there's a bug in digitalPinToInterrupt():
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_ISR, CHANGE);
}

void button_check()
{
  if (button_pushed)
  {
    if ((now.unixtime() - button_pushed_time) >= 2)
    {
      if (button_is_down())
      {
        settings.radioSilence = !settings.radioSilence;
      }
      button_pushed = false;
    }
    else
    {
      if (!button_is_down())
      {
        button_pushed = false;
      }
    }
  }
}

static void task_ms_clock(void *pvParameters)
{
  Serial.println("Millisecond clock started");

  while(1)
  {
    msClock++;
    myDelayMs(INTERVAL_MS_CLOCK);
  }
}

static void task_status_led(void *pvParameters)
{
  Serial.println("Status LED task started");

  while(1)
  {
    led.tick();
    myDelayMs(INTERVAL_LED_TICK);
  }
}

/*
#define NOSERIAL

void debugOutPrint(char *line)
{
#if ndefined NOSERIAL
  Serial.print(line);
#endif
}

void debugOutPrintln(char *line)
{
#if ndefined NOSERIAL
  Serial.println(line);
#endif
}
*/

bool initialized = false;

void batteryVoltage()
{
  int16_t battOneThirdV;
  battOneThirdV = ads1115_b.readADC_SingleEnded(3);
  readings.batteryV = ((float) battOneThirdV) * ADC_VOLT_CONVERSION * 3;
  if ((readings.batteryV < settings.battThreshold) && (readings.sampleSet != 0))
  {
    command.setSampleSet(0);
  }
}

static void task_driver_tick(void *pvParameters)
{

  Serial.println("driver heartbeat started");
  while(1)
  {
    if (initialized)
    {
      now = rtc.now();
      button_check();
      co2.tick();
      co_1.tick();
      //co_2.tick();
      ptrh.tick();
      caseTemp.tick();
      selectorValves.tick();
      pump.tick();
      gasValve.tick();
      oaValve.tick();    
      amValve.tick();
      //p1.tick();
      //p2.tick();
      radio.tick();
      dataLogger.tick();
      command.tick();
      batteryVoltage();
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
  //gasValve.setManual(2000);
  gasValve.enablePID(enablePID);
  oaValve.enablePID(enablePID);
  amValve.enablePID(enablePID);
  //gasValve.setPWMpin(5);
  //oaValve.setPWMpin(6);
  //amValve.setPWMpin(9);
  initialized = true;
  //selectorValves.openSet(0);



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
    /*
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
    */
    myDelayMs(INTERVAL_DRIVER_TEST);
  }
}



//**************************************************************************
// RTOS global variables
//**************************************************************************
TaskHandle_t handle_clock_task;
TaskHandle_t handle_driver_tick_task;
TaskHandle_t handle_test_task;
TaskHandle_t handle_led_task;

QueueHandle_t handle_command_queue;
QueueHandle_t handle_command_response_queue;
QueueHandle_t handle_data_queue;

SemaphoreHandle_t  i2cMutex;

void initDrivers()
{
  initRTC();
  i2cMux.begin();
  i2cMux.closeAll();
  selectorValves.init();
  selectorValves.setMSClock(&msClock);
  mcp.begin();
  mcp.setChannelValue(gas_VALVE_CHANNEL, 0);
  mcp.setChannelValue(oa_VALVE_CHANNEL, 0);
  mcp.setChannelValue(am_VALVE_CHANNEL, 0);
  mcp.setChannelValue(PUMP_CHANNEL, 0);
  initialized = ads1115_a.begin(GAS_SENSOR_ADC_I2C);
  ads1115_a.setGain(GAIN_TWOTHIRDS);
  initialized = ads1115_b.begin(FLOW_ADC_I2C);
  ads1115_b.setGain(GAIN_TWOTHIRDS);
  pump.init();
  gasValve.init();
  oaValve.init();
  amValve.init();
  now = rtc.now();
  co2.open(19200, &now, 1);
  co_1.init(&now, &readings.coConc1, &readings.coV1, &readings.co1OP1V, &readings.co1OP2V, &settings.coSlope1, &settings.coIntercept1, &ads1115_a,  CO_1_ADC_CHANNEL);
  //co_2.init(&now, &readings.coConc2, &readings.coV2, &settings.coSlope2, &settings.coIntercept2, &ads1115_a,  CO_2_ADC_CHANNEL);
  ptrh.init(&now, &gasPTRH);
  caseTemp.init(&now, &mcp9809, MCP9808_T_SENSOR_I2C_ADDR);
  led.init(STATUS_LED_PIN);  
  //p1.init();
  //p2.init();
  button_setup();
  initialized = true;
}

void initGlobals()
{
  settings.samplePumpOn = false;
  settings.baseStationAnswering = false;
  settings.radioSilence = true;
}

Adafruit_I2CDevice i2c_dev = Adafruit_I2CDevice(0x10);

void i2cAddrTest() {
  int addr;
  while (!Serial) { delay(10); }
  Serial.begin(115200);
  Serial.println("I2C address detection test");
  for (addr = 1; addr < 0xff; addr++)
  {
    i2c_dev = Adafruit_I2CDevice(addr);
    if (!i2c_dev.begin()) {
      Serial.print("Did not find device at 0x");
      Serial.println(i2c_dev.address(), HEX);
    }
    else
    {
      Serial.print("Device found on address 0x");
      Serial.println(i2c_dev.address(), HEX);
    }
  }
}

#define TASK_PRIORITY_LED tskIDLE_PRIORITY + 3
#define TASK_PRIORITY_DRIVER_TICK tskIDLE_PRIORITY + 2
#define TASK_PRIORITY_DRIVER_TEST tskIDLE_PRIORITY + 1
#define TASK_PRIORITY_MS_CLOCK tskIDLE_PRIORITY + 4



// the setup routine runs once when you press reset:
void setup() 
{
  // initialize serial communication at 115200 bits per second:

  Serial.begin(115200);
  delay(1000); // prevents usb driver crash on startup, do not omit this
  /*
  while (!Serial) 
  {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
  */

  Serial.println("USB Serial Initialized");
  Serial.println(versionString);
  // Disable the radio so it does not hold onto the MISO pin
  // and get in the way of the SD card
  // (temporary measure until radio used)
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);

  dataLogger.init();
  initGlobals();
  //i2cAddrTest();
  initDrivers();
  command.init();
  radio.init();


  handle_data_queue = xQueueCreate( RADIO_DATA_QUEUE_NUM_RECORDS, dataLogger.dataPacketSize());
  handle_command_queue = xQueueCreate( RADIO_COMMAND_QUEUE_NUM_RECORDS, RADIO_COMMAND_QUEUE_RECORD_SIZE);
  handle_command_response_queue = xQueueCreate( RADIO_COMMAND_RESPONSE_QUEUE_NUM_RECORDS, RADIO_COMMAND_RESPONSE_QUEUE_RECORD_SIZE);
  dataLogger.setCO2Driver(&co2);
  dataLogger.setDateTime(&now);
  dataLogger.beginLogging();

  vSetErrorLed(ERROR_LED_PIN, ERROR_LED_LIGHTUP_STATE);
  vSetErrorSerial(&Serial);

  // Create mutual exclusion semaphore for I2C bus
  i2cMutex = xSemaphoreCreateMutex();

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
  xTaskCreate(task_ms_clock,     "msClock",       128, NULL, TASK_PRIORITY_MS_CLOCK, &handle_clock_task);
  xTaskCreate(task_driver_tick,     "drvrTick",       1024, NULL, TASK_PRIORITY_DRIVER_TICK, &handle_driver_tick_task);
  xTaskCreate(task_test_drivers, "test", 256, NULL, TASK_PRIORITY_DRIVER_TEST, &handle_test_task);
  xTaskCreate(task_status_led, "test", 128, NULL, TASK_PRIORITY_LED, &handle_led_task);

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
