/*
	FPS_GT511C3.h v1.0 - Library for controlling the GT-511C3 Finger Print Scanner (FPS)
	Created by Josh Hawley, July 23rd 2013
	Licensed for non-commercial use, must include this license message
	basically, Feel free to hack away at it, but just give me credit for my work =)
	TLDR; Wil Wheaton's Law
*/

#include "FPS_GT511C3.h"

#ifndef __GNUC__
#pragma region -= Command_Packet Definitions =-
#endif  //__GNUC__

// returns the 12 bytes of the generated command packet
// remember to call delete on the returned array
uint8_t* Command_Packet::GetPacketBytes()
{
	uint8_t* packetbytes= new uint8_t[12];

	// update command before calculating checksum (important!)
	uint16_t cmd = Command;
	command[0] = GetLowByte(cmd);
	command[1] = GetHighByte(cmd);

	uint16_t checksum = _CalculateChecksum();

	packetbytes[0] = COMMAND_START_CODE_1;
	packetbytes[1] = COMMAND_START_CODE_2;
	packetbytes[2] = COMMAND_DEVICE_ID_1;
	packetbytes[3] = COMMAND_DEVICE_ID_2;
	packetbytes[4] = Parameter[0];
	packetbytes[5] = Parameter[1];
	packetbytes[6] = Parameter[2];
	packetbytes[7] = Parameter[3];
	packetbytes[8] = command[0];
	packetbytes[9] = command[1];
	packetbytes[10] = GetLowByte(checksum);
	packetbytes[11] = GetHighByte(checksum);

	return packetbytes;
}

// Converts the uint32_t to bytes and puts them into the parameter array
void Command_Packet::ParameterFrom(uint32_t u)
{
	Parameter[0] = (u & 0x000000ff);
	Parameter[1] = (u & 0x0000ff00) >> 8;
	Parameter[2] = (u & 0x00ff0000) >> 16;
	Parameter[3] = (u & 0xff000000) >> 24;
}

// Returns the high byte from a word
uint8_t Command_Packet::GetHighByte(uint16_t w)
{
	return (uint8_t)(w>>8)&0x00FF;
}

// Returns the low byte from a word
uint8_t Command_Packet::GetLowByte(uint16_t w)
{
	return (uint8_t)w&0x00FF;
}

uint16_t Command_Packet::_CalculateChecksum()
{
	uint16_t w = 0;
	w += COMMAND_START_CODE_1;
	w += COMMAND_START_CODE_2;
	w += COMMAND_DEVICE_ID_1;
	w += COMMAND_DEVICE_ID_2;
	w += Parameter[0];
	w += Parameter[1];
	w += Parameter[2];
	w += Parameter[3];
	w += command[0];
	w += command[1];

	return w;
}

Command_Packet::Command_Packet()
{
};
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= Response_Packet Definitions =-
#endif  //__GNUC__
// creates and parses a response packet from the finger print scanner
Response_Packet::Response_Packet(uint8_t* buffer, bool UseSerialDebug)
{
	CheckParsing(buffer[0], RESPONSE_START_CODE_1, RESPONSE_START_CODE_1, "RESPONSE_START_CODE_1", UseSerialDebug);
	CheckParsing(buffer[1], RESPONSE_START_CODE_2, RESPONSE_START_CODE_2, "RESPONSE_START_CODE_2", UseSerialDebug);
	CheckParsing(buffer[2], RESPONSE_DEVICE_ID_1, RESPONSE_DEVICE_ID_1, "RESPONSE_DEVICE_ID_1", UseSerialDebug);
	CheckParsing(buffer[3], RESPONSE_DEVICE_ID_2, RESPONSE_DEVICE_ID_2, "RESPONSE_DEVICE_ID_2", UseSerialDebug);
	CheckParsing(buffer[8], 0x30, 0x31, "AckNak_LOW", UseSerialDebug);
	if (buffer[8] == 0x30) ACK = true; else ACK = false;
	CheckParsing(buffer[9], 0x00, 0x00, "AckNak_HIGH", UseSerialDebug);

	uint16_t checksum = CalculateChecksum(buffer, 10);
	uint8_t checksum_low = GetLowByte(checksum);
	uint8_t checksum_high = GetHighByte(checksum);
	CheckParsing(buffer[10], checksum_low, checksum_low, "Checksum_LOW", UseSerialDebug);
	CheckParsing(buffer[11], checksum_high, checksum_high, "Checksum_HIGH", UseSerialDebug);

	Error = ErrorCodes::ParseFromBytes(buffer[5], buffer[4]);

	ParameterBytes[0] = buffer[4];
	ParameterBytes[1] = buffer[5];
	ParameterBytes[2] = buffer[6];
	ParameterBytes[3] = buffer[7];
	ResponseBytes[0]=buffer[8];
	ResponseBytes[1]=buffer[9];
	for (int i=0; i < 12; i++)
	{
		RawBytes[i]=buffer[i];
	}
}

