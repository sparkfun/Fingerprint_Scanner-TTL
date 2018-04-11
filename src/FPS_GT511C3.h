/*
	FPS_GT511C3.h v1.0 - Library for controlling the GT-511C3 Finger Print Scanner (FPS)
	Created by Josh Hawley, July 23rd 2013
	Licensed for non-commercial use, must include this license message
	basically, Feel free to hack away at it, but just give me credit for my work =)
	TLDR; Wil Wheaton's Law
*/

#ifndef FPS_GT511C3_h
#define FPS_GT511C3_h

#include "Arduino.h"
#include "SoftwareSerial.h"
#ifndef __GNUC__
#pragma region -= Command_Packet =-
#endif  //__GNUC__
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
					ChangeBaudRate		= 0x04,		// ChangeBaudrate Change UART baud rate
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
		uint8_t Parameter[4];								// Parameter 4 bytes, changes meaning depending on command
		uint8_t* GetPacketBytes();							// returns the bytes to be transmitted
		void ParameterFrom(uint32_t u);

		Command_Packet();

	private:
		static const uint8_t COMMAND_START_CODE_1 = 0x55;	// Static byte to mark the beginning of a command packet	-	never changes
		static const uint8_t COMMAND_START_CODE_2 = 0xAA;	// Static byte to mark the beginning of a command packet	-	never changes
		static const uint8_t COMMAND_DEVICE_ID_1 = 0x01;	// Device ID Byte 1 (lesser byte)							-	theoretically never changes
		static const uint8_t COMMAND_DEVICE_ID_2 = 0x00;	// Device ID Byte 2 (greater byte)							-	theoretically never changes
		uint8_t command[2];								// Command 2 bytes

		uint16_t _CalculateChecksum();						// Checksum is calculated using byte addition
		uint8_t GetHighByte(word w);
		uint8_t GetLowByte(word w);
};
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= Response_Packet =-
#endif  //__GNUC__
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
		Response_Packet(uint8_t* buffer, bool UseSerialDebug);
		ErrorCodes::Errors_Enum Error;
		uint8_t RawBytes[12];
		uint8_t ParameterBytes[4];
		uint8_t ResponseBytes[2];
		bool ACK;
		static const uint8_t RESPONSE_START_CODE_1 = 0x55;	// Static byte to mark the beginning of a command packet	-	never changes
		static const uint8_t RESPONSE_START_CODE_2 = 0xAA;	// Static byte to mark the beginning of a command packet	-	never changes
		static const uint8_t RESPONSE_DEVICE_ID_1 = 0x01;	// Device ID Byte 1 (lesser byte)							-	theoretically never changes
		static const uint8_t RESPONSE_DEVICE_ID_2 = 0x00;	// Device ID Byte 2 (greater byte)							-	theoretically never changes
		uint32_t FromParameter();

	private:
		bool CheckParsing(uint8_t b, uint8_t propervalue, uint8_t alternatevalue, const char* varname, bool UseSerialDebug);
		uint16_t CalculateChecksum(uint8_t* buffer, uint16_t length);
		uint8_t GetHighByte(uint16_t w);
		uint8_t GetLowByte(uint16_t w);
};
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= Data_Packet =-
#endif  //__GNUC__
// Data Mule packet for receiving large data(in 128 byte pieces) from the FPS
// This class can only transmit one packet at a time
class Data_Packet
{
public:
    Data_Packet(uint8_t* buffer, bool UseSerialDebug);
    uint16_t checksum = 0;
    static const uint8_t DATA_START_CODE_1 = 0x5A;	// Static byte to mark the beginning of a data packet	-	never changes
    static const uint8_t DATA_START_CODE_2 = 0xA5;	// Static byte to mark the beginning of a data packet	-	never changes
    static const uint8_t DATA_DEVICE_ID_1 = 0x01;	// Device ID Byte 1 (lesser byte)							-	theoretically never changes
    static const uint8_t DATA_DEVICE_ID_2 = 0x00;	// Device ID Byte 2 (greater byte)

    void GetData(uint8_t buffer[], uint16_t length, bool UseSerialDebug);
	void GetLastData(uint8_t buffer[], uint16_t length, bool UseSerialDebug);
private:
	bool CheckParsing(uint8_t b, uint8_t propervalue, uint8_t alternatevalue, const char* varname, bool UseSerialDebug);
	uint16_t CalculateChecksum(uint8_t* buffer, uint16_t length);
    uint8_t GetHighByte(uint16_t w);
    uint8_t GetLowByte(uint16_t w);
};
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__


/*
	Object for controlling the GT-511C3 Finger Print Scanner (FPS)
*/
class FPS_GT511C3
{

 public:
	// Enables verbose debug output using hardware Serial
	bool UseSerialDebug;
	uint32_t desiredBaud;

#ifndef __GNUC__
	#pragma region -= Constructor/Destructor =-
#endif  //__GNUC__
	// Creates a new object to interface with the fingerprint scanner
	// It will establish the communication to the desired baud rate if defined
	FPS_GT511C3(uint8_t rx, uint8_t tx, uint32_t baud = 9600);

