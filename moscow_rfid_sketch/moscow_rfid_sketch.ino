#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN             9  // Configurable, see typical pin layout above
#define SS_PIN             10  // Configurable, see typical pin layout above
#define OTP_PAGE            3
#define OTP_INDEX  OTP_PAGE*4

#define DATA_PIN    8 
#define LATCH_PIN   7 
#define CLOCK_PIN   6 

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

typedef enum STATE_T {
  ST_IDLE,
  ST_READ,
  ST_SHOW_RIDES,
  ST_SHOW_DUEDATE
} State;

typedef enum FORMAT_T {
  F_UNKNOWN = -1,
  F_OLD_UNITY,  // Единый
  F_2016_09_UNITY,
  F_2016_09_TAT,
  F_LAST           // for iterating
} Format;


byte OTP_LIST[] = {
    0xFF, 0xFF, 0xFF, 0xFC,  // F_OLD_UNITY
    0x00, 0x7F, 0xFF, 0xFC,  // F_2016_09_UNITY
    0x00, 0x00, 0x00, 0x00   // F_2016_09_TAT
};

// in order: A, B, C, D, E, F, G, H
// H is reserved for floating point
// in pin notation:
// A     - 11
// B     -  7
// C     -  4
// D     -  2
// E     -  1
// F     - 10
// G     -  5
// H(DP) -  3 

byte SEGMENTS[10] = {
    0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
    0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110
};

/* Formats:
 *    F_OLD_UNITY: (http://soltau.ru/index.php/arduino/item/402-kak-prochitat-bilet-na-metro-i-avtobus-s-pomoshchyu-arduino)
 *        p0b[0-2],p1b[0-3] - UID
 *        p2b[2-3] - Lock Bytes
 *        p3 - OTP
 *        p4b[2.5-3]-p5b[0-2.5] - ticket number
 *        
 *        p8b[0-1] - date of purchace since 1992-01-01
 *        p8b2 - validity duration in days
 *        p9b1 - Number of rides left
 *        p10 - checksum
 *        p11b0,p11b1 - date of last use
 *        
 *        p12-p15 are a copy of p8-p11
 *        
 *    F_2016_09_UNITY:
 *        p8b0 - Number of rides left
 */

State state = ST_IDLE;
Format format = F_OLD_UNITY;
byte num_rides = 0;


inline byte to_offset(byte read_page, byte target_page, byte offset) {
    return 4 * (target_page - read_page) + offset;
}

inline byte get_byte(byte* buffer, byte read_page, byte target_page, byte offset) {
    byte i = to_offset(read_page, target_page, offset);
    return buffer[i];
}


bool is_page_empty(byte* buffer, byte read_page, byte target_page) {
    for (byte offset = 0; offset < 4; offset++) {
        byte data = get_byte(buffer, read_page, target_page, offset); 
        if (data != 0) {
            return false; 
        }
    }
    return true;
}


void print_array(byte* arr, byte size) {
    for (byte i = 0; i < size; i++) {
        Serial.print(arr[i], HEX);
        Serial.print(" "); 
        if ((i + 1) % 4 == 0) {
          Serial.println();
        }
    }
    Serial.println();
}


Format get_format(byte* buffer) {    
    for (byte format=0; format < F_LAST; format++) {
        byte offset = format * 4;
              
        if (memcmp(buffer+OTP_INDEX, OTP_LIST+offset, sizeof(byte)*4) == 0) {
            return format;
        }
    }
    return F_UNKNOWN; // format unknown  
}


void send_to_register(int data) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, data);
    digitalWrite(LATCH_PIN, HIGH);
}


void setup() {
    Serial.begin(9600);   // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();      // Init SPI bus
    mfrc522.PCD_Init();   // Init MFRC522
    mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD -м MFRC522 Card Reader details    

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
}


State do_idle() {
    // Look for new cards
    if ( !mfrc522.PICC_IsNewCardPresent()) {
        return ST_IDLE;
    }
  
    // Select one of the cards
    if ( !mfrc522.PICC_ReadCardSerial()) {
        return ST_IDLE;
    }
    
    return ST_READ;
}


State do_read() { 
    MFRC522::StatusCode status;
    byte byteCount;
    byte buffer[18];
    byte i;
  
    byte read_page = 8;

    num_rides = 0;
    
    byteCount = sizeof(buffer);
    
    // we need OTP from 3rd page, hence we read 4 first pages starting from 0
    status = mfrc522.MIFARE_Read(0, buffer, &byteCount);  
    if (status != MFRC522::StatusCode::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return ST_IDLE;    
    }

    // Get card type (defined by data layout/format)
    format = get_format(buffer);
    
        
    // fill buffer with data from read_page
    status = mfrc522.MIFARE_Read(read_page, buffer, &byteCount);
    if (status != MFRC522::StatusCode::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return ST_IDLE;    
    }

    switch (format) {
        case F_OLD_UNITY:
            num_rides = get_byte(buffer, read_page, 9, 1);
            Serial.println("F_OLD_UNITY");
            break;
        case F_2016_09_UNITY:
            num_rides = get_byte(buffer, read_page, 8, 0);
            Serial.println("F_2016_09_UNITY");
            break;
        case F_2016_09_TAT:
            num_rides = get_byte(buffer, read_page, 9, 1);    
            Serial.println("F_2016_09_TAT");
            break;
        case F_UNKNOWN:
        default:
            Serial.println("UNKNOWN");
            break;        
    }  
    
    return ST_SHOW_RIDES;  // FIXME
}



State do_show_rides() {
    Serial.print("NUM OF RIDES: ");
    Serial.println(num_rides);

    Serial.println("#####");
    
    // mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
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
        case ST_SHOW_DUEDATE:
        default:
            state = ST_IDLE;
            break;
    }
}