// parses bytes into one of the possible errors from the finger print scanner
Response_Packet::ErrorCodes::Errors_Enum Response_Packet::ErrorCodes::ParseFromBytes(uint8_t high, uint8_t low)
{
	Errors_Enum e = INVALID;
	if (high == 0x00)
	{
	}
	// grw 01/03/15 - replaced if clause with else clause for any non-zero high byte
	// if (high == 0x01)
	// {
	else {
		switch(low)
		{
			case 0x00: e = NO_ERROR; break;
			case 0x01: e = NACK_TIMEOUT; break;
			case 0x02: e = NACK_INVALID_BAUDRATE; break;
			case 0x03: e = NACK_INVALID_POS; break;
			case 0x04: e = NACK_IS_NOT_USED; break;
			case 0x05: e = NACK_IS_ALREADY_USED; break;
			case 0x06: e = NACK_COMM_ERR; break;
			case 0x07: e = NACK_VERIFY_FAILED; break;
			case 0x08: e = NACK_IDENTIFY_FAILED; break;
			case 0x09: e = NACK_DB_IS_FULL; break;
			case 0x0A: e = NACK_DB_IS_EMPTY; break;
			case 0x0B: e = NACK_TURN_ERR; break;
			case 0x0C: e = NACK_BAD_FINGER; break;
			case 0x0D: e = NACK_ENROLL_FAILED; break;
			case 0x0E: e = NACK_IS_NOT_SUPPORTED; break;
			case 0x0F: e = NACK_DEV_ERR; break;
			case 0x10: e = NACK_CAPTURE_CANCELED; break;
			case 0x11: e = NACK_INVALID_PARAM; break;
			case 0x12: e = NACK_FINGER_IS_NOT_PRESSED; break;
		}
	}
	return e;
}

// Gets a uint32_t from the parameter bytes
uint32_t Response_Packet::FromParameter()
{
	uint32_t retval = 0;
	retval = (retval << 8) + ParameterBytes[3];
	retval = (retval << 8) + ParameterBytes[2];
	retval = (retval << 8) + ParameterBytes[1];
	retval = (retval << 8) + ParameterBytes[0];
	return retval;
}

// checks to see if the byte is the proper value, and logs it to the serial channel if not
bool Response_Packet::CheckParsing(uint8_t b, uint8_t propervalue, uint8_t alternatevalue, const char* varname, bool UseSerialDebug)
{
	bool retval = (b != propervalue) && (b != alternatevalue);
	if ((UseSerialDebug) && (retval))
	{
		Serial.print("Response_Packet parsing error ");
		Serial.print(varname);
		Serial.print(" ");
		Serial.print(propervalue, HEX);
		Serial.print(" || ");
		Serial.print(alternatevalue, HEX);
		Serial.print(" != ");
		Serial.println(b, HEX);
	}
  return retval;
}

// calculates the checksum from the bytes in the packet
uint16_t Response_Packet::CalculateChecksum(uint8_t* buffer, uint16_t length)
{
	uint16_t checksum = 0;
	for (uint16_t i=0; i<length; i++)
	{
		checksum +=buffer[i];
	}
	return checksum;
}

// Returns the high byte from a word
uint8_t Response_Packet::GetHighByte(uint16_t w)
{
	return (uint8_t)(w>>8)&0x00FF;
}

