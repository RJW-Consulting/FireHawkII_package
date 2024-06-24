#include "Radio.h"


// Singleton instance of the radio driver
RH_RF95 radio_driver_rf95(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram radio_manager(radio_driver_rf95);


void Radio::init()
{
    initHardware();
    radio_manager.setThisAddress(settings.droneRadioAddress);
    if (!radio_driver_rf95.init())
        Serial.println("Radio driver init failed");
    if (!radio_manager.init())
        Serial.println("Radio manager init failed");
    if (!radio_driver_rf95.setFrequency(RF95_FREQ))
        Serial.println("Radio set frequency failed");
    radio_driver_rf95.setTxPower(20, false);

};

void Radio::initHardware()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

};

void Radio::tick()
{
    if (radio_manager.available())
    {
        uint8_t len = sizeof(packetBuffer);
        uint8_t from;
        if (radio_manager.recvfromAck((uint8_t *) &packetBuffer, &len, &from))
        {
            //packetBuffer.StringPacket[len-1] = 0;
            xQueueSend(handle_command_queue, &packetBuffer.stringPacket.chars, portMAX_DELAY);
        }
    }

    if (xQueueReceive(handle_command_response_queue, (void *) &packetBuffer, 0))
    {
        Serial.println("sending command response");
        if (radio_manager.sendtoWait((uint8_t*) &packetBuffer, (uint8_t) sizeof(packetBuffer), settings.stationRadioAddress))
        {
            Serial.println("Send Response Succeeded");        
        }
        else
        {
            Serial.println("Send Response Failed");
        }
    }

    if (xQueueReceive(handle_data_queue, (void *) &packetBuffer, 0))
    {
        if (radio_manager.sendtoWait((uint8_t*) &packetBuffer, (uint8_t) sizeof(packetBuffer), settings.stationRadioAddress))
        {
            Serial.println("Send Succeeded"); 
            settings.baseStationAnswering = true;      
        }
        else
        {
            Serial.println("Send Failed");
            settings.baseStationAnswering = false;      
        }
    }

};
