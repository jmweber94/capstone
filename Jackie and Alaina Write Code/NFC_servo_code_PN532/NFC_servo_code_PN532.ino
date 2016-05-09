/* Arduino Code for Team 17: SMART
 *
 */
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a software SPI connection (recommended):
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <Servo.h>
Servo daServo;

#define LED_ON LOW
#define LED_OFF HIGH

#define redLed 6		// Set Led Pins
#define greenLed 7
#define blueLed 11

#define relay 9			// Set Relay Pin (this is actully our servo)

boolean match = false;          // initialize card match to false
boolean programMode = false;	// initialize programming mode to false
boolean locked = false;        // initialize brake to be on and motor to be powered

boolean successRead;		          // Variable integer to keep if we have Successful Read from Reader
int timeRelease = 0;          //initialize to false

byte storedCard[7];		// Stores an ID read from EEPROM
byte readCard[7];		// Stores scanned ID read from RFID Module
byte masterCard[7];		// Stores master card's ID read from EEPROM
byte uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
byte uidLength = 7;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

void setup() { 
  daServo.attach(relay);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(relay, OUTPUT);
  daServo.write(0);		// Make sure door is locked
  digitalWrite(redLed, LED_OFF);	// Make sure led is off
  digitalWrite(greenLed, LED_OFF);	// Make sure led is off
  digitalWrite(blueLed, LED_OFF);	// Make sure led is off

  //Protocol Configuration
  Serial.begin(115200);	 // Initialize serial communications with PC
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

  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine Master Card
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
  if (EEPROM.read(1) != 143) {  		
    Serial.println(F("No Master Card Defined"));
    Serial.println(F("Scan A PICC to Define as Master Card"));
    do {
      successRead = getID();               // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(blueLed, LED_ON);       // Visualize Master Card need to be defined
      delay(200);
      digitalWrite(blueLed, LED_OFF);
      delay(200);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( int j = 0; j < 7; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, uid[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
    Serial.println(F("Master Card Defined"));
  }
  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  for ( int i = 0; i < 7; i++ ) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
  }
  
  nfc.PrintHex(masterCard,uidLength);
  
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Waiting PICCs to be scanned"));
  Serial.println("");
  cycleLeds();    // Everything ready lets give user some feedback by cycling leds
}

///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  boolean success = false; 
  do {
    success = getID(); 	                     // sets success to 1 when we get read from reader otherwise 0
    if (programMode) {
      Serial.println("Program Mode Lights");
      cycleLeds();                               // Program Mode cycles through RGB waiting to read a new card
    }
    else {
      //normalModeOn(); 		                       // Normal mode, blue Power LED is on, all others are off
      Serial.println("Normal Mode Lights");
    }
  }
  while (!success); 	                       // the program will not go further while you not get a successful read
  Serial.println(locked);
  if (programMode) {
    Serial.println("programMode = True");
    doProgramMode();
  }
  else {                                         // if we're not in program mode
    //Serial.println("first else");
    if ( isMaster(uid) && !locked) {  	     // If scanned card's ID matches Master Card's ID and it's not locked, enter program mode
      programMode = true;
      Serial.println("Master Scanned, programMode = True");
    }
    else {
      //Serial.println("second else");
      if ( isMaster(uid) && locked) {       // if master card and locked
        Serial.println(F("UNLOCKED"));
        unlock();
      }
      else {
        //Serial.println("third else");
        //if ( findID(uid) && !locked) {	     // If not master card but the tag is in memory, lock the device
        if ( !locked) {
          Serial.println(F("LOCKED"));
          lock();
        }
      }
    }
  }
}

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void lock () {
  digitalWrite(blueLed, LED_OFF); 	// Turn off blue LED
  digitalWrite(redLed, LED_ON); 	// Turn on red LED
  digitalWrite(greenLed, LED_OFF); 	// Turn off green LED
  daServo.write(150); 		// brake device
  locked = true;
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void unlock() {
  digitalWrite(greenLed, LED_ON); 	// Make sure green LED is off
  digitalWrite(blueLed, LED_OFF); 	// Make sure blue LED is off
  digitalWrite(redLed, LED_OFF); 	// Turn on red LED
  daServo.write(0);     // brake device
  delay(1000);
  locked = false;
}
//////////////////////////////////////////////////////////////////////////////////////////

void doProgramMode(){
  Serial.println(F("Entered Program Mode"));
      int count = EEPROM.read(0);                              // Read the first Byte of EEPROM that
      Serial.print(F("I have "));                              // stores the number of ID's in EEPROM
      Serial.print(count);
      Serial.print(F(" record(s) on EEPROM"));
      Serial.println("");
      Serial.println(F("Scan a PICC to ADD or REMOVE"));
      Serial.println(F("-----------------------------"));
  if ( isMaster(uid) ) {                                  // If master card scanned again exit program mode
      Serial.println(F("Master Card Scanned"));
      Serial.println(F("Exiting Program Mode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if ( findID(uid) ) {                                // If scanned card is known delete it
        Serial.println(F("Removing..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
      }
      else {                                                   // If scanned card is not known add it
        Serial.println(F("Adding..."));
        writeID(uid);
        Serial.println(F("-----------------------------"));
      }
    }
}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
boolean getID() {
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  // Getting ready for Reading PICCs
  if ( ! nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) { //If a new PICC placed to RFID reader continue
    return false;
  }

  Serial.println(F("Scanned PICC's UID:"));
  nfc.PrintHex(uid, uidLength);
  Serial.println("");
  return true;
}

////////////////////////////////// Cycle Leds (Program Mode) /////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LED_OFF); 	// Make sure red LED is off
  digitalWrite(greenLed, LED_ON); 	// Make sure green LED is on
  digitalWrite(blueLed, LED_OFF); 	// Make sure blue LED is off
  delay(200);
  digitalWrite(redLed, LED_OFF); 	// Make sure red LED is off
  digitalWrite(greenLed, LED_OFF); 	// Make sure green LED is off
  digitalWrite(blueLed, LED_ON); 	// Make sure blue LED is on
  delay(200);
  digitalWrite(redLed, LED_ON); 	// Make sure red LED is on
  digitalWrite(greenLed, LED_OFF); 	// Make sure green LED is off
  digitalWrite(blueLed, LED_OFF); 	// Make sure blue LED is off
  delay(200);
}

//////////////////////////////////////// Normal Mode Led /////////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON); 	// Blue LED ON and ready to read card
  digitalWrite(redLed, LED_OFF); 	// Make sure Red LED is off
  digitalWrite(greenLed, LED_OFF); 	// Make sure Green LED is off
  daServo.write(50); 		// Make sure Door is Locked
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( int number ) {
  int start = (number * 7 ) + 2; 		// Figure out starting position
  for ( int i = 0; i < 7; i++ ) { 		// Loop 7 times to get the 7 Bytes
    storedCard[i] = EEPROM.read(start + i); 	// Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Add ID to EEPROM ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) { 		// Before we write to the EEPROM, check to see if we have seen this card before!
    int num = EEPROM.read(0); 		// Get the numer of used spaces, position 0 stores the number of ID cards
    int start = ( num * 7 ) + 6; 	// Figure out where the next slot starts
    num++; 								// Increment the counter by one
    EEPROM.write( 0, num ); 		// Write the new count to the counter
    for ( int j = 0; j < 7; j++ ) { 	// Loop 4 times
      EEPROM.write( start + j, a[j] ); 	// Write the array values to EEPROM in the right position
    }
    successWrite();
	Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else {
    failedWrite();
	Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) { 		// Before we delete from the EEPROM, check to see if we have this card!
    failedWrite(); 			// If not
	Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    int num = EEPROM.read(0); 	// Get the numer of used spaces, position 0 stores the number of ID cards
    int slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    int start = (slot * 7) + 2;			// = ( num * 4 ) + 6; // Figure out where the next slot starts
    int looping = ((num - slot) * 7); 		// The number of times the loop repeats
    int j;
    int count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    num--; 			// Decrement the counter by one
    EEPROM.write( 0, num ); 	// Write the new count to the counter
    for ( j = 0; j < looping; j++ ) { 				// Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 7 + j)); 	// Shift the array values to 4 places earlier in the EEPROM
    }
    for ( int k = 0; k < 7; k++ ) { 				// Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
	Serial.println(F("Succesfully removed ID record from EEPROM"));
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != NULL ) 			// Make sure there is something in the array first
    match = true; 			// Assume they match at first
  for ( int k = 0; k < 7; k++ ) { 	// Loop 4 times
    if ( a[k] != b[k] ) 		// IF a != b then set match = false, one fails, all fail
      match = false;
  }
  return(match);
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
int findIDSLOT( byte find[] ) {
  int count = EEPROM.read(0); 			// Read the first Byte of EEPROM that
  for ( int i = 1; i <= count; i++ ) { 		// Loop once for each EEPROM entry
    readID(i); 								// Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) { 	// Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i; 				// The slot number of the card
      break; 					// Stop looking we found it
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
boolean findID( byte find[] ) {
  int count = EEPROM.read(0);			// Read the first Byte of EEPROM that
  for ( int i = 1; i <= count; i++ ) {  	// Loop once for each EEPROM entry
    readID(i); 					// Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {  	// Check to see if the storedCard read from EEPROM
      return true;
      break; 	// Stop looking we found it
    }
    else {  	// If not, return false
    }
  }
  return false;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() {
  digitalWrite(blueLed, LED_OFF); 	// Make sure blue LED is off
  digitalWrite(redLed, LED_OFF); 	// Make sure red LED is off

  for (int i = 1; i <=3; i++){
    digitalWrite(greenLed, LED_OFF); 	// Make sure green LED is on
    delay(200);
    digitalWrite(greenLed, LED_ON); 	// Make sure green LED is on
    delay(200);
  }
}

////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() {
  digitalWrite(blueLed, LED_OFF); 	// Make sure blue LED is off
  digitalWrite(greenLed, LED_OFF);   // Make sure green LED is off
  for(int i = 1; i <=3; i++){
    digitalWrite(redLed, LED_OFF); 	// Make sure red LED is off
    delay(200);
    digitalWrite(redLed, LED_ON); 	// Make sure red LED is on
    delay(200);
  }
}

///////////////////////////////// Success Remove UID From EEPROM  ////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() {
  digitalWrite(redLed, LED_OFF); 	// Make sure red LED is off
  digitalWrite(greenLed, LED_OFF); 	// Make sure green LED is off
  for(int i = 1; i <=3; i++){
    digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
    delay(200);
    digitalWrite(blueLed, LED_ON); 	// Make sure blue LED is on
    delay(200);
  }
}

/////////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
boolean isMaster( byte test[] ) {
  if ( checkTwo( test, masterCard ) ){
    Serial.println("Is master");
    return true;}
  else{
    Serial.println("Is not master");
    return false;}
}