// Returns the low byte from a word
uint8_t Response_Packet::GetLowByte(uint16_t w)
{
	return (uint8_t)w&0x00FF;
}
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= Data_Packet =-
#endif  //__GNUC__
Data_Packet::Data_Packet(uint8_t* buffer, bool UseSerialDebug)
{
        // The checksum here is arguably useless and may make the serial buffer overflow
    /*CheckParsing(buffer[0], DATA_START_CODE_1, DATA_START_CODE_1, "DATA_START_CODE_1", UseSerialDebug);
	CheckParsing(buffer[1], DATA_START_CODE_2, DATA_START_CODE_2, "DATA_START_CODE_2", UseSerialDebug);
	CheckParsing(buffer[2], DATA_DEVICE_ID_1, DATA_DEVICE_ID_1, "DATA_DEVICE_ID_1", UseSerialDebug);
	CheckParsing(buffer[3], DATA_DEVICE_ID_2, DATA_DEVICE_ID_2, "DATA_DEVICE_ID_2", UseSerialDebug);

	this->checksum = CalculateChecksum(buffer, 4);*/
}

// Get a data packet (128 bytes), calculate checksum and send it to serial
void Data_Packet::GetData(uint8_t buffer[], uint16_t length)
{
    for(uint16_t i = 0; i<length; i++) Serial.write(buffer[i]);
    //this->checksum = CalculateChecksum(buffer, 128); // Checksum slowdown
}

// Get the last data packet (<=128 bytes), calculate checksum, validate checksum received and send it to serial
void Data_Packet::GetLastData(uint8_t buffer[], uint16_t length, bool UseSerialDebug)
{
    for(uint16_t i = 0; i<(length-2); i++) Serial.write(buffer[i]);
        // The checksum here is arguably useless and may make the serial buffer overflow
    /*this->checksum = CalculateChecksum(buffer, length);
	uint8_t checksum_low = GetLowByte(this->checksum);
	uint8_t checksum_high = GetHighByte(this->checksum);
	CheckParsing(buffer[length-2], checksum_low, checksum_low, "Checksum_LOW", UseSerialDebug);
	CheckParsing(buffer[length-1], checksum_high, checksum_high, "Checksum_HIGH", UseSerialDebug);*/
}

// checks to see if the byte is the proper value, and logs it to the serial channel if not
bool Data_Packet::CheckParsing(uint8_t b, uint8_t propervalue, uint8_t alternatevalue, const char* varname, bool UseSerialDebug)
{
	bool retval = (b != propervalue) && (b != alternatevalue);
	if ((UseSerialDebug) && (retval))
	{
		Serial.print("Data_Packet parsing error ");
		Serial.print(varname);
		Serial.print(" ");
		Serial.print(propervalue, HEX);
		Serial.print(" || ");
		Serial.print(alternatevalue, HEX);
		Serial.print(" != ");
		Serial.println(b, HEX);
	}
  return retval;
}

// calculates the checksum from the bytes in the packet
uint16_t Data_Packet::CalculateChecksum(uint8_t* buffer, uint16_t length)
{
	uint16_t checksum = this->checksum;
	for (uint16_t i=0; i<length; i++)
	{
		checksum +=buffer[i];
	}
	return checksum;
}

// Returns the high byte from a word
uint8_t Data_Packet::GetHighByte(uint16_t w)
{
	return (uint8_t)(w>>8)&0x00FF;
}

// Returns the low byte from a word
uint8_t Data_Packet::GetLowByte(uint16_t w)
{
	return (uint8_t)w&0x00FF;
}

#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= FPS_GT511C3 Definitions =-
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= Constructor/Destructor =-
#endif  //__GNUC__
// Creates a new object to interface with the fingerprint scanner
// It will establish the communication to the desired baud rate if defined
FPS_GT511C3::FPS_GT511C3(uint8_t rx, uint8_t tx, uint32_t baud)
	: _serial(rx,tx)
{
	pin_RX = rx;
	pin_TX = tx;
	this->UseSerialDebug = false;
    this->Started = false;
    desiredBaud = baud;
};

