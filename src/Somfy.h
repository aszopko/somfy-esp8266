/*

	ESP8266 Somfy
	
	Info and inspiration from the following projects:
	
	-	https://4x5mg.net/2018/06/10/controlling-somfy-blinds-with-esp8266/
	-	https://github.com/Nickduino/Somfy_Remote/blob/master/Somfy_Remote.ino
	-	https://nodemcu.readthedocs.io/en/release/modules/somfy/
	-	https://github.com/RoyOltmans/somfy_esp8266_remote_arduino/blob/master/somfy_remote_arduino.ino
	
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <vector>

#ifndef Esp8266Somfy_h
#define Esp8266Somfy_h

class Somfy
{
	
  public:

    Somfy(byte, unsigned int);
	void AddRemote(char* remoteName);
    void Setup();
	String ProcessMessageAndExecuteCommand(char*);

  private:

	#define SYMBOL 640
	#define UP 0x2
	#define STOP 0x1
	#define DOWN 0x4
	#define PROG 0x8

	#define REMOTE_EEPROM_START_ADDRESS 0
	#define REMOTE_START_ROLLING_CODE 1	//doesn't really matter since it will be read from the memory - so the number will be quite random
	#define MAX_REMOTES 16 // <- max number of remotes

	struct REMOTE {
		unsigned int id;
		char const* name;
		unsigned int rollingCode;
		uint32_t eepromAddress;
	};

	byte frame[7];
	byte checksum;
	byte portRF;
	std::vector<REMOTE> remotes;
	unsigned int startRemoteId;
	
	REMOTE* GetRemoteByBlindName(char*, bool&);
	void BuildFrame(REMOTE&, byte);
	void SendCommand(byte);
};

#endif

