#include <Wire.h>
#include <VirtualWire.h>

#define ADD_RP6 5

#define COMIN_STOP   100
#define COMIN_FWD    101
#define COMIN_RIGHT  102
#define COMIN_LEFT   103
#define COMIN_BWD    104

#define COM_STOP     4  // Stop movement
#define COM_CHDIR    6  // Change direction
#define COM_MOVSPEED 5  // Move at speed
#define COM_SETLED   3  // Set LEDs

#define DIR_FWD      0
#define DIR_BWD      1
#define DIR_LBWD     2
#define DIR_RBWD     3

#define MSG_RATE 250
#define MSG_LOST 500
#define BITRATE  1000

long startingTime;

struct direct {
  char directionID;
  char leds;
  char speed;
};

direct fwd   = {DIR_FWD,  0b100100, 0x40};
direct bwd   = {DIR_BWD,  0b001001, 0x40};
direct left  = {DIR_LBWD, 0b111000, 0x20};
direct right = {DIR_RBWD, 0b000111, 0x20};

void setup()
{
    Serial.begin(9600);	// Debugging only
    
    Wire.begin();
    pinMode(13, OUTPUT);

    // Initialise the IO and ISR
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_set_rx_pin(12);
    vw_setup(BITRATE);	 // Bits per sec

    vw_rx_start();       // Start the receiver PLL running
    
    startingTime = millis();
}

void loop()
{
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    char c[VW_MAX_MESSAGE_LEN];

    if (vw_get_message(buf, &buflen))
    {
        digitalWrite(13, HIGH);
	
        Serial.println("Received: ");
	for (int i = 0; i < buflen; i++)
	{
	    Serial.print(buf[i], HEX);
	    Serial.print(" ");
	}

        sendCommand(buf[0]);
        Serial.println("");
        digitalWrite(13, LOW);
        startingTime = millis();
    }

    if (millis()-startingTime > MSG_LOST) {
        Serial.println("No message received");
        sendCommand(COMIN_STOP);
        startingTime = millis();
    }
}

void sendCommand(char command)
{
  int erg;
  
  switch (command) {
    case COMIN_FWD:
      execCommandMovement(fwd);
      break;
    
    case COMIN_BWD:
      execCommandMovement(bwd);
      break;
      
    case COMIN_RIGHT:
      execCommandMovement(right);
      break;
      
    case COMIN_LEFT:
      execCommandMovement(left);
      break;
      
    case COMIN_STOP:
    default:
      sendCommandStop();
      delay(5);
      sendCommandLED(0b010010);
      break;
  }
}

void execCommandMovement(struct direct dir)
{
  static int direction = -1;
  if (direction != dir.directionID) {
    sendCommandChDir(dir.directionID);
    direction = dir.directionID;
    delay(5);
  }        
  sendCommandMove(dir.speed);
  delay(5);
  sendCommandLED(dir.leds); 
}

void sendCommandStop(void)
{  
  Wire.beginTransmission(ADD_RP6);
  Wire.write(0x00);
  Wire.write(COM_STOP);
  if (Wire.endTransmission() != 0) Serial.println("Error on transmission of command STOP");
}

void sendCommandMove(char speed)
{
  Wire.beginTransmission(ADD_RP6);
  Wire.write(0x00);
  Wire.write(COM_MOVSPEED);
  Wire.write(speed);
  Wire.write(speed);
  if (Wire.endTransmission() != 0) Serial.println("Error on transmission of command MOVEATSPEED");
}

void sendCommandChDir(char dir)
{
  Wire.beginTransmission(ADD_RP6);
  Wire.write(0x00);
  Wire.write(COM_CHDIR);
  Wire.write(dir);
  if (Wire.endTransmission() != 0) Serial.println("Error on transmission of command CHANGE DIRECTION");
}

void sendCommandLED(char leds)
{
  Wire.beginTransmission(ADD_RP6);
  Wire.write(0x00);
  Wire.write(COM_SETLED);
  Wire.write(leds);
  if (Wire.endTransmission() != 0) Serial.println("Error on transmission of command SETLEDS");
}