// destructor
FPS_GT511C3::~FPS_GT511C3()
{
	_serial.~SoftwareSerial();
}
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= Device Commands =-
#endif  //__GNUC__
//Initialises the device and gets ready for commands
//Returns true if the communication established
bool FPS_GT511C3::Open()
{
    if (!Started) Start();
	if (UseSerialDebug) Serial.println("FPS - Open");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Open;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = true;
	if (rp->ACK == false) retval = false;
	delete rp;
	return retval;
}

// According to the DataSheet, this does nothing...
// Implemented it for completeness.
void FPS_GT511C3::Close()
{
	if (UseSerialDebug) Serial.println("FPS - Close");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Close;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	delete rp;
};

// Turns on or off the LED backlight
// Parameter: true turns on the backlight, false turns it off
// Returns: True if successful, false if not
bool FPS_GT511C3::SetLED(bool on)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CmosLed;
	if (on)
	{
		if (UseSerialDebug) Serial.println("FPS - LED on");
		cp->Parameter[0] = 0x01;
	}
	else
	{
		if (UseSerialDebug) Serial.println("FPS - LED off");
		cp->Parameter[0] = 0x00;
	}
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = true;
	if (rp->ACK == false) retval = false;
	delete rp;
	return retval;
};

// Changes the baud rate of the connection
// Parameter: 9600, 19200, 38400, 57600, 115200
// Returns: True if success, false if invalid baud
// NOTE: Untested (don't have a logic level changer and a voltage divider is too slow)
bool FPS_GT511C3::ChangeBaudRate(uint32_t baud)
{
    if ((baud == 9600) || (baud == 19200) || (baud == 38400) || (baud == 57600) || (baud == 115200))
	{
        if (UseSerialDebug) Serial.println("FPS - ChangeBaudRate");
		Command_Packet* cp = new Command_Packet();
		cp->Command = Command_Packet::Commands::ChangeBaudRate;
		cp->ParameterFrom(baud);
		uint8_t* packetbytes = cp->GetPacketBytes();
		delete cp;
		SendCommand(packetbytes, 12);
		delete packetbytes;
		Response_Packet* rp = GetResponse();
		bool retval = rp->ACK;
		if (retval) _serial.begin(baud);
		delete rp;
		return retval;
	}
	return false;
}

// Gets the number of enrolled fingerprints
// Return: The total number of enrolled fingerprints
uint16_t FPS_GT511C3::GetEnrollCount()
{
	if (UseSerialDebug) Serial.println("FPS - GetEnrolledCount");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::GetEnrollCount;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	uint32_t retval = rp->FromParameter();
	delete rp;
	return retval;
}

// checks to see if the ID number is in use or not
// Parameter: 0-2999, if using GT-521F52
//            0-199, if using GT-521F32/GT-511C3
// Return: True if the ID number is enrolled, false if not
bool FPS_GT511C3::CheckEnrolled(uint16_t id)
{
	if (UseSerialDebug) Serial.println("FPS - CheckEnrolled");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CheckEnrolled;
	cp->ParameterFrom(id);
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = false;
	retval = rp->ACK;
	delete rp;
	return retval;
}

// Starts the Enrollment Process
// Parameter: 0-2999, if using GT-521F52
//            0-199, if using GT-521F32/GT-511C3
// Return:
//	0 - ACK
//	1 - Database is full
//	2 - Invalid Position
//	3 - Position(ID) is already used
uint8_t FPS_GT511C3::EnrollStart(uint16_t id)
{
	if (UseSerialDebug) Serial.println("FPS - EnrollStart");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::EnrollStart;
	cp->ParameterFrom(id);
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	uint8_t retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_DB_IS_FULL) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_INVALID_POS) retval = 2;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_IS_ALREADY_USED) retval = 3;
	}
	delete rp;
	return retval;
}

