#include "Driver_ProportionalValve.h"

Driver_ProportionalValve::Driver_ProportionalValve(
            Adafruit_MCP4728 *imcp, 
            TCA9548A *ii2cMux, 
            uint_fast8_t imuxChannel,
            uint_fast8_t iflowI2CAddr, 
            uint_fast8_t ivalveChannel,
            double *isetpoint,
            double *ireadback)
{
    mcp = imcp;
    i2cMux = ii2cMux;
    muxChannel = imuxChannel;
    flowI2CAddr = iflowI2CAddr;
    valveChannel = ivalveChannel;
    setpoint = isetpoint;
    readback = ireadback;
    enabled = false;
    //pid = &PID(readback, &valveSetting, setpoint, kp, ki, kd, DIRECT);
    pid = new PID(readback, &valveSetting, setpoint, kp, ki, kd, 0);
    pid->SetOutputLimits(MIN_DAC, MAX_DAC);
    pid->SetMode(AUTOMATIC);
}

#define GET_FLOW_MAX_RETRIES 10

int Driver_ProportionalValve::getFlow()
{
  int flow = 0;
  bool success = false;

  int retry = GET_FLOW_MAX_RETRIES;

  i2cMux->openChannel(muxChannel);
  while (retry)
  {


    Wire.requestFrom(flowI2CAddr,2);
    if (2 <= Wire.available())
    {
      flow = Wire.read();
      flow = flow << 8;
      flow |= Wire.read();
      break;
    }
    else
      retry--;

  }
  i2cMux->closeChannel(muxChannel);
  if (!retry)
  {
    //Serial.println("flow sensor retry failed.");
    flow = -1;
  }
  return flow;
}

#define PWM_MAX 255

void Driver_ProportionalValve::setValvePower(int power)
{
  int setting = power;
  if (pwmPin < 0)
  {
      mcp->setChannelValue((MCP4728_channel_t) valveChannel, (int) setting);
  }
  else
  {
    if (setting > PWM_MAX)
      setting = PWM_MAX;
    analogWrite(pwmPin, setting);
  }
}

void Driver_ProportionalValve::tick()
{
    *readback = getFlow();
    if (manualValveSetting >= 0)
       setValvePower(manualValveSetting);      
    else
    {
      if (!enabled || (*setpoint == 0) || readings.sampleSet == 0)
      {
          setValvePower(MIN_DAC);
      }
      else
      {
          pid->Compute();
          setValvePower((int) valveSetting);
      }
    }
}

