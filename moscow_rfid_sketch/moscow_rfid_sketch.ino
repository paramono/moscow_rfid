#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

typedef enum STATE_T {
  ST_IDLE,
  ST_READ,
  ST_SHOW_RIDES,
  ST_SHOW_DUEDATE
} State;


State state = ST_IDLE;
byte num_rides;


inline byte to_offset(byte read_page, byte target_page, byte offset) {
    return 4 * (target_page - read_page) + offset;
}

inline byte get_byte(byte* buffer, byte read_page, byte target_page, byte offset) {
    byte i = to_offset(read_page, target_page, offset);
    return buffer[i];
}

void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}


State do_idle() {
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return ST_IDLE;
    }
  
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
        return ST_IDLE;
    }
    
    return ST_READ;
}


State do_read() {
    // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    MFRC522::StatusCode status;
    byte byteCount;
    byte buffer[18];
    byte i;
  
    byte page = 8;
    bool is_metro = false;
  
    num_rides = 0;
    
    byteCount = sizeof(buffer);
    // fill buffer with data
    status = mfrc522.MIFARE_Read(page, buffer, &byteCount);
    if (status != MFRC522::StatusCode::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return ST_IDLE;    
    }
  
    // ride number is either p9b1 (land transport) or p8b0 (underground)
    // use p8b0 if card has all zeros on page 9 (and page 14)
    // use p9b1 if card has all zeros of page 8 (and page 15 probably)
  
    byte p9b1 = get_byte(buffer, page, 9, 1);
    if (p9b1 == 0) {

        is_metro = true;
    }
    Serial.print("p9b1: ");
    Serial.println(p9b1);
        
    if (is_metro) {
        num_rides = get_byte(buffer, page, 8, 0);    
        Serial.println(F("Metro"));
    } else {
        num_rides = p9b1;
        Serial.println(F("Bus"));
    }
  
 
    
    return ST_SHOW_RIDES;  // FIXME
  
//  byte offset = 1;  // page 9
//  for (byte index = 0; index < 4; index++) { 
//    i = 4 * offset + index;
//    if (buffer[i] !=
//  }
//  
//
//  for (byte offset = 0; offset < 4; offset++) { //vertical
//      i = page + offset;
//      for (byte index = 0; index < 4; index++) {
//          i = 4 * offset + index;
//          if(buffer[i] < 0x10)
//              Serial.print(F(" 0"));
//          else
//              Serial.print(F(" "));
//          Serial.print(buffer[i], HEX);
//      }
//      Serial.println();
//    }
//  
//  
//  Serial.println(F("Page  0  1  2  3"));
//  // Try the mpages of the original Ultralight. Ultralight C has more pages.
//  for (byte page = 0; page < 16; page +=4) { // Read returns data for 4 pages at a time.
//    
//    
//  }
//  
    return ST_IDLE;
}



State do_show_rides() {
    Serial.print("NUM OF RIDES: ");
    Serial.println(num_rides);

}


void loop() {

    switch(state) {
        case ST_IDLE: 
            state = do_idle();
            break;
        case ST_READ:
            state = do_read();
            break;
        case ST_SHOW_RIDES:
            state = do_show_rides();
            break;
        default:
            break;

    }
}