// Gets the first scan of an enrollment
// Return:
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
uint8_t FPS_GT511C3::Enroll1()
{
	if (UseSerialDebug) Serial.println("FPS - Enroll1");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll1;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	uint32_t retval = rp->FromParameter();
    //Change to  "retval < 3000", if using GT-521F52
    //Leave "reval < 200", if using GT-521F32/GT-511C3
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Gets the Second scan of an enrollment
// Return:
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
uint8_t FPS_GT511C3::Enroll2()
{
	if (UseSerialDebug) Serial.println("FPS - Enroll2");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll2;
	byte* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	uint32_t retval = rp->FromParameter();
    //Change to "retval < 3000", if using GT-521F52
    //Leave "reval < 200", if using GT-521F32/GT-511C3
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Gets the Third scan of an enrollment
// Finishes Enrollment
// Return:
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
uint8_t FPS_GT511C3::Enroll3()
{
	if (UseSerialDebug) Serial.println("FPS - Enroll3");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll3;
	byte* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	uint32_t retval = rp->FromParameter();
    //Change to "retval < 3000", if using GT-521F52
    //Leave "reval < 200", if using GT-521F32/GT-511C3
    if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Checks to see if a finger is pressed on the FPS
// Return: true if finger pressed, false if not
bool FPS_GT511C3::IsPressFinger()
{
	if (UseSerialDebug) Serial.println("FPS - IsPressFinger");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::IsPressFinger;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = false;
    if (!rp->ParameterBytes[0] && !rp->ParameterBytes[1] && !rp->ParameterBytes[2] && !rp->ParameterBytes[3]) retval = true;
	delete rp;
	return retval;
}

// Deletes the specified ID (enrollment) from the database
// Parameter: 0-2999, if using GT-521F52 (id number to be deleted)
//            0-199, if using GT-521F32/GT-511C3(id number to be deleted)
// Returns: true if successful, false if position invalid
bool FPS_GT511C3::DeleteID(uint16_t id)
{
	if (UseSerialDebug) Serial.println("FPS - DeleteID");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::DeleteID;
	cp->ParameterFrom(id);
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	return retval;
}

// Deletes all IDs (enrollments) from the database
// Returns: true if successful, false if db is empty
bool FPS_GT511C3::DeleteAll()
{
	if (UseSerialDebug) Serial.println("FPS - DeleteAll");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::DeleteAll;
	uint8_t* packetbytes = cp->GetPacketBytes();
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	delete cp;
	return retval;
}

// Checks the currently pressed finger against a specific ID
// Parameter: 0-2999, if using GT-521F52 (id number to be checked)
//            0-199, if using GT-521F32/GT-511C3 (id number to be checked)
// Returns:
//	0 - Verified OK (the correct finger)
//	1 - Invalid Position
//	2 - ID is not in use
//	3 - Verified FALSE (not the correct finger)
uint8_t FPS_GT511C3::Verify1_1(uint16_t id)
{
	if (UseSerialDebug) Serial.println("FPS - Verify1_1");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Verify1_1;
	cp->ParameterFrom(id);
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	uint8_t retval = 0;
	if (rp->ACK == false)
	{
		retval = 3; // grw 01/03/15 - set default value of not verified before assignment
		if (rp->Error == Response_Packet::ErrorCodes::NACK_INVALID_POS) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_IS_NOT_USED) retval = 2;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_VERIFY_FAILED) retval = 3;
	}
	delete rp;
	return retval;
}

// Checks the currently pressed finger against all enrolled fingerprints
// Returns:
//	Verified against the specified ID (found, and here is the ID number)
//           0-2999, if using GT-521F52
//           0-199, if using GT-521F32/GT-511C3
//      Failed to find the fingerprint in the database
// 	     3000, if using GT-521F52
//           200, if using GT-521F32/GT-511C3
uint16_t FPS_GT511C3::Identify1_N()
{
	if (UseSerialDebug) Serial.println("FPS - Identify1_N");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Identify1_N;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	uint32_t retval = rp->FromParameter();
//Change to "retval > 3000" and "retval = 3000", if using GT-521F52
//Leave "reval > 200" and "retval = 200", if using GT-521F32/GT-511C3
	if (retval > 200) retval = 200;
	delete rp;
	return retval;
}

// Captures the currently pressed finger into onboard ram use this prior to other commands
// Parameter: true for high quality image(slower), false for low quality image (faster)
// Generally, use high quality for enrollment, and low quality for verification/identification
// Returns: True if ok, false if no finger pressed
bool FPS_GT511C3::CaptureFinger(bool highquality)
{
	if (UseSerialDebug) Serial.println("FPS - CaptureFinger");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CaptureFinger;
	if (highquality)
	{
		cp->ParameterFrom(1);
	}
	else
	{
		cp->ParameterFrom(0);
	}
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	return retval;
}

// Gets an image that is 258x202 (52116 bytes) and sends it over serial
// Returns: True (device confirming download)
    // It only worked with baud rate at 38400-57600 in GT-511C3.
    // Slower speeds and the FPS will shutdown. Higher speeds and the serial buffer will overflow.
    // Make sure you are allocating enough CPU time for this task or you will overflow nonetheless.
    // Also, avoid using UseSerialDebug for this task, since it's easier to overflow.
bool FPS_GT511C3::GetImage()
{
    if (UseSerialDebug) Serial.println("FPS - GetImage");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::GetImage;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
    delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	GetData(52116+6);
	return retval;
}

// Gets an image that is qvga 160x120 (19200 bytes) and sends it over serial
// Returns: True (device confirming download)
    // It only worked with baud rate at 38400-57600 in GT-511C3.
    // Slower speeds and the FPS will shutdown. Higher speeds and the serial buffer will overflow.
    // Make sure you are allocating enough CPU time for this task or you will overflow nonetheless.
    // Also, avoid using UseSerialDebug for this task, since it's easier to overflow.
bool FPS_GT511C3::GetRawImage()
{
    if (UseSerialDebug) Serial.println("FPS - GetRawImage");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::GetRawImage;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
	delete packetbytes;
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	GetData(19200+6);
	return retval;
}

// Gets a template from the fps (498 bytes)
// Parameter: 0-199 ID number
// Returns:
//	0 - ACK Download starting
//	1 - Invalid position
//	2 - ID not used (no template to download
uint8_t FPS_GT511C3::GetTemplate(uint16_t id)
{
    if (UseSerialDebug) Serial.println("FPS - GetTemplate");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::GetTemplate;
	cp->ParameterFrom(id);
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
    delete packetbytes;
	Response_Packet* rp = GetResponse();
	if(rp->ACK)
	{
        delete rp;
        GetData(498+6);
        return 0;
	} else
	{
	    uint32_t retval = rp->FromParameter();
	    delete rp;
	    return retval;
	}
}

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
uint16_t FPS_GT511C3::SetTemplate(byte* tmplt, uint16_t id, bool duplicateCheck)
{
    if (UseSerialDebug) Serial.println("FPS - SetTemplate");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::SetTemplate;
	cp->ParameterFrom(0xFFFF0000 * !duplicateCheck + id); // Will set the HIWORD if duplicateCheck = false
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetbytes, 12);
    delete packetbytes;
	Response_Packet* rp = GetResponse();
	if(!rp->ACK)
	{
        delete rp;
        return 2;
	} else
	{
	    SendCommand(tmplt, 498); // Not actually a command ;)
	    rp = GetResponse();
	    if (rp->ACK)
        {
            delete rp;
            return 0;
        } else
        {
            //Change to  "retval < 3000", if using GT-521F52
            //Leave "reval < 200", if using GT-521F32/GT-511C3
            if (rp->FromParameter() < 200)
            {
                delete rp;
                return 1;
            }
            uint16_t error = rp->Error;
            delete rp;
            if (error == Response_Packet::ErrorCodes::NACK_COMM_ERR) return 3;
            if (error == Response_Packet::ErrorCodes::NACK_DEV_ERR) return 4;
            return -1; // Undefined error
        }
	}
}
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma region -= Not imlemented commands =-
#endif  //__GNUC__
// Commands that are not implemented (and why)
// VerifyTemplate1_1 - Couldn't find a good reason to implement this on an arduino
// IdentifyTemplate1_N - Couldn't find a good reason to implement this on an arduino
// MakeTemplate - Couldn't find a good reason to implement this on an arduino
// UsbInternalCheck - not implemented - Not valid config for arduino
// GetDatabaseStart - historical command, no longer supported
// GetDatabaseEnd - historical command, no longer supported
// UpgradeFirmware - Data sheet says not supported
// UpgradeISOCDImage - Data sheet says not supported
// SetIAPMode - for upgrading firmware (which data sheet says is not supported)
// Ack and Nack	are listed as commands for some unknown reason... not implemented
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__


#ifndef __GNUC__
#pragma region -= Private Methods =-
#endif  //__GNUC__
// Configures the device correctly for communications at the desired baud rate
void FPS_GT511C3::Start()
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Open;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	uint8_t* packetbytes = cp->GetPacketBytes();
	delete cp;

	uint32_t baud = desiredBaud;
    if (!(baud == 9600) && !(baud == 19200) && !(baud == 38400) && !(baud == 57600) && !(baud == 115200)) baud=9600;
	uint32_t actualBaud = 0;
	uint32_t BaudRates[5] = {9600, 19200, 38400, 57600, 115200};
	for(uint8_t i = 0; i<5; i++) // Trying to find FPS baud rate
    {
        if(UseSerialDebug)
        {
            Serial.print("Establishing connection with FPS at baud rate: ");
            Serial.print(BaudRates[i]);
            Serial.println();
        }
        _serial.begin(BaudRates[i]);
        _serial.listen();
        SendCommand(packetbytes, 12);

        uint8_t firstbyte, secondbyte = 0;
        bool done = false;
        uint8_t byteCount = 0;
        while (done == false && byteCount<100)
        {
            byteCount++;
            delay(25);
            if(_serial.peek() == -1) break;
            firstbyte = (uint8_t)_serial.read();
            if (firstbyte == Response_Packet::RESPONSE_START_CODE_1)
            {
                delay(25);
                if(_serial.peek() == -1) break;
                secondbyte = (uint8_t)_serial.read();
                if (secondbyte == Response_Packet::RESPONSE_START_CODE_2)
                {
                    done = true;
                }
            }
        }
        if (!done)
        {
            while (_serial.available()) _serial.read(); // Clear Serial buffer
        } else
        {
            uint8_t resp[12];
            resp[0] = firstbyte;
            resp[1] = secondbyte;
            for (uint8_t i=2; i < 12; i++)
            {
                while (_serial.available() == false) delay(10);
                resp[i]= (uint8_t) _serial.read();
            }
            if (UseSerialDebug)
            {
                Response_Packet* rp = new Response_Packet(resp, UseSerialDebug);
                Serial.print("FPS - RECV: ");
                SendToSerial(rp->RawBytes, 12);
                Serial.println();
                Serial.println();
                delete rp;
            }
            actualBaud = BaudRates[i];
            break;
        }
    }

    if(UseSerialDebug)
    {
        Serial.print("Connection established succesfully. FPS baud rate was: ");
        Serial.print(actualBaud);
        Serial.println();
        Serial.println();
    }

    if (actualBaud == 0) while(true)
    {
        if(UseSerialDebug)
        {
            Serial.print("EXCEPTION: FPS didn't answer to communications. Code execution stopped.");
            Serial.println();
        }
        delay(1000); // Something went terribly wrong with the FPS, and you aren't allowed to leave
    }

    if (actualBaud != baud)
    {
        if(UseSerialDebug)
        {
            Serial.print("Undesired baud rate. Changing baud rate to: ");
            Serial.print(baud);
            Serial.println();
            Serial.println();
        }
        ChangeBaudRate(baud);
    }
    Started = true;
}

