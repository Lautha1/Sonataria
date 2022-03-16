#pragma once

#include <string>

class RFIDCardReader
{
public:
	static RFIDCardReader* getCardReader();

	~RFIDCardReader();

	bool initialize();
	void poll();

	bool updateCardInfo();

	bool readCardData(std::string& dataOut, uint16_t& bytes);
	bool writeCardData(const std::string& dataIn, uint16_t& bytes);

	std::string getLastCardData();
	
	void clearLastCardData();

protected:
	static RFIDCardReader* globalCardReader;
	RFIDCardReader(bool autoInitialize = false);

	bool isStateOkay(const std::wstring& message = L"RFID Reader Error") const;

	static std::string convertToString(char* a, int size);
	static std::string uidToString(uint8_t* uid, uint8_t uidSize);

	bool readerOpen;
	bool cardPresent;

	// Reader & Card status
	std::string lastCardData;
	uint32_t lastStatus;
	uint32_t readerType;

	// Card ID info
	std::string cardUID;
};
