#include "RFIDCardReader.h"

#include "Logger.h"

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include <uFCoder.h>

constexpr auto DEFAULT_BYTES_TO_READ = 17;

RFIDCardReader* RFIDCardReader::globalCardReader = new RFIDCardReader(true);

RFIDCardReader* RFIDCardReader::getCardReader() {
    return globalCardReader;
}

string RFIDCardReader::convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

string RFIDCardReader::uidToString(uint8_t* uid, uint8_t uidSize)
{
    stringstream sout;
    for (int i = 0; i < uidSize; i++)
    {
        sout << hex << (int)uid[i];
        if (i < uidSize - 1) {
            sout << ":";
        }
    }
    return sout.str();
}

RFIDCardReader::RFIDCardReader(bool autoInitialize)
{
    readerOpen = false;
    cardPresent = false;
    cardUID = "";
    lastCardData = "";
    lastStatus = UFR_OK;

    if (autoInitialize) initialize();
}

RFIDCardReader::~RFIDCardReader() {}

bool RFIDCardReader::isStateOkay(const wstring& message) const
{
    if (lastStatus != UFR_OK && lastStatus != UFR_NO_CARD) {
        wstringstream wout;
        wout << message << L":" << endl;
        wout << L"\t" << UFR_Status2String((UFR_STATUS)lastStatus) << endl;
        logger.logError(wout.str());
        return false;
    }

    return true;
}

void RFIDCardReader::poll()
{
    // Only proceed if the reader is open
    if (readerOpen)
    {
        // Attempt to update the card info
        string oldUID = cardUID;
        if (updateCardInfo())
        {
            // Is this a new card
            if (oldUID != cardUID)
            {
                uint16_t bytes = DEFAULT_BYTES_TO_READ;
                readCardData(lastCardData, bytes);
                logger.log("New RFID Card UID: " + cardUID);
                logger.log("RFID Card Data: " + lastCardData);
            }
        }
    }
}

bool RFIDCardReader::initialize()
{
    // Open connection with card reader
    lastStatus = ReaderOpenEx(1, "", 2, NULL);
    if (!isStateOkay(L"Error opening reader"))
    {
        readerOpen = false;
        return false;
    }

    // Query reader type
    lastStatus = GetReaderType(&readerType);
    readerOpen = isStateOkay(L"Error getting reader type");

    // Flash lights and beep to indicate readyness
    if (readerOpen)
    {
        logger.log("RFID reader opened");
        lastStatus = ReaderUISignal(1, 1);
        readerOpen = isStateOkay(L"Error sending light/beep acknowledgement");
    }

    return readerOpen;
}

bool RFIDCardReader::updateCardInfo()
{
    // Detect card and read its UID
    uint8_t sak, uidSize;
    uint8_t uid[16];
    lastStatus = GetCardIdEx(&sak, uid, &uidSize);
    cardPresent = isStateOkay(L"Error reading card ID") && lastStatus != UFR_NO_CARD;

    // If a card is present, update the uid
    if (cardPresent)
    {
        cardUID = uidToString(uid, uidSize);
        return true;
    }

    // No card present or error and return failure
    return false;
}

bool RFIDCardReader::readCardData(string& dataOut, uint16_t &bytes)
{
    // Read data back from the card
    uint8_t* baReadData = new uint8_t[DEFAULT_BYTES_TO_READ];
    lastStatus = LinearRead(baReadData, 0, DEFAULT_BYTES_TO_READ, &bytes, MIFARE_AUTHENT1A, 0);
    if (!isStateOkay(L"Error reading data from RFID Card"))
    {
        return false;
    }

    // Convert to string and return success
    dataOut = convertToString((char*)baReadData, DEFAULT_BYTES_TO_READ - 1);
    lastCardData = dataOut;
    return true;
}

bool RFIDCardReader::writeCardData(const string& dataIn, uint16_t& bytes)
{
    lastStatus = LinearWrite((const uint8_t*)dataIn.c_str(), 0, bytes, &bytes, MIFARE_AUTHENT1A, 0);
    if (!isStateOkay(L"Error writing data to RFID Card"))
    {
        return false;
    }

    lastCardData = dataIn;
    return true;
}

string RFIDCardReader::getLastCardData() {
    return lastCardData;
}

void RFIDCardReader::clearLastCardData() {
    lastCardData = "";
    cardUID = "";
}