// Sends the command to the software serial channel
void FPS_GT511C3::SendCommand(uint8_t cmd[], uint16_t length)
{
	_serial.write(cmd, length);
	if (UseSerialDebug)
	{
		Serial.print("FPS - SEND: ");
		SendToSerial(cmd, length);
		Serial.println();
	}
};

// Gets the response to the command from the software serial channel (and waits for it)
Response_Packet* FPS_GT511C3::GetResponse()
{
	uint8_t firstbyte, secondbyte = 0;
	bool done = false;
	_serial.listen();
	while (done == false)
	{
	    while (_serial.available() == false) delay(10);
		firstbyte = (uint8_t)_serial.read();
		if (firstbyte == Response_Packet::RESPONSE_START_CODE_1)
		{
		    while (_serial.available() == false) delay(10);
		    secondbyte = (uint8_t)_serial.read();
		    if (secondbyte == Response_Packet::RESPONSE_START_CODE_2)
			{
			    done = true;
			}
		}
	}

	uint8_t resp[12];
	resp[0] = firstbyte;
	resp[1] = secondbyte;
	for (uint8_t i=2; i < 12; i++)
	{
		while (_serial.available() == false) delay(10);
		resp[i]= (uint8_t) _serial.read();
	}

	Response_Packet* rp = new Response_Packet(resp, UseSerialDebug);
	if (UseSerialDebug)
	{
		Serial.print("FPS - RECV: ");
		SendToSerial(rp->RawBytes, 12);
		Serial.println();
		Serial.println();
	}
	return rp;
};

