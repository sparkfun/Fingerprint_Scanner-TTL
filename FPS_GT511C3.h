/* 
	FPS_GT511C3.h v1.0 - Library for controlling the GT-511C3 Finger Print Scanner (FPS)
	Created by Josh Hawley, July 23rd 2013
	Licensed for non-commercial use, must include this license message
	basically, Feel free to hack away at it, but just give me credit for my work =)
	TLDR; Wil Wheaton's Law
*/

#ifndef FPS_GT511C3_h
#define FPS_GT511C3_h

#include "Arduino.h";
#include "SoftwareSerial.h";
#pragma region -= Command_Packet =-
/*
	Command_Packet represents the 12 byte command that we send to the finger print scanner
*/
class Command_Packet
{
	public:
		class Commands
		{
			public:
				enum Commands_Enum
				{
					NotSet				= 0x00,		// Default value for enum. Scanner will return error if sent this.
					Open				= 0x01,		// Open Initialization
					Close				= 0x02,		// Close Termination
					UsbInternalCheck	= 0x03,		// UsbInternalCheck Check if the connected USB device is valid
					ChangeEBaudRate		= 0x04,		// ChangeBaudrate Change UART baud rate
					SetIAPMode			= 0x05,		// SetIAPMode Enter IAP Mode In this mode, FW Upgrade is available
					CmosLed				= 0x12,		// CmosLed Control CMOS LED
					GetEnrollCount		= 0x20,		// Get enrolled fingerprint count
					CheckEnrolled		= 0x21,		// Check whether the specified ID is already enrolled
					EnrollStart			= 0x22,		// Start an enrollment
					Enroll1				= 0x23,		// Make 1st template for an enrollment
					Enroll2				= 0x24,		// Make 2nd template for an enrollment
					Enroll3				= 0x25,		// Make 3rd template for an enrollment, merge three templates into one template, save merged template to the database
					IsPressFinger		= 0x26,		// Check if a finger is placed on the sensor
					DeleteID			= 0x40,		// Delete the fingerprint with the specified ID
					DeleteAll			= 0x41,		// Delete all fingerprints from the database
					Verify1_1			= 0x50,		// Verification of the capture fingerprint image with the specified ID
					Identify1_N			= 0x51,		// Identification of the capture fingerprint image with the database
					VerifyTemplate1_1	= 0x52,		// Verification of a fingerprint template with the specified ID
					IdentifyTemplate1_N	= 0x53,		// Identification of a fingerprint template with the database
					CaptureFinger		= 0x60,		// Capture a fingerprint image(256x256) from the sensor
					MakeTemplate		= 0x61,		// Make template for transmission
					GetImage			= 0x62,		// Download the captured fingerprint image(256x256)
					GetRawImage			= 0x63,		// Capture & Download raw fingerprint image(320x240)
					GetTemplate			= 0x70,		// Download the template of the specified ID
					SetTemplate			= 0x71,		// Upload the template of the specified ID
					GetDatabaseStart	= 0x72,		// Start database download, obsolete
					GetDatabaseEnd		= 0x73,		// End database download, obsolete
					UpgradeFirmware		= 0x80,		// Not supported
					UpgradeISOCDImage	= 0x81,		// Not supported
					Ack					= 0x30,		// Acknowledge.
					Nack				= 0x31		// Non-acknowledge
			};
		};

		Commands::Commands_Enum Command;
		byte Parameter[4];								// Parameter 4 bytes, changes meaning depending on command							
		byte* GetPacketBytes();							// returns the bytes to be transmitted
		void ParameterFromInt(int i);

		Command_Packet();

	private: 
		static const byte COMMAND_START_CODE_1 = 0x55;	// Static byte to mark the beginning of a command packet	-	never changes
		static const byte COMMAND_START_CODE_2 = 0xAA;	// Static byte to mark the beginning of a command packet	-	never changes
		static const byte COMMAND_DEVICE_ID_1 = 0x01;	// Device ID Byte 1 (lesser byte)							-	theoretically never changes
		static const byte COMMAND_DEVICE_ID_2 = 0x00;	// Device ID Byte 2 (greater byte)							-	theoretically never changes
		byte command[2];								// Command 2 bytes

		word _CalculateChecksum();						// Checksum is calculated using byte addition
		byte GetHighByte(word w);						
		byte GetLowByte(word w);
};
#pragma endregion

