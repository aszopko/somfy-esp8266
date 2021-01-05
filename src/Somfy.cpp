#include "Somfy.h"

Somfy::Somfy(byte RFSenderPin, unsigned int startId){
	portRF = RFSenderPin;
	startRemoteId = startId;
}

void Somfy::AddRemote(char* remoteName)
{
//for debugging purposes	
/*	String result;
	result = "remotes.size()        : " + String(remotes.size());
	result += "\neepromAddress         : " + String(REMOTE_EEPROM_START_ADDRESS + remotes.size() * sizeof(REMOTE_START_ROLLING_CODE));
	result += "\nREMOTE_START_ROLLING_CODE size        : " + String(sizeof(REMOTE_START_ROLLING_CODE));
*/
	if(remotes.size() < MAX_REMOTES && strlen(remoteName) >= 3)
	{
		REMOTE newRemote = {
			startRemoteId + remotes.size(), 
			remoteName, 
			REMOTE_START_ROLLING_CODE, 
			REMOTE_EEPROM_START_ADDRESS + remotes.size() * sizeof(REMOTE_START_ROLLING_CODE)
		};
		remotes.push_back(newRemote);
	}
	
//	return result;
}

void Somfy::Setup(){
  pinMode(portRF, OUTPUT);
  digitalWrite(portRF, LOW);
  EEPROM.begin(MAX_REMOTES * sizeof(REMOTE_START_ROLLING_CODE));
  for ( REMOTE& remote : remotes ) {
	EEPROM.get(remote.eepromAddress, remote.rollingCode);
 }
}

String Somfy::ProcessMessageAndExecuteCommand(char* message) {
	String result;
	if (strlen(message) >= 4)
	{
		char mqttCommand = message[strlen(message) - 1];		
		message[strlen(message) - 1] = '\0';

		result = "Command: " + String(mqttCommand);
		result += "\nBlindname: " + String(message);

		bool ok;
        REMOTE *selectedRemote = GetRemoteByBlindName(message, ok);
		
		if (!ok)
		{
			result += "\nBlind name not registered";
		}
		else
		{
			switch (mqttCommand)
			{
				case 'U':
					result += "\nUp command received";
					BuildFrame(*selectedRemote, UP);
				break;
				case 'D':
					result += "\nDown command received";
					BuildFrame(*selectedRemote, DOWN);
				break;
				case 'S':
					result += "\nStop command received";
					BuildFrame(*selectedRemote, STOP);
				break;
				case 'P':
					result += "\nProg command received";
					BuildFrame(*selectedRemote, PROG);
				break;
				default:
					result += "\nInvalid command";
					ok = false;
			}
		}
		if(ok)
		{
			result += "\nFrame built";
            SendCommand(2);
            for(int i = 0; i<2; i++) {
              SendCommand(7);
            }          
			result += "\nCommand executed";
		}
		
		//for debugging purposes
		/*Serial.print("Rolling Code in ProcessMessageAndExecuteCommand : "); Serial.println((*selectedRemote).rollingCode);
		for (REMOTE remote : remotes)
		{
			Serial.println("Remote id				:" + String(remote.id));
			Serial.println("Remote name				:" + String(remote.name));
			Serial.println("Remote rolling code		:" + String(remote.rollingCode));
			Serial.println("Remote eeprom start adr	:" + String(remote.eepromAddress));
		}*/
	}	
	else{
		result = "Message too short (must be at least 4 chars long)";
	}

	return result;
}

Somfy::REMOTE* Somfy::GetRemoteByBlindName(char* name, bool &found){
  for (REMOTE& remote : remotes)
	{
		if (strcmp(remote.name, name) == 0){
			found = true;
			return &remote;
		}
	}
	REMOTE empty = {0, "", 0, 0};
	found = false;
	return &empty;
}

void Somfy::BuildFrame(REMOTE &remote, byte button) {

  Serial.print("Remote	      : ");
  Serial.println(remote.name);
  
  frame[0] = 0xA7; // Encryption key. Doesn't matter much
  frame[1] = button << 4;  // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = remote.rollingCode >> 8;    // Rolling code (big endian)
  frame[3] = remote.rollingCode;         // Rolling code
  frame[4] = remote.id >> 16; // Remote address
  frame[5] = remote.id >>  8; // Remote address
  frame[6] = remote.id;       // Remote address


  Serial.print("Frame         : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) { //  Displays leading zero in case the most significant
      Serial.print("0");     // nibble is a 0.
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  
// Checksum calculation: a XOR of all the nibbles
  checksum = 0;
  for(byte i = 0; i < 7; i++) {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111; // We keep the last 4 bits only


//Checksum integration
  frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will
                        // consider the checksum ok.

  Serial.println(""); Serial.print("With checksum : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  
// Obfuscation: a XOR of all the bytes
  for(byte i = 1; i < 7; i++) {
    frame[i] ^= frame[i-1];
  }

  Serial.println(""); Serial.print("Obfuscated    : ");
  for(byte i = 0; i < 7; i++) {
    if(frame[i] >> 4 == 0) {
      Serial.print("0");
    }
    Serial.print(frame[i],HEX); Serial.print(" ");
  }
  Serial.println("");
  Serial.print("Rolling Code  : "); Serial.println(remote.rollingCode);
  remote.rollingCode++;
  EEPROM.put(remote.eepromAddress, remote.rollingCode); 
  EEPROM.commit();                                       
}

void Somfy::SendCommand(byte sync) {
  if(sync == 2) { // Only with the first frame.
  //Wake-up pulse & Silence
    digitalWrite(portRF, HIGH);
    delayMicroseconds(9415);
    digitalWrite(portRF, LOW);
    delayMicroseconds(89565);
  }

// Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++) {
    digitalWrite(portRF, HIGH);
    delayMicroseconds(4*SYMBOL);
    digitalWrite(portRF, LOW);
    delayMicroseconds(4*SYMBOL);
  }

// Software sync
  digitalWrite(portRF, HIGH);
  delayMicroseconds(4550);
  digitalWrite(portRF, LOW);
  delayMicroseconds(SYMBOL);
  
  
//Data: bits are sent one by one, starting with the MSB.
  for(byte i = 0; i < 56; i++) {
    if(((frame[i/8] >> (7 - (i%8))) & 1) == 1) {
      digitalWrite(portRF, LOW);
      delayMicroseconds(SYMBOL);
      digitalWrite(portRF, HIGH);
      delayMicroseconds(SYMBOL);
    }
    else {
      digitalWrite(portRF, HIGH);
      delayMicroseconds(SYMBOL);
      digitalWrite(portRF, LOW);
      delayMicroseconds(SYMBOL);
    }
  }
  
  digitalWrite(portRF, LOW);
  delayMicroseconds(30415); // Inter-frame silence
}