#include <VirtualWire.h>

#define TAKT 250
#define BITRATE 1000

// Commands:
#define COM_STOP   0
#define COM_FWD    1
#define COM_RIGHT  2
#define COM_LEFT   3
#define COM_BWD    4

long startTime;

void setup() {
  Serial.begin(9600);
  startTime = millis();
  
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_tx_pin(12);
  vw_setup(BITRATE);	 // Bits per sec
}

void loop() {
  long actualTime = millis();
  long dt = actualTime - startTime;
  char inByte;

  if (Serial.available() > 0) {
    inByte = Serial.read();
    sendCommand(inByte, inByte);
    startTime = actualTime;
  }

  else if (dt > TAKT)
  {
    startTime = actualTime;    
    sendCommand(100, 100);
  }  
}

void sendCommand(char command, char value)
{
  char msg[1];
  msg[0] = command;
  msg[1] = value;
  vw_send((uint8_t *)msg, strlen(msg));
  vw_wait_tx(); // Wait until the whole message is gone
  Serial.println(command);
}
