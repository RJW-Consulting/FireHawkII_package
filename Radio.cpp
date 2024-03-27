#include "Radio.h"

#define RFM95_CS    8
#define RFM95_INT   3
#define RFM95_RST   4

#define RF95_FREQ 915.0

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
    if (xQueueReceive(handle_data_queue, (void *) &packetBuffer, 0))
    {
        Serial.println("sending message");
        if (radio_manager.sendtoWait((uint8_t*) &packetBuffer, (uint8_t) sizeof(packetBuffer), settings.stationRadioAddress))
        {
            uint8_t len = sizeof(packetBuffer);
            uint8_t from;   
            if (radio_manager.recvfromAckTimeout((uint8_t*) &packetBuffer, &len, 500, &from))
            {
                Serial.print("got reply from : 0x");
                Serial.print(from, HEX);
                Serial.print(": ");
                Serial.println((char *)&packetBuffer);
            }
            else
            {
                Serial.println("No reply, is rf95_reliable_datagram_server running?");
            }
        }
        else
        {
            Serial.println("Send Failed");
        }
        //radio_driver_rf95.send((uint8_t *) &packetBuffer,  (uint8_t) sizeof(packetBuffer));
        //Serial.println("Waiting for packet to complete...");
        //delay(10);
        //radio_driver_rf95.waitPacketSent();
    }
};