// Gets the data (length bytes) from the software serial channel (and waits for it)
// and sends it over serial sommunications
void FPS_GT511C3::GetData(uint16_t length)
{
	uint8_t firstbyte, secondbyte = 0;
	bool done = false;
	_serial.listen();
	while (done == false)
	{
	    while (_serial.available() == false) delay(10);
		firstbyte = (uint8_t)_serial.read();
		if (firstbyte == Data_Packet::DATA_START_CODE_1)
		{
		    while (_serial.available() == false) delay(10);
		    secondbyte = (uint8_t)_serial.read();
		    if (secondbyte == Data_Packet::DATA_START_CODE_2)
            {
                done = true;
            }
		}
	}

    uint8_t firstdata[4];
	firstdata[0] = firstbyte;
	firstdata[1] = secondbyte;
	for (uint8_t i=2; i < 4; i++)
	{
		while (_serial.available() == false) delay(10);
		firstdata[i]= (uint8_t) _serial.read();
	}
	Data_Packet dp(firstdata, UseSerialDebug);

	uint16_t numberPacketsNeeded = (length-4) / 128;
	bool smallLastPacket = false;
	uint8_t lastPacketSize = (length-4) % 128;
	if(lastPacketSize != 0)
	{
            numberPacketsNeeded++;
            smallLastPacket = true;
	}

    uint8_t data[128];
	for (uint16_t packetCount=1; packetCount < numberPacketsNeeded; packetCount++)
    {
        for (uint8_t i=0; i < 128; i++)
        {
            while (_serial.available() == false) delay(1);
            if(_serial.overflow())
            {
                if(UseSerialDebug)
                {
                    Serial.println("Overflow! Data download stopped");
                    Serial.println("Cleaning serial buffer...");
                }
                for (uint16_t j = 0; j<length; j++)
                {
                    _serial.read();
                    delay(1);
                }
                if(UseSerialDebug) Serial.println("Done!");
                return;
            }
            data[i]= (uint8_t) _serial.read();
        }
        dp.GetData(data, 128);
	}

	uint8_t lastdata[lastPacketSize];
	for (uint8_t i=0; i < lastPacketSize; i++)
	{
		while (_serial.available() == false) delay(10);
		lastdata[i]= (uint8_t) _serial.read();
	}
	dp.GetLastData(lastdata, lastPacketSize, UseSerialDebug);
};

// sends the byte array to the serial debugger in our hex format EX: "00 AF FF 10 00 13"
void FPS_GT511C3::SendToSerial(uint8_t data[], uint16_t length)
{
  boolean first=true;
  Serial.print("\"");
  for(uint16_t i=0; i<length; i++)
  {
	if (first) first=false; else Serial.print(" ");
	serialPrintHex(data[i]);
  }
  Serial.print("\"");
}

// sends a byte to the serial debugger in the hex format we want EX "0F"
void FPS_GT511C3::serialPrintHex(uint8_t data)
{
  char tmp[16];
  sprintf(tmp, "%.2X",data);
  Serial.print(tmp);
}
#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

#ifndef __GNUC__
#pragma endregion
#endif  //__GNUC__

