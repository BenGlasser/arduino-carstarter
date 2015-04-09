/**************************************************************************/
/*! 
    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.

    To enable debug message, define DEBUG in PN532/PN532_debug.h
*/
/**************************************************************************/

#include <SPI.h>
#include <PN532_SPI.h>
#include "PN532.h"
#include <EEPROM.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 10);
PN532 nfc(pn532spi);
NfcAdapter nfcAdaptor = NfcAdapter(pn532spi);

//init constants
int ledPin = 7;  
int ledControl = 6;  
int addr = 0;
byte value;
bool isControlMode = false;

String controlTag = "04 49 40 7A 8F 36 80";

void setup(void) {
  pinMode(ledPin, OUTPUT); 
  pinMode(ledControl, OUTPUT); 
  
  Serial.begin(115200);
  Serial.println("Hello!");
  clearMem();
  readId();
  
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    String tagID = getUidString(uid ,uidLength);
    
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");nfc.PrintHex(uid, uidLength);
    Serial.print(">>>> TAGID: ");Serial.println(tagID);
    Serial.println("");
    
    if (tagID == controlTag)
       {
         Serial.println("Control card detected!");
         digitalWrite(ledControl, HIGH);
         addId(uid, &uidLength, tagID);
         digitalWrite(ledControl, LOW);
       }
       else if (tagID == "04 1A 0E 12 FF 38 85")
       {
         digitalWrite(ledPin, HIGH);
         spin("swithching starter.");
         
         Serial.println("swithching off starter.");
         digitalWrite(ledPin, LOW);
       }
   }
}
// just spin while tag is present.
void spin(String message)
{
   while (nfcAdaptor.tagPresent())
   {
     // spin lock till tag is removed
     Serial.println(message);
   }
}
// add a new  ID
void addId(uint8_t *uid, uint8_t *uidLength, String tagID)
{
   bool isControlMode = true;
   Serial.println("\nScan new entry tag\n");
   
   while(isControlMode)
   {
      if (nfcAdaptor.tagPresent())
      {
         nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, uidLength);
         tagID = getUidString(uid ,*uidLength);
         if (tagID != controlTag)
         {
           writeId(uid, *uidLength);
           Serial.println("New tag keyed for entry!");
           isControlMode = false;
           readId();
         }
         Serial.print("Tag ID: ");
         Serial.println(tagID);
      }
   }
}
String getUidString(const uint8_t *data, const uint32_t numBytes)
{
    String uidString = "";
    for (int i = 0; i < numBytes; i++)
    {
        if (i > 0)
        {
            uidString += " ";
        }

        if (data[i] < 0xF)
        {
            uidString += "0";
        }

        uidString += String((unsigned int)data[i], (unsigned char)HEX);
    }
    uidString.toUpperCase();
    return uidString;
}

void readId()
{  
   uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
   uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
   
   int numBytes = EEPROM.read(addr);
   
   for (int i = 0; i < numBytes; i++)
   {
     
   }
   String id = getUidString(addr+1, EEPROM.read(addr));
   Serial.println(id) 
//  int address = 0;
//  while (address < 512) 
//  {
//    // read a byte from the current address of the EEPROM
//    value = EEPROM.read(address);
//    
//    Serial.print(address);
//    Serial.print("\t");
//    Serial.print(value, DEC);
//    Serial.println();
//    
//    // advance to the next address of the EEPROM
//    address = address + 1;
    
  }
}
void clearMem()
{
  int address = 0;
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(address, 0);
    address = address + 1;
  }
}
void writeId(const uint8_t *data, const uint32_t numBytes)
{
  EEPROM.write(addr, numBytes);
   addr = addr + 1;
  for (int i = 0; i < numBytes; i++)
  {
    EEPROM.write(addr, data[i]);
    addr = addr + 1;
  }
}
