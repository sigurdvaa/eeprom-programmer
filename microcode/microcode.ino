/**
 * This sketch programs the microcode EEPROMs for the 8-bit breadboard computer
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
 * Control signals
 */
#define HLT 0b10000000000000000000000000000000  // Halt clock
#define MI  0b01000000000000000000000000000000  // Memory address register in
#define RI  0b00100000000000000000000000000000  // RAM data in
#define RO  0b00010000000000000000000000000000  // RAM data out
#define TR  0b00001000000000000000000000000000  // T-state reset, triggered on clock low
#define II  0b00000100000000000000000000000000  // Instruction register in
#define AI  0b00000010000000000000000000000000  // A register in
#define AO  0b00000001000000000000000000000000  // A register out
#define EO  0b00000000100000000000000000000000  // ALU out
#define SU  0b00000000010000000000000000000000  // ALU subtract
#define BI  0b00000000001000000000000000000000  // B register in
#define OI  0b00000000000100000000000000000000  // Output register in
#define CE  0b00000000000010000000000000000000  // Program counter enable
#define CO  0b00000000000001000000000000000000  // Program counter out
#define J   0b00000000000000100000000000000000  // Jump (program counter in)
#define FI  0b00000000000000010000000000000000  // Flags in
#define JC  0b00000000000000001000000000000000  // JC
#define JZ  0b00000000000000000100000000000000  // JZ
#define BO  0b00000000000000000010000000000000  // B register out
#define LI  0b00000000000000000001000000000000  // Bootloader address register in
#define LO  0b00000000000000000000100000000000  // Bootloader data out
#define SO  0b00000000000000000000010000000000  // Settings register out
#define SI  0b00000000000000000000001000000000  // Settings register in


/*
 * Define instruction microsteps
 * Most control signals are triggered on clock rising edge.
 * Exceptions are II, TR, and microstep counter, which trigger on inverted clock.
 */
const static uint32_t ucode[32][16] PROGMEM = {
  { CO|LI|MI|OI, LO|RI, CO|II, CE|TR,                                          }, // 00000 - BOOT
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|AI|TR,                                }, // 00001 - LDA
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|BI|TR,                                }, // 00010 - LDB
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|SI|TR,                                }, // 00011 - LDS
  { CO|MI, RO|II|CE, CO|MI, RO|AI|CE|TR,                                       }, // 00100 - LDIA
  { CO|MI, RO|II|CE, CO|MI, RO|BI|CE|TR,                                       }, // 00101 - LDIB
  { CO|MI, RO|II|CE, CO|MI, RO|SI|CE|TR,                                       }, // 00110 - LDIS
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|MI, RO|AI|TR,                         }, // 00111 - LDPA
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|MI, RO|BI|TR,                         }, // 01000 - LDPB
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, AO|RI|TR,                                }, // 01001 - STA
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, BO|RI|TR,                                }, // 01010 - STB
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, SO|RI|TR,                                }, // 01011 - STS
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|MI, AO|RI|TR,                         }, // 01100 - STPA
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|MI, BO|RI|TR,                         }, // 01101 - STPB
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|BI, EO|AI|FI|TR,                      }, // 01110 - ADD
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|BI, EO|AI|FI|TR|SU,                   }, // 01111 - SUB
  { CO|MI, RO|II|CE, CO|MI, RO|BI|CE, EO|AI|FI|TR,                             }, // 10000 - ADDI
  { CO|MI, RO|II|CE, CO|MI, RO|BI|CE, EO|AI|FI|TR|SU,                          }, // 10001 - SUBI
  { CO|MI, RO|II|CE, CO|MI, RO|MI|CE, RO|AI, CO|MI, RO|MI|CE, RO|BI, SU|FI|TR, }, // 10010 - CMP
  { CO|MI, RO|II|CE, CO|MI, RO|J|TR,                                           }, // 10011 - JMP
  { CO|MI, RO|II|CE, CO|MI, RO|JC|CE|TR,                                       }, // 10100 - JMPC
  { CO|MI, RO|II|CE, CO|MI, RO|JZ|CE|TR,                                       }, // 10101 - JMPZ
  { CO|MI, RO|II|CE, CO|MI, CE, CO|BI, RO|J, BO|JC|TR,                         }, // 10110 - JMPNC
  { CO|MI, RO|II|CE, CO|MI, CE, CO|BI, RO|J, BO|JZ|TR,                         }, // 10111 - JMPNZ
  { CO|MI, RO|II|CE, AO|OI|TR,                                                 }, // 11000 - OUTA
  { CO|MI, RO|II|CE, BO|OI|TR,                                                 }, // 11001 - OUTB
  { CO|MI, RO|II|CE, CO|MI, RO|OI|CE|TR,                                       }, // 11010 - OUTI
  { CO|MI, RO|II|CE, SO|AI, CO|MI, RO|BI|CE, EO|SI|TR,                         }, // 11011 - SET
  { CO|MI, RO|II|CE, SO|AI, CO|MI, RO|BI|CE, EO|SI|TR|SU,                      }, // 11100 - UNSET
  { CO|MI, RO|II|CE, HLT,                                                      }, // 11101 - HLT
  { CO|MI, RO|II|CE, TR,                                                       }, // 11110 - NOP
  { CO|MI, RO|II|CE, TR, CE|TR,                                                }, // 11111 - NOP, BOOT end
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

    int bit_select_3 = (address & 0b10000000000) >> 10;
    int bit_select_2 = (address & 0b01000000000) >> 9;
    int instruction  = (address & 0b00111110000) >> 4;
    int step         = (address & 0b00000001111);

    uint8_t ucode_byte;

    if (bit_select_3) {
      ucode_byte = pgm_read_dword(&ucode[instruction][step]) >> 8;
    } else if (bit_select_2) {
      ucode_byte = pgm_read_dword(&ucode[instruction][step]) >> 16;
    } else {
      ucode_byte = pgm_read_dword(&ucode[instruction][step]) >> 24;
    }

    uint8_t eeprom_byte = readEEPROM(address);

    if (ucode_byte != eeprom_byte) {
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
  Serial.print("Programming microcode EEPROM");

  // Program the 8 high-order bits of microcode into the EEPROM
  for (int address = 0; address < EEPROM_SIZE; address += 1) {
    int bit_select_3 = (address & 0b10000000000) >> 10;
    int bit_select_2 = (address & 0b01000000000) >> 9;
    int instruction  = (address & 0b00111110000) >> 4;
    int step         = (address & 0b00000001111);

    if (bit_select_3) {
      writeEEPROM(address, pgm_read_dword(&ucode[instruction][step]) >> 8);
    } else if (bit_select_2) {
      writeEEPROM(address, pgm_read_dword(&ucode[instruction][step]) >> 16);
    } else {
      writeEEPROM(address, pgm_read_dword(&ucode[instruction][step]) >> 24);
    }

    if (address % 64 == 0) {
      Serial.print(".");
    }
  }

  Serial.println(" done");

  // Verify the contents of the EEPROM
  Serial.print("Verifying microcode EEPROM content...");
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

