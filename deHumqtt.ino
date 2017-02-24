#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>

#include <EEPROM.h>
#include "EEPROMAnything.h"

struct config_led
{
    int pin;
    boolean state;
} ledConfig;

char stringBuf[25];

byte mac[]    = {  0x90, 0xA2, 0xDA, 0x00, 0x5B, 0x26 };
IPAddress ip(172, 16, 0, 147);
IPAddress server(172, 16, 0, 70);

# define HUM_PIN 7

void callback(char* topic, byte* payload, unsigned int length)
{
  
  payload[length] = '\0';
  String message = (char *) payload;
  
  if(length == 2 && message == "on")
  {
    on(HUM_PIN);
  }
  
  if(length == 3 && message == "off")
  {
    off(HUM_PIN);
  }

  if(length == 0 || length == 6 && message == "toggle")
  {
    toggle(HUM_PIN);
  }
  
  if(length == 5 && message == "state")
  {
    pubState(HUM_PIN, digitalRead(HUM_PIN));
  }

}


EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup()
{
  EEPROM_readAnything(0, ledConfig);
  
  if(ledConfig.state)
  {
    changeState(HIGH, ledConfig.pin, false);
  }
  else
  {
    changeState(LOW, ledConfig.pin, false);
  }  
  
  Ethernet.begin(mac, ip);
  pinMode(HUM_PIN, OUTPUT);
}

void loop()
{
  
  if(client.connected())
  {
      client.loop();
  }
  else
  {
    client.connect("deHumqtt");
    delay(5000);
    client.subscribe("vw/dehum");
  }
 
}

void on(int pin)
{
  if(digitalRead(pin) == LOW)
  {
    changeState(HIGH, pin, true);
  }
}

void off(int pin)
{
  if(digitalRead(pin) == HIGH)
  {
    changeState(LOW, pin, true);
  }
}

void toggle(int pin)
{
  changeState(!digitalRead(pin), pin, true);
}

void pubState(int pin, boolean state)
{
  if(state) {
    client.publish("vw/dehum/state", "on");
  }
  else {
    client.publish("vw/dehum/state", "off");
  }
}

void saveState(int pin, boolean state)
{
  ledConfig.pin = pin;
  ledConfig.state = state;
  EEPROM_writeAnything(0, ledConfig);
}

void changeState(boolean state, int pin, boolean publishState)
{
  
  digitalWrite(pin, state);
  
  if(publishState)
  {
    pubState(pin, state);
  }
   
   saveState(pin, state);
   
}

