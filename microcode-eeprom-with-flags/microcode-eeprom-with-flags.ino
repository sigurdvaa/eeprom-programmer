/**
 * This sketch programs the microcode EEPROMs for the 8-bit breadboard computer
 * It includes support for a flags register with carry and zero flags
 * See this video for more: https://youtu.be/Zg1NdPKoosU
 */
#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

#define HLT 0b100000000000000000000000  // Halt clock
#define MI  0b010000000000000000000000  // Memory address register in
#define RI  0b001000000000000000000000  // RAM data in
#define RO  0b000100000000000000000000  // RAM data out
#define TR  0b000010000000000000000000  // T-state reset
#define II  0b000001000000000000000000  // Instruction register in
#define AI  0b000000100000000000000000  // A register in
#define AO  0b000000010000000000000000  // A register out
#define EO  0b000000001000000000000000  // ALU out
#define SU  0b000000000100000000000000  // ALU subtract
#define BI  0b000000000010000000000000  // B register in
#define OI  0b000000000001000000000000  // Output register in
#define CE  0b000000000000100000000000  // Program counter enable
#define CO  0b000000000000010000000000  // Program counter out
#define J   0b000000000000001000000000  // Jump (program counter in)
#define FI  0b000000000000000100000000  // Flags in

#define FLAGS_Z0C0 0
#define FLAGS_Z0C1 1
#define FLAGS_Z1C0 2
#define FLAGS_Z1C1 3

#define JC  0b0111
#define JZ  0b1000

uint16_t UCODE_TEMPLATE[16][8] = {
  { CO|MI,  RO|II|CE,  TR,     0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00000 - NOP
  { CO|MI,  RO|II|CE,  CO|MI,  RO|MI|CE,  RO|AI,  TR,           0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00001 - LDA
  { CO|MI,  RO|II|CE,  CO|MI,  RO|MI|CE,  RO|BI,  EO|AI|FI,     TR, 0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00010 - ADD
  { CO|MI,  RO|II|CE,  CO|MI,  RO|MI|CE,  RO|BI,  EO|AI|SU|FI,  TR, 0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00011 - SUB
  { CO|MI,  RO|II|CE,  CO|MI,  RO|MI|CE,  AO|RI,  TR,           0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00100 - STA
  { CO|MI,  RO|II|CE,  CO|MI,  RO|AI|CE,  TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00101 - LDI
  { CO|MI,  RO|II|CE,  CO|MI,  RO|J,      TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00110 - JMP
  { CO|MI,  RO|II|CE,  CO|MI,  RO|JC|CE,  TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00111 - JC
  { CO|MI,  RO|II|CE,  CO|MI,  RO|JZ|CE,  TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01000 - JZ
  { CO|MI,  RO|II|CE,  TR,     0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01001 - NOP
  { CO|MI,  RO|II|CE,  TR,     0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01010 - NOP
  { CO|MI,  RO|II|CE,  TR,     0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01011 - NOP
  { CO|MI,  RO|II|CE,  TR,     0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01100 - NOP
  { CO|MI,  RO|II|CE,  TR,     0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01101 - NOP
  { CO|MI,  RO|II|CE,  AO|OI,  TR,        0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01110 - OUT
  { CO|MI,  RO|II|CE,  HLT,    0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01111 - HLT
};

uint16_t ucode[4][16][8];

void initUCode() {
  // ZF = 0, CF = 0
  memcpy(ucode[FLAGS_Z0C0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));

  // ZF = 0, CF = 1
  memcpy(ucode[FLAGS_Z0C1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z0C1][JC][2] = IO|J;

  // ZF = 1, CF = 0
  memcpy(ucode[FLAGS_Z1C0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z1C0][JZ][2] = IO|J;

  // ZF = 1, CF = 1
  memcpy(ucode[FLAGS_Z1C1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z1C1][JC][2] = IO|J;
  ucode[FLAGS_Z1C1][JZ][2] = IO|J;
}

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
    sprintf(buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}


void setup() {
  // put your setup code here, to run once:
  initUCode();

  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);
  Serial.begin(57600);

  // Program data bytes
  Serial.print("Programming EEPROM");

  // Program the 8 high-order bits of microcode into the first 128 bytes of EEPROM
  for (int address = 0; address < 1024; address += 1) {
    int flags       = (address & 0b1100000000) >> 8;
    int byte_sel    = (address & 0b0010000000) >> 7;
    int instruction = (address & 0b0001111000) >> 3;
    int step        = (address & 0b0000000111);

    if (byte_sel) {
      writeEEPROM(address, ucode[flags][instruction][step]);
    } else {
      writeEEPROM(address, ucode[flags][instruction][step] >> 8);
    }

    if (address % 64 == 0) {
      Serial.print(".");
    }
  }

  Serial.println(" done");


  // Read and print out the contents of the EERPROM
  Serial.println("Reading EEPROM");
  printContents(0, 1024);
}


void loop() {
  // put your main code here, to run repeatedly:

}

