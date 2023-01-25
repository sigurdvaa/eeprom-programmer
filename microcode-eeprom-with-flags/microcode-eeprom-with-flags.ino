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


const uint32_t FETCH_PC = CO|MI;
const uint32_t FETCH_INS = RO|II|CE;
const uint32_t FETCH_ADDR = CO|MI;

const static uint32_t ucode[32][16] PROGMEM = {
  { CO|LI|MI|II,      LO|RI|CE,          TR,         0,      0,   0,           0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00000 - BOOT
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|MI|CE,  RO|AI,  TR,           0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00001 - LDA
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|MI|CE,  RO|BI,  EO|AI|FI,     TR, 0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00010 - ADD
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|MI|CE,  RO|BI,  EO|AI|SU|FI,  TR, 0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00011 - SUB
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|MI|CE,  AO|RI,  TR,           0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00100 - STA
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|AI|CE,  TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00101 - LDI
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|J,      TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00110 - JMP
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|JC|CE,  TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 00111 - JC
  {    FETCH_PC,     FETCH_INS,  FETCH_ADDR,  RO|JZ|CE,  TR,     0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01000 - JZ
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01001 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01010 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01011 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01100 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01101 - (NOP)
  {    FETCH_PC,     FETCH_INS,  AO|OI,       TR,        0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01110 - OUT
  {    FETCH_PC,     FETCH_INS,  HLT,         0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 01111 - HLT

  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10000 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10001 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10010 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10011 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10100 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10101 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10110 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 10111 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11000 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11001 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11010 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11011 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11100 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11101 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11110 - (NOP)
  {    FETCH_PC,     FETCH_INS,  TR,          0,         0,      0,            0,  0, 0, 0, 0, 0, 0, 0, 0, 0 },   // 11111 - (NOP)
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

  // Program the 8 high-order bits of microcode into the first 2048 bytes of EEPROM
  for (int address = 0; address < 2048; address += 1) {
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
  printContents(0, 2048);
}


void loop() {
  // put your main code here, to run repeatedly:

}

