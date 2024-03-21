#include "Driver_ProportionalValve.h"

Driver_ProportionalValve::Driver_ProportionalValve(
            Adafruit_MCP4728 *imcp, 
            TCA9548A *ii2cMux, 
            uint_fast8_t imuxChannel,
            uint_fast8_t iflowI2CAddr,
            Adafruit_ADS1115 *iflowADC,
            uint_fast8_t iflowADCChannel, 
            uint_fast8_t ivalveChannel,
            double *isetpoint,
            double *ireadback,
            double *pKp,
            double *pKi,
            double *pKd,
            char marker,
            uint_fast8_t period)
{
  which = marker;
  mcp = imcp;
  i2cMux = ii2cMux;
  muxChannel = imuxChannel;
  flowI2CAddr = iflowI2CAddr;
  valveChannel = ivalveChannel;
  flowADC = iflowADC;
  flowADCChannel = iflowADCChannel;
  setpoint = isetpoint;
  readback = ireadback;
  kp = pKp;
  ki = pKi;
  kd = pKd;
  enabled = false;
  //pid = &PID(readback, &valveSetting, setpoint, kp, ki, kd, DIRECT);
  pid = new PID(readback, &valveSetting, setpoint, *kp, *ki, *kd, 0);
  pid->SetOutputLimits(MIN_DAC, MAX_DAC);
  pid->SetMode(AUTOMATIC);
  pidPeriod = period;
  periodCount = 0;
 }

void Driver_ProportionalValve::init()
{
  initFlowBuffer();
  setValvePower(MIN_DAC);
  updateKs();
}

void Driver_ProportionalValve::updateKs()
{
  ksChanged = true;
}

#define GET_FLOW_MAX_RETRIES 10

void Driver_ProportionalValve::initFlowBuffer()
{
  bufferHead = 0;
  bufferTail = 0;
  bufferSize = 0;
  bufferSum = 0;
  for (uint i = 0; i < FLOW_BUFFER_SIZE; i++)
  {
    flowBuffer[i] = 0;
  }  
}

void Driver_ProportionalValve::addFlowPoint(int flow)
{
 #if FLOW_BUFFER_SIZE > 0 
    if (bufferSize == FLOW_BUFFER_SIZE) 
    {
        // Remove the oldest data point if the buffer is full
        bufferSum -= flowBuffer[bufferHead];
        bufferHead = (bufferHead + 1) % FLOW_BUFFER_SIZE;
    }

    flowBuffer[bufferTail] = flow;
    bufferTail = (bufferTail + 1) % FLOW_BUFFER_SIZE;
    bufferSize++;
    if (bufferSize > FLOW_BUFFER_SIZE)
      bufferSize = FLOW_BUFFER_SIZE;
    bufferSum += flow;
  #endif
}

int Driver_ProportionalValve::getMeanFlow()
{
    if (bufferSize == 0) {
      return 0; // Avoid division by zero when the buffer is empty
    }
    return (int)(bufferSum / bufferSize);

}

/* code for Honeywell flow sensor with analog output */

static std::vector<FlowData> flowTable = {
    {0, 1.00},
    {1000, 2.90},
    {2000, 3.75},
    {3000, 4.35},
    {4000, 4.65},
    {5000, 4.89},
    {6000, 5.00},
    {10000, 6.00}
};

#define ADC_VOLT_CONVERSION 0.000188058298072402

int Driver_ProportionalValve::getAnalogFlow()
{
  int_fast16_t analogSignal = flowADC->readADC_SingleEnded(flowADCChannel);
  double flowVolts = ((double)analogSignal * ADC_VOLT_CONVERSION);
  double flowSCCM = calculateFlowRate(flowVolts, flowTable);
  /*
  Serial.print("volts: ");
  Serial.println(flowVolts);
  Serial.print("flow: ");
  Serial.println(flowSCCM);
  */
  return flowSCCM;
}

double Driver_ProportionalValve::calculateFlowRate(double voltage, const std::vector<FlowData>& flowTable) 
{
    double retValue = -1;
    // Check if voltage is outside the range
    if (voltage < flowTable.front().voltage_vdc || voltage > flowTable.back().voltage_vdc) {
        //std::cerr << "Input voltage is out of range!" << std::endl;
        return -1; // Error code
    }

    // Perform linear interpolation
    for (size_t i = 0; i < flowTable.size() - 1; ++i) 
    {
        if (voltage >= flowTable[i].voltage_vdc && voltage <= flowTable[i + 1].voltage_vdc) 
        {
            double flow1 = flowTable[i].flow_sccm;
            double flow2 = flowTable[i + 1].flow_sccm;
            double voltage1 = flowTable[i].voltage_vdc;
            double voltage2 = flowTable[i + 1].voltage_vdc;
            // Linear interpolation formula
            return flow1 + ((flow2 - flow1) / (voltage2 - voltage1)) * (voltage - voltage1);
        }
    }
    return retValue;
}

int Driver_ProportionalValve::getFlow()
{
  int flow = 0;
  bool success = false;

  int retry = GET_FLOW_MAX_RETRIES;

  if (flowI2CAddr == 0) 
  {
    flow = getAnalogFlow();
  }
  else
  {
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
  // 
  if (FLOW_BUFFER_SIZE > 0)
  {
    addFlowPoint(flow);
    *readback = getMeanFlow();
  }
  else
    *readback = flow;
  periodCount++;
  if (periodCount == pidPeriod)
  {
    periodCount = 0;
    int flow = getFlow();
    /*
    Serial.print(which);
    Serial.print(flow);
    Serial.print(':');
    Serial.print(valveSetting);
    */


    if (manualValveSetting >= 0)
    {
      valveSetting = manualValveSetting;
      setValvePower(manualValveSetting);      
    }
    else
    {
      if (!enabled || (*setpoint == 0) || readings.sampleSet == 0)
      {
          setValvePower(MIN_DAC); 
          valveSetting = MIN_DAC;
      }
      else
      {
          if (ksChanged)
          {
            pid->SetTunings(*kp,*ki,*kd);
            ksChanged = false;
          }
          pid->Compute();
          setValvePower((int) valveSetting);
      }
    }
  }
}