#pragma region -= Response_Packet =-
/*
	Response_Packet represents the returned data from the finger print scanner 
*/
class Response_Packet
{
	public:
		class ErrorCodes
		{
			public:
				enum Errors_Enum
				{
					NO_ERROR					= 0x0000,	// Default value. no error
					NACK_TIMEOUT				= 0x1001,	// Obsolete, capture timeout
					NACK_INVALID_BAUDRATE		= 0x1002,	// Obsolete, Invalid serial baud rate
					NACK_INVALID_POS			= 0x1003,	// The specified ID is not between 0~199
					NACK_IS_NOT_USED			= 0x1004,	// The specified ID is not used
					NACK_IS_ALREADY_USED		= 0x1005,	// The specified ID is already used
					NACK_COMM_ERR				= 0x1006,	// Communication Error
					NACK_VERIFY_FAILED			= 0x1007,	// 1:1 Verification Failure
					NACK_IDENTIFY_FAILED		= 0x1008,	// 1:N Identification Failure
					NACK_DB_IS_FULL				= 0x1009,	// The database is full
					NACK_DB_IS_EMPTY			= 0x100A,	// The database is empty
					NACK_TURN_ERR				= 0x100B,	// Obsolete, Invalid order of the enrollment (The order was not as: EnrollStart -> Enroll1 -> Enroll2 -> Enroll3)
					NACK_BAD_FINGER				= 0x100C,	// Too bad fingerprint
					NACK_ENROLL_FAILED			= 0x100D,	// Enrollment Failure
					NACK_IS_NOT_SUPPORTED		= 0x100E,	// The specified command is not supported
					NACK_DEV_ERR				= 0x100F,	// Device Error, especially if Crypto-Chip is trouble
					NACK_CAPTURE_CANCELED		= 0x1010,	// Obsolete, The capturing is canceled
					NACK_INVALID_PARAM			= 0x1011,	// Invalid parameter
					NACK_FINGER_IS_NOT_PRESSED	= 0x1012,	// Finger is not pressed
					INVALID						= 0XFFFF	// Used when parsing fails
				};

				static Errors_Enum ParseFromBytes(byte high, byte low);
		};
		Response_Packet(byte* buffer, bool UseSerialDebug);
		ErrorCodes::Errors_Enum Error;
		byte RawBytes[12];
		byte ParameterBytes[4];
		byte ResponseBytes[2];
		bool ACK;
		static const byte COMMAND_START_CODE_1 = 0x55;	// Static byte to mark the beginning of a command packet	-	never changes
		static const byte COMMAND_START_CODE_2 = 0xAA;	// Static byte to mark the beginning of a command packet	-	never changes
		static const byte COMMAND_DEVICE_ID_1 = 0x01;	// Device ID Byte 1 (lesser byte)							-	theoretically never changes
		static const byte COMMAND_DEVICE_ID_2 = 0x00;	// Device ID Byte 2 (greater byte)							-	theoretically never changes
		int IntFromParameter();

	private: 
		bool CheckParsing(byte b, byte propervalue, byte alternatevalue, char* varname, bool UseSerialDebug);
		word CalculateChecksum(byte* buffer, int length);
		byte GetHighByte(word w);						
		byte GetLowByte(word w);
};
#pragma endregion

#pragma region -= Data_Packet =- 
// Data Mule packet for receiving large data(in 128 byte pieces) from the FPS
// This class can only transmit one packet at a time
//class Data_Packet
//{
//public:
//	static int CheckSum;
//	int PacketID;
//	int ValidByteLength;
//	byte Data[128];
//	void StartNewPacket();
//	bool IsLastPacket;
//private:
//	static int NextPacketID;
//};
#pragma endregion


/*
	Object for controlling the GT-511C3 Finger Print Scanner (FPS)
*/
class FPS_GT511C3
{
 
 public:
	// Enables verbose debug output using hardware Serial 
	bool UseSerialDebug;

	#pragma region -= Constructor/Destructor =-
	// Creates a new object to interface with the fingerprint scanner
	FPS_GT511C3(uint8_t rx, uint8_t tx);
	
	// destructor
	~FPS_GT511C3();
	#pragma endregion


	#pragma region -= Device Commands =-
	//Initialises the device and gets ready for commands
	void Open();

	// Does not actually do anything (according to the datasheet)
	// I implemented open, so had to do closed too... lol
	void Close();

	// Turns on or off the LED backlight
	// LED must be on to see fingerprints
	// Parameter: true turns on the backlight, false turns it off
	// Returns: True if successful, false if not
	bool SetLED(bool on);
	
	// Changes the baud rate of the connection
	// Parameter: 9600 - 115200
	// Returns: True if success, false if invalid baud
	// NOTE: Untested (don't have a logic level changer and a voltage divider is too slow)
	bool ChangeBaudRate(int baud);

	// Gets the number of enrolled fingerprints
	// Return: The total number of enrolled fingerprints
	int GetEnrollCount();

	// checks to see if the ID number is in use or not
	// Parameter: 0-199
	// Return: True if the ID number is enrolled, false if not
	bool CheckEnrolled(int id);

