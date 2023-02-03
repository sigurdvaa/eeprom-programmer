/**
 * This sketch programs the microcode EEPROMs for the 8-bit breadboard computer
 * It includes support for a flags register with carry and zero flags
 * See this video for more: https://youtu.be/Zg1NdPKoosU
 */
#define SHIFT_DATA  2
#define SHIFT_CLK   3
#define SHIFT_LATCH 4
#define EEPROM_D0   5
#define EEPROM_D7   12
#define WRITE_EN    13
#define EEPROM_SIZE 2048
#define RAM_SIZE    256

#define HLT 0b10000000000000000000000000000000  // Halt clock
#define MI  0b01000000000000000000000000000000  // Memory address register in
#define RI  0b00100000000000000000000000000000  // RAM data in
#define RO  0b00010000000000000000000000000000  // RAM data out
#define TR  0b00001000000000000000000000000000  // T-state reset
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


const static uint32_t GET_PC = CO|MI;
const static uint32_t GET_INS = RO|II|CE;

const static uint32_t ucode[32][16] PROGMEM = {
  { CO|LI|MI|OI, LO|RI, CO|II, CE|TR,                                                }, // 00000 - BOOT
  { GET_PC, GET_INS, GET_PC, RO|MI, RO|AI|CE|TR,                                   }, // 00001 - LDA
  { GET_PC, GET_INS, GET_PC, RO|MI, RO|BI|CE|TR,                                   }, // 00010 - LDB
  { GET_PC, GET_INS, GET_PC, RO|AI|CE|TR,                                          }, // 00011 - LDIA
  { GET_PC, GET_INS, GET_PC, RO|BI|CE|TR,                                          }, // 00100 - LDIB
  { GET_PC, GET_INS, GET_PC, RO|MI, RO|MI, RO|AI|CE|TR,                            }, // 00101 - LDPA
  { GET_PC, GET_INS, GET_PC, RO|MI, RO|MI, RO|BI|CE|TR,                            }, // 00110 - LDPB
  { GET_PC, GET_INS, GET_PC, RO|MI, AO|RI|CE|TR,                                   }, // 00111 - STA
  { GET_PC, GET_INS, GET_PC, RO|MI, BO|RI|CE|TR,                                   }, // 01000 - STB
  { GET_PC, GET_INS, GET_PC, RO|MI, RO|BI, EO|AI|FI|CE|TR,                         }, // 01001 - ADD
  { GET_PC, GET_INS, GET_PC, RO|MI, RO|BI, EO|AI|FI|CE|TR|SU,                      }, // 01010 - SUB
  { GET_PC, GET_INS, GET_PC, RO|BI, EO|AI|FI|CE|TR,                                }, // 01011 - ADDI
  { GET_PC, GET_INS, GET_PC, RO|BI, EO|AI|SU|FI|CE|TR,                             }, // 01100 - SUBI
  { GET_PC, GET_INS, GET_PC, RO|MI, RO|AI|CE, GET_PC, RO|MI, RO|BI, SU|FI|CE|TR,   }, // 01101 - CMP
  { GET_PC, GET_INS, GET_PC, RO|J|TR,                                              }, // 01110 - JMP
  { GET_PC, GET_INS, GET_PC, RO|JC|CE|TR,                                          }, // 01111 - JMPC
  { GET_PC, GET_INS, GET_PC, RO|JZ|CE|TR,                                          }, // 10000 - JMPZ
  { GET_PC, GET_INS, AO|OI|TR,                                                     }, // 10001 - OUTA
  { GET_PC, GET_INS, BO|OI|TR,                                                     }, // 10010 - OUTB
  { GET_PC, GET_INS, HLT,                                                          }, // 10011 - HLT
  { GET_PC, GET_INS, TR,                                                           }, // 10100 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 10101 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 10110 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 10111 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 11000 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 11001 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 11010 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 11011 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 11100 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 11101 - NOP
  { GET_PC, GET_INS, TR,                                                           }, // 11110 - NOP
  { GET_PC, GET_INS, TR, CE|TR,                                                    }, // 11111 - RUN
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

  // Read and print out the contents of the EERPROM
  Serial.println("Reading EEPROM");
  printContents(0, EEPROM_SIZE);
}


void loop() {
}

