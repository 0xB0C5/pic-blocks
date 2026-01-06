#define PIN_POWER 2
#define PIN_MCLR 3
#define PIN_CLOCK 4
#define PIN_DATA 5

#define DEVICE_ID_ADDRESS 0x8006
#define ROW_SIZE_WORDS 32

const uint8_t unlock_code[4] = {'M','C','H','P'};

uint16_t row_buffer[ROW_SIZE_WORDS];

void write_byte(uint8_t b) {
  for (int i = 7; i >= 0; i--) {
    digitalWrite(PIN_DATA, (b >> i) & 1);
    digitalWrite(PIN_CLOCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_CLOCK, LOW);
    delayMicroseconds(1);
  }
}

void write_payload(uint32_t payload) {
  // Shift to add end bit.
  payload <<= 1;
  for (int i = 23; i >= 0; i--) {
    digitalWrite(PIN_DATA, (payload >> i) & 1);
    digitalWrite(PIN_CLOCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_CLOCK, LOW);
    delayMicroseconds(1);
  }
}

uint32_t read_payload() {
  uint32_t payload = 0;
  pinMode(PIN_DATA, INPUT);
  for (int i = 23; i >= 0; i--) {
    digitalWrite(PIN_CLOCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_CLOCK, LOW);
    payload |= ((uint32_t)digitalRead(PIN_DATA)) << i;
    delayMicroseconds(1);
  }
  pinMode(PIN_DATA, OUTPUT);
  digitalWrite(PIN_DATA, LOW);

  // Shift to remove end bit.
  payload >>= 1;

  return payload;
}

void set_address(uint16_t address) {
  write_byte(0x80);
  write_payload(address);
  delayMicroseconds(1);
}

uint16_t read_and_increment() {
  write_byte(0xfe);
  uint16_t result = (uint16_t)read_payload();
  delayMicroseconds(1);
  return result;
}

void chip_erase() {
  set_address(0);
  write_byte(0x18);
  delay(10);
}

void write_row(uint16_t address) {
  set_address(address);
  for (int i = 0; i < ROW_SIZE_WORDS; i++) {
    write_byte(0x02); // Write and increment.
    write_payload(row_buffer[i]);
  }
  set_address(address);
  write_byte(0xe0);
  delay(10);
}

void write_config_word(uint16_t address, uint16_t w) {
  set_address(address);
  write_byte(0x00); // Write, no increment.
  write_payload(w);
  write_byte(0xe0);
  delay(10);
}

void read_row(uint16_t address) {
  set_address(address);
  for (int i = 0; i < ROW_SIZE_WORDS; i++) {
    row_buffer[i] = read_and_increment();
  }
}

void unlock() {
  // Write unlock sequence.
  for (int i = 0; i < sizeof(unlock_code); i++) {
    uint8_t b = unlock_code[i];
    write_byte(b);
  }
}

void setup() {
  pinMode(PIN_POWER, OUTPUT);
  pinMode(PIN_MCLR, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);

  digitalWrite(PIN_POWER, LOW);
  digitalWrite(PIN_MCLR, HIGH);
  digitalWrite(PIN_CLOCK, LOW);
  digitalWrite(PIN_DATA, LOW);

  Serial.begin(115200);

  delay(500);

  // Enter program mode.
  digitalWrite(PIN_POWER, HIGH);
  digitalWrite(PIN_MCLR, LOW);

  delay(500);

  unlock();
}

void cmd_erase() {
  chip_erase();
  Serial.write("e\n");
}

uint8_t serial_read_nibble() {
  while (!Serial.available());

  uint8_t c = Serial.read();

  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('A' <= c && c <= 'Z') {
    return 10 + (c - 'A');
  } else if ('a' <= c && c <= 'z') {
    return 10 + (c - 'a');
  }
}

uint16_t serial_read_word() {
  // This is weird because it's most significant nibble first,
  // but least significant byte first.
  uint16_t w = serial_read_nibble() << 4;
  w |= serial_read_nibble();
  
  w |= serial_read_nibble() << 12;
  w |= serial_read_nibble() << 8;

  return w;
}

void serial_write_nibble(uint8_t nibble) {
  if (nibble < 10) {
    Serial.write('0' + nibble);
  } else {
    Serial.write('A' + (nibble - 10));
  }
}

void serial_write_word(uint16_t w) {
  // This is weird because it's most significant nibble first,
  // but least significant byte first.
  serial_write_nibble((w >> 4) & 0xf);
  serial_write_nibble(w & 0xf);

  serial_write_nibble(w >> 12);
  serial_write_nibble((w >> 8) & 0xf);
}

void cmd_write() {
  uint16_t addr = serial_read_word();
  for (int i = 0; i < ROW_SIZE_WORDS; i++) {
    row_buffer[i] = serial_read_word();
  }

  write_row(addr);
  Serial.write("w\n");
  Serial.flush();
}

void cmd_write_config_word() {
  uint16_t addr = serial_read_word();
  uint16_t w = serial_read_word();
  write_config_word(addr, w);


  Serial.write("c\n");
}

void cmd_read() {
  uint16_t addr = serial_read_word();
  read_row(addr);

  Serial.write('r');

  for (int i = 0; i < ROW_SIZE_WORDS; i++) {
    serial_write_word(row_buffer[i]);
  }

  Serial.write('\n');
  Serial.flush();
}

void cmd_hello() {
  Serial.write("h\n");
  Serial.flush();
}

void cmd_dump() {
  Serial.write('d');
  for (int i = 0; i < ROW_SIZE_WORDS; i++) {
    serial_write_word(row_buffer[i]);
  }
  Serial.write('\n');
  Serial.flush();
}

void cmd_stop() {
  digitalWrite(PIN_POWER, LOW);
  digitalWrite(PIN_MCLR, HIGH);

  delay(500);

  digitalWrite(PIN_POWER, HIGH);
  pinMode(PIN_CLOCK, INPUT);
  pinMode(PIN_DATA, INPUT);
  
  Serial.write(".\n");
  Serial.flush();
}

void loop() {
  if (Serial.available()) {
    uint8_t b = Serial.read();
    switch (b) {
      case 'r':
        cmd_read();
        break;
      case 'w':
        cmd_write();
        break;
      case 'c':
        cmd_write_config_word();
        break;
      case 'e':
        cmd_erase();
        break;
      case 'h':
        cmd_hello();
        break;
      case 'd':
        cmd_dump();
        break;

      case '.':
        cmd_stop();
        break;
    }
  }
}
