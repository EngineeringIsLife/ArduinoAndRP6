#include <Wire.h>
#include <VirtualWire.h>

#define ADD_RP6 5

#define COMIN_STOP   100
#define COMIN_FWD    101
#define COMIN_RIGHT  102
#define COMIN_LEFT   103
#define COMIN_BWD    104

#define COM_STOP     4  // Sofort anhalten
#define COM_CHDIR    6  // Richtung ändern
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
long msgCount = 0;
long msgCountLoss = 0;

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
    Serial.println("setup");
    
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

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
	int i;
        msgCount++;

        digitalWrite(13, HIGH); // Flash a light to show received good message
	// Message with a good checksum received, dump it.
	Serial.print("Got: ");
	
	for (i = 0; i < buflen; i++)
	{
	    Serial.print(buf[i], HEX);
	    Serial.print(" ");
	}
        if (buf[0] != 'c') sendCommand(buf[0]);  //COMIN_FWD);
        else sendCommand(COMIN_STOP);
        Serial.print("  Loss-Ratio: ");
        Serial.print((float)(msgCountLoss/msgCount));
        Serial.print("   ");
        Serial.print(millis()-startingTime);
        Serial.println("");
        digitalWrite(13, LOW);
        startingTime = millis();
    }

    if (millis()-startingTime > MSG_LOST) {
        Serial.println("Nachricht verloren");
        startingTime += MSG_RATE;
        msgCountLoss++;
        sendCommand(COMIN_STOP);
    }
}

void sendCommand(char command)
{
  int erg;
  
  switch (command) {
    case COMIN_STOP:
      sendCommandStop();
      delay(5);
      sendCommandLED(0b010010);
      break;
      
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
  }
}

void execCommandMovement(struct direct dir)
{
  static int richtung = -1;
  if (richtung != dir.directionID) {
    sendCommandChDir(dir.directionID);
    richtung = dir.directionID;
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
  if (Wire.endTransmission(ADD_RP6) != 0) Serial.println("Fehler beim Übertragen des Stopbefehls");
}

void sendCommandMove(char speed)
{
  Wire.beginTransmission(ADD_RP6);
  Wire.write(0x00);
  Wire.write(COM_MOVSPEED);
  Wire.write(speed);
  Wire.write(speed);
  if (Wire.endTransmission(ADD_RP6) != 0) Serial.println("Fehler beim Übertragen des Bewegungsbefehls");
}

void sendCommandChDir(char dir)
{
  Wire.beginTransmission(ADD_RP6);
  Wire.write(0x00);
  Wire.write(COM_CHDIR);
  Wire.write(dir);
  if (Wire.endTransmission(ADD_RP6) != 0) Serial.println("Fehler beim Übertragen der Bewegungsrichtung");
}

void sendCommandLED(char leds)
{
  Wire.beginTransmission(ADD_RP6);
  Wire.write(0x00);
  Wire.write(COM_SETLED);
  Wire.write(leds);
  if (Wire.endTransmission(ADD_RP6) != 0) Serial.println("Fehler beim Übertragen der LED-Beschaltung");
}
