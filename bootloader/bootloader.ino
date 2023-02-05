/**
 * This sketch programs the bootloader EEPROM
 */
#define SHIFT_DATA  2
#define SHIFT_CLK   3
#define SHIFT_LATCH 4
#define EEPROM_D0   5
#define EEPROM_D7   12
#define WRITE_EN    13
#define EEPROM_SIZE 2048
#define RAM_SIZE    256


/*
 * Instructions defined in the microcode
 */
enum ins {
  BOOT,
  LDA,
  LDB,
  LDIA,
  LDIB,
  LDPA,
  LDPB,
  STA,
  STB,
  STPA,
  STPB,
  ADD,
  SUB,
  ADDI,
  SUBI,
  CMP,
  JMP,
  JMPC,
  JMPZ,
  OUTA,
  OUTB,
  HLT,
  NOP = 31,
};


const static uint8_t programs[EEPROM_SIZE / RAM_SIZE][RAM_SIZE] PROGMEM = {
  #define PRG0_NUMBER 9
  {
    /*
     * Add 1
     */
    LDA, PRG0_NUMBER,
    ADDI, 1,
    STA, PRG0_NUMBER,
    OUTA,
    JMP, 0, // jump to start
    0,  // PRG0_NUMBER
  },

  #define PRG1_ADDR_PTR 24
  #define PRG1_PRG_END  25
  {
    /*
     * Simple memtest
     * Check if expected value is in RAM location.
     * Checks locations after the actual program, halts on unexpected values.
     */
    OUTA,
    LDPA, PRG1_ADDR_PTR,
    SUB, PRG1_ADDR_PTR,
    JMPZ, 8, // skip HLT if RAM location has expected value (zero flag set)
      HLT,
    LDA, PRG1_ADDR_PTR,
    ADDI, 1,
    JMPC, 18, // jump to reset if PRG1_ADDR_PTR overflow (carry flag set)
      STA, PRG1_ADDR_PTR,
      JMP, 0, // jump to start
    LDIA, PRG1_PRG_END, // reset PRG1_ADDR_PTR to PRG1_PRG_END
    STA, PRG1_ADDR_PTR,
    JMP, 0, // jump to start

    // vars
    PRG1_PRG_END, // PRG1_ADDR_PTR
    PRG1_PRG_END, // PRG1_PRG_END
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
  },

  #define PRG2_ADDR_PTR   36
  #define PRG2_PATTERN    37
  #define PRG2_COMPLIMENT 38
  #define PRG2_PRG_LEN    39
  {
    /*
     * Memtest
     * For each RAM location after the program, check if pattern match and write the compliment. HLT if not match.
     * To keep the program as short as possible, we'll use the default init value as pattern, 0b00000000, with the compliment 0b11111111.
     */
    OUTA,
    LDPA, PRG2_ADDR_PTR,
    SUB, PRG2_PATTERN,
    JMPZ, 8, // skip HLT if memory match pattern
      HLT,
    LDA, PRG2_COMPLIMENT, // addr 8
    STPA, PRG2_ADDR_PTR,
    LDA, PRG2_ADDR_PTR,
    ADDI, 1,
    JMPC, 22, // end reached, goto reset
      STA, PRG2_ADDR_PTR,
      JMP, 0,
    LDIA, PRG2_PRG_LEN, // reset (addr 22): set PRG2_ADDR_PTR to PRG2_PRG_LEN, flip pattern and compliment, goto start
    STA, PRG2_ADDR_PTR,
    LDA, PRG2_PATTERN,
    LDB, PRG2_COMPLIMENT,
    STA, PRG2_COMPLIMENT,
    STB, PRG2_PATTERN,
    JMP, 0,
  
    // vars
    PRG2_PRG_LEN, // PRG2_ADDR_PTR
    0, // PRG2_PATTERN
    255, // PRG2_COMPLIMENT
  },
  {},
  {},
  {},
  {},
  { JMP, 0, },
};


/*
 * Output the address bits and outputEnable signal using shift registers.
 */
void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}


/*
 * Read a byte from the EEPROM at the specified address.
 */
byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }
  setAddress(address, /*outputEnable*/ true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}


/*
 * Write a byte to the EEPROM at the specified address.
 */
void writeEEPROM(int address, byte data) {
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}


/*
 * Read the contents of the EEPROM and print them to the serial monitor.
 */
void printContents(int start, int length) {
  for (int base = start; base < length; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    sprintf(buf, "%04x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}


void setup() {
  // put your setup code here, to run once:
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);
  Serial.begin(57600);

  // Program data bytes
  Serial.print("Programming EEPROM");

  // Write the programs into the EEPROM
  for (int address = 0; address < EEPROM_SIZE; address += 1) {
    int program = address >> 8;
    int instruction = address & 0b11111111;

    writeEEPROM(address, pgm_read_byte(&programs[program][instruction]));

    if (address % 64 == 0) {
      Serial.print(".");
    }
  }

  Serial.println(" done");

  // Read and print out the contents of the EERPROM
  Serial.println("Reading EEPROM");
  printContents(0, EEPROM_SIZE);
  Serial.println("Write and read bootloader done");
}


void loop() {
}
