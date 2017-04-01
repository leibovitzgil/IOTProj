#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
int msg;
RF24 radio(9,10);
const uint64_t pipe = 0xE8E8F0F0E1LL;

void setup(void)
{
 Serial.begin(9600);
 radio.begin();
 radio.openWritingPipe(pipe);
}

void loop(void)
{
 
 for (int x=0;x<30;x++)
 {
 msg = x;
 radio.write(&msg, sizeof(msg));
 }
}