	// Starts the Enrollment Process
	// Parameter: 0-199
	// Return:
	//	0 - ACK
	//	1 - Database is full
	//	2 - Invalid Position
	//	3 - Position(ID) is already used
	int EnrollStart(int id);

	// Gets the first scan of an enrollment
	// Return: 
	//	0 - ACK
	//	1 - Enroll Failed
	//	2 - Bad finger
	//	3 - ID in use
	int Enroll1();

	// Gets the Second scan of an enrollment
	// Return: 
	//	0 - ACK
	//	1 - Enroll Failed
	//	2 - Bad finger
	//	3 - ID in use
	int Enroll2();

	// Gets the Third scan of an enrollment
	// Finishes Enrollment
	// Return: 
	//	0 - ACK
	//	1 - Enroll Failed
	//	2 - Bad finger
	//	3 - ID in use
	int Enroll3();

	// Checks to see if a finger is pressed on the FPS
	// Return: true if finger pressed, false if not
	bool IsPressFinger();

	// Deletes the specified ID (enrollment) from the database
	// Returns: true if successful, false if position invalid
	bool DeleteID(int ID);

	// Deletes all IDs (enrollments) from the database
	// Returns: true if successful, false if db is empty
	bool DeleteAll();

	// Checks the currently pressed finger against a specific ID
	// Parameter: 0-199 (id number to be checked)
	// Returns:
	//	0 - Verified OK (the correct finger)
	//	1 - Invalid Position
	//	2 - ID is not in use
	//	3 - Verified FALSE (not the correct finger)
	int Verify1_1(int id);

	// Checks the currently pressed finger against all enrolled fingerprints
	// Returns:
	//	0-199: Verified against the specified ID (found, and here is the ID number)
	//	200: Failed to find the fingerprint in the database
	int Identify1_N();

	// Captures the currently pressed finger into onboard ram
	// Parameter: true for high quality image(slower), false for low quality image (faster)
	// Generally, use high quality for enrollment, and low quality for verification/identification
	// Returns: True if ok, false if no finger pressed
	bool CaptureFinger(bool highquality);
	#pragma endregion

	#pragma region -= Not implemented commands =-
	// Gets an image that is 258x202 (52116 bytes) and returns it in 407 Data_Packets
	// Use StartDataDownload, and then GetNextDataPacket until done
	// Returns: True (device confirming download starting)
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//bool GetImage();

	// Gets an image that is qvga 160x120 (19200 bytes) and returns it in 150 Data_Packets
	// Use StartDataDownload, and then GetNextDataPacket until done
	// Returns: True (device confirming download starting)
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//bool GetRawImage();

	// Gets a template from the fps (498 bytes) in 4 Data_Packets
	// Use StartDataDownload, and then GetNextDataPacket until done
	// Parameter: 0-199 ID number
	// Returns: 
	//	0 - ACK Download starting
	//	1 - Invalid position
	//	2 - ID not used (no template to download
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//int GetTemplate(int id);

	// Uploads a template to the fps 
	// Parameter: the template (498 bytes)
	// Parameter: the ID number to upload
	// Parameter: Check for duplicate fingerprints already on fps
	// Returns: 
	//	0-199 - ID duplicated
	//	200 - Uploaded ok (no duplicate if enabled)
	//	201 - Invalid position
	//	202 - Communications error
	//	203 - Device error
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//int SetTemplate(byte* tmplt, int id, bool duplicateCheck);

	// Commands that are not implemented (and why)
	// VerifyTemplate1_1 - Couldn't find a good reason to implement this on an arduino
	// IdentifyTemplate1_N - Couldn't find a good reason to implement this on an arduino
	// MakeTemplate - Couldn't find a good reason to implement this on an arduino
	// UsbInternalCheck - not implemented - Not valid config for arduino
	// GetDatabaseStart - historical command, no longer supported
	// GetDatabaseEnd - historical command, no longer supported
	// UpgradeFirmware - Data Sheet says not supported
	// UpgradeISOCDImage - Data Sheet says not supported
	// SetIAPMode - for upgrading firmware (which is not supported)
	// Ack and Nack	are listed as a commands for some unknown reason... not implemented
	#pragma endregion

	#pragma endregion

	void serialPrintHex(byte data);
	void SendToSerial(byte data[], int length);

	// resets the Data_Packet class, and gets ready to download
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//void StartDataDownload();

	// Returns the next data packet 
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//Data_Packet GetNextDataPacket();

private:
	 void SendCommand(byte cmd[], int length);
	 Response_Packet* GetResponse();
	 uint8_t pin_RX,pin_TX;
	 SoftwareSerial _serial;
};


#endif