	// destructor
	~FPS_GT511C3();
#ifndef __GNUC__
	#pragma endregion
#endif  //__GNUC__


#ifndef __GNUC__
	#pragma region -= Device Commands =-
#endif  //__GNUC__
	//Initialises the device and gets ready for commands
	//Returns true if the communication established
	bool Open();

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
	bool ChangeBaudRate(uint32_t baud);

	// Gets the number of enrolled fingerprints
	// Return: The total number of enrolled fingerprints
	uint16_t GetEnrollCount();

	// checks to see if the ID number is in use or not
	// Parameter: 0-2999, if using GT-521F52
        //            0-199, if using GT-521F32/GT-511C3
	// Return: True if the ID number is enrolled, false if not
	bool CheckEnrolled(uint16_t id);

	// Starts the Enrollment Process
	// Parameter: 0-2999, if using GT-521F52
        //            0-199, if using GT-521F32/GT-511C3
	// Return:
	//	0 - ACK
	//	1 - Database is full
	//	2 - Invalid Position
	//	3 - Position(ID) is already used
	uint8_t EnrollStart(uint16_t id);

	// Gets the first scan of an enrollment
	// Return:
	//	0 - ACK
	//	1 - Enroll Failed
	//	2 - Bad finger
	//	3 - ID in use
	uint8_t Enroll1();

	// Gets the Second scan of an enrollment
	// Return:
	//	0 - ACK
	//	1 - Enroll Failed
	//	2 - Bad finger
	//	3 - ID in use
	uint8_t Enroll2();

	// Gets the Third scan of an enrollment
	// Finishes Enrollment
	// Return:
	//	0 - ACK
	//	1 - Enroll Failed
	//	2 - Bad finger
	//	3 - ID in use
	uint8_t Enroll3();

	// Checks to see if a finger is pressed on the FPS
	// Return: true if finger pressed, false if not
	bool IsPressFinger();

	// Deletes the specified ID (enrollment) from the database
	// Returns: true if successful, false if position invalid
	bool DeleteID(uint16_t ID);

	// Deletes all IDs (enrollments) from the database
	// Returns: true if successful, false if db is empty
	bool DeleteAll();

	// Checks the currently pressed finger against a specific ID
	// Parameter: 0-2999, if using GT-521F52 (id number to be checked)
        //            0-199, if using GT-521F32/GT-511C3 (id number to be checked)
	// Returns:
	//	0 - Verified OK (the correct finger)
	//	1 - Invalid Position
	//	2 - ID is not in use
	//	3 - Verified FALSE (not the correct finger)
	uint8_t Verify1_1(uint16_t id);

	// Checks the currently pressed finger against all enrolled fingerprints
	// Returns:
	//	Verified against the specified ID (found, and here is the ID number)
        //           0-2999, if using GT-521F52
        //           0-199, if using GT-521F32/GT-511C3
        //      Failed to find the fingerprint in the database
        // 	     3000, if using GT-521F52
        //           200, if using GT-521F32/GT-511C3
	uint16_t Identify1_N();

	// Captures the currently pressed finger into onboard ram
	// Parameter: true for high quality image(slower), false for low quality image (faster)
	// Generally, use high quality for enrollment, and low quality for verification/identification
	// Returns: True if ok, false if no finger pressed
	bool CaptureFinger(bool highquality);

    // Gets an image that is 258x202 (52116 bytes + 2 bytes checksum) and sends it over serial
    // Returns: True (device confirming download)
	bool GetImage();

	// Gets an image that is qvga 160x120 (19200 bytes + 2 bytes checksum) and sends it over serial
    // Returns: True (device confirming download)
	bool GetRawImage();

    // Gets a template from the fps (498 bytes + 2 bytes checksum)
	// Parameter: 0-199 ID number
	// Returns:
	//	0 - ACK Download starting
	//	1 - Invalid position
	//	2 - ID not used (no template to download
	uint8_t GetTemplate(uint16_t id);

	// Uploads a template to the fps
	// Parameter: the template (498 bytes)
	// Parameter: the ID number to upload
	// Parameter: Check for duplicate fingerprints already on fps
    // Returns:
    // -1 - Undefined error (shouldn't ever happen)
    //	0 - Uploaded ok (no duplicate if enabled)
    //	1 - ID duplicated
    //	2 - Invalid position
    //	3 - Communications error
    //	4 - Device error
	uint16_t SetTemplate(byte* tmplt, uint16_t id, bool duplicateCheck);
#ifndef __GNUC__
	#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
	#pragma region -= Not implemented commands =-
#endif  //__GNUC__
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
#ifndef __GNUC__
  #pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
	#pragma endregion
#endif  //__GNUC__

	void serialPrintHex(uint8_t data);
	void SendToSerial(uint8_t data[], uint16_t length);

private:

    // Indicates if the communication was configured for the first time
	bool Started;

    //Configures the device correctly for communications at the desired baud rate
    void Start();

    void SendCommand(uint8_t cmd[], uint16_t length);
    Response_Packet* GetResponse();
    void GetData(uint16_t length);
    uint8_t pin_RX,pin_TX;
    SoftwareSerial _serial;
};


#endif

