/**
 * This sketch programs the bootloader EEPROM for the 8-bit breadboard computer
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
  LDS,
  LDIA,
  LDIB,
  LDIS,
  LDPA,
  LDPB,
  STA,
  STB,
  STS,
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
  JMPNC,
  JMPNZ,
  OUTA,
  OUTB,
  OUTI,
  SET,
  UNSET,
  HLT,
  NOP = 31,
};


const static uint8_t programs[EEPROM_SIZE / RAM_SIZE][RAM_SIZE] PROGMEM = {
  {
    /*
     * Add
     */
    OUTA,
    ADDI, 1,
    JMP, 0,
  },

  {
    /*
     * Add - two's complement
     */
    SET, 0b0001,
    OUTA,
    ADDI, 1,
    JMP, 2,
  },

  {
    /*
     * Wave
     */
    OUTA,
    ADDI, 1,
    JMPC, 8,
    JMP, 0,
    OUTA,
    SUBI, 1,
    JMPZ, 0,
    JMP, 7,
  },

  #define PRG1_ADDR_PTR   36
  #define PRG1_PATTERN    37
  #define PRG1_COMPLEMENT 38
  #define PRG1_PRG_LEN    39
  {
    /*
     * Memtest
     * For each RAM location after the program, check if pattern match and write the complement.
     * HLT if not match.
     * To keep the program as short as possible,
     * we'll use the default init value as pattern, 0b00000000, with the complement 0b11111111.
     */
    OUTA,
    LDPA, PRG1_ADDR_PTR,
    SUB, PRG1_PATTERN,
    JMPZ, 8, // skip HLT if memory match pattern
      HLT,
    LDA, PRG1_COMPLEMENT,
    STPA, PRG1_ADDR_PTR,
    LDA, PRG1_ADDR_PTR,
    ADDI, 1,
    JMPC, 22, // end reached, goto reset
      STA, PRG1_ADDR_PTR,
      JMP, 0,
    LDA, PRG1_PATTERN, // reset: flip pattern and complement
    LDB, PRG1_COMPLEMENT,
    STA, PRG1_COMPLEMENT,
    STB, PRG1_PATTERN,
    LDIA, PRG1_PRG_LEN, // set PRG1_ADDR_PTR to PRG1_PRG_LEN
    STA, PRG1_ADDR_PTR,
    JMP, 0, // goto start
  
    // vars
    PRG1_PRG_LEN, // PRG1_ADDR_PTR
    0,            // PRG1_PATTERN
    255,          // PRG1_COMPLEMENT
  },

  #define PRG2_TMP 21
  #define PRG2_OLD 22
  {
    /*
     * Fibonacci sequence
     */
    OUTA,
    STA, PRG2_TMP,
    ADD, PRG2_OLD,
    LDB, PRG2_TMP,
    STB, PRG2_OLD,
    JMPC, 13, // overflow, reset sequence
      JMP, 0,
    OUTI, 0,
    LDIA, 1,
    STA, PRG2_OLD,
    JMP, 0,
    0, // PRG2_TMP
    1, // PRG2_OLD
  },

  {
    /*
     * Symmetric Pattern
     */
    LDIA, 1,   LDIB, 1,   OUTI, 1,
    LDIA, 2,   LDIB, 2,   OUTI, 2,
    LDIA, 4,   LDIB, 4,   OUTI, 4,
    LDIA, 8,   LDIB, 8,   OUTI, 8,
    LDIA, 16,  LDIB, 16,  OUTI, 16,
    LDIA, 32,  LDIB, 32,  OUTI, 32,
    LDIA, 64,  LDIB, 64,  OUTI, 64,
    LDIA, 128, LDIB, 128, OUTI, 128,
    LDIA, 128, LDIB, 128, OUTI, 128,
    LDIA, 64,  LDIB, 64,  OUTI, 64,
    LDIA, 32,  LDIB, 32,  OUTI, 32,
    LDIA, 16,  LDIB, 16,  OUTI, 16,
    LDIA, 8,   LDIB, 8,   OUTI, 8,
    LDIA, 4,   LDIB, 4,   OUTI, 4,
    LDIA, 2,   LDIB, 2,   OUTI, 2,
    LDIA, 1,   LDIB, 1,   OUTI, 1,
    JMP, 0,   
  },
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
  delay(5);
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


/*
 * Verify the contents of the EEPROM
 */
bool verifyContents(int start, int length) {
  for (int address = start; address < length; address++) {
    int program = address >> 8;
    int instruction = address & 0b11111111;

    uint8_t program_byte = pgm_read_byte(&programs[program][instruction]);
    uint8_t eeprom_byte = readEEPROM(address);

    if (program_byte != eeprom_byte) {
      return false;
    }
  }

  return true;
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
  Serial.print("Programming bootloader EEPROM");

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

  // Verify the contents of the EEPROM
  Serial.print("Verifying bootloader EEPROM content...");
  if (verifyContents(0, EEPROM_SIZE)) {
    Serial.println(" ok");
  } else {
    Serial.println(" error");
  }

  // Read and print out the contents of the EERPROM
  // Serial.println("Reading EEPROM");
  // printContents(0, EEPROM_SIZE);
}


void loop() {
}
