#define PIN_READ_BUS_CLOCK_EXEC 2
#define PIN_WRITE_TO_BUS 3

static bool did_write_to_bus = false;

static bool read_from_bus = false;
static uint8_t read_bus_value = 0;

static uint8_t write_bus_value = 0;
static bool write_to_bus = false;

typedef enum {
    WaitingForProgram,
    WaitingForAck,
    SendingLowSize,
    SendingHighSize,
    SendingBytes
} State;

static State state = WaitingForProgram;
static State prev_state = state;

static uint16_t state_sending_bytes_count = 0;

#define BUFFER_CAP 80
static char buffer[BUFFER_CAP];

#define BUFFER2_CAP 64
static char buffer2[BUFFER_CAP];

#define PROGRAM_SIZE_CAP 1024
static uint8_t loaded_program[PROGRAM_SIZE_CAP] = {0};
static uint16_t n_program_bytes;

static void isr_read_from_bus_clock_exec() {
    read_bus_value = (PINC << 4) | (PINB & 0xf);
    read_from_bus = true;

    prev_state = state;

    switch (state) {
    case WaitingForProgram: {
        state = state;
        break;
    }
    case WaitingForAck:
        if (read_bus_value == 0xab) {
            state = SendingLowSize;
        } else {
            state = state;
        }
        break;
    case SendingLowSize:
    case SendingHighSize:
    case SendingBytes:
        state = state;
        break;
    }
}

static void isr_write_to_bus_clock_exec() {
    prev_state = state;

    write_to_bus = !(PIND & 0b00001000);

    if (write_to_bus) {
        switch (state) {
        case WaitingForProgram:
            write_bus_value = 0xff;
            state = state;
            break;

        case WaitingForAck:
            write_bus_value = 0x01;
            state = state;
            break;
        case SendingLowSize:
            write_bus_value = n_program_bytes & 0xff;
            state = SendingHighSize;
            break;
        case SendingHighSize:
            write_bus_value = n_program_bytes >> 8;
            state = SendingBytes;
            state_sending_bytes_count = 0;
            break;
        case SendingBytes:
            write_bus_value = loaded_program[state_sending_bytes_count++];

            if (state_sending_bytes_count >= n_program_bytes) {
                state = WaitingForAck;
            } else {
                state = state;
            }
            break;
        }

        u8 output_value = write_bus_value;

        DDRB |= 0b00001111; // D8..D11 output. B0..B3
        DDRC |= 0b00001111; // A0..A3  output. B4..B7

        PORTB = (PORTB & 0xf0) | (output_value & 0xf);
        PORTC = (PORTC & 0xf0) | (output_value >> 4);

        did_write_to_bus = true;
    } else {
        DDRB &= 0b11110000; // D8..D11 input. B0..B3
        DDRC &= 0b11110000; // A0..A3  input. B4..B7
    }
}

void setup() {
    DDRB &= 0b11110000; // D8..D11 input. B0..B3
    DDRC &= 0b11110000; // A0..A3  input. B4..B7

    Serial.begin(115200);
    Serial.println("arduino boot device 1.0");

    pinMode(PIN_READ_BUS_CLOCK_EXEC, INPUT);
    pinMode(PIN_WRITE_TO_BUS, INPUT);

    attachInterrupt(digitalPinToInterrupt(PIN_READ_BUS_CLOCK_EXEC), isr_read_from_bus_clock_exec, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_WRITE_TO_BUS), isr_write_to_bus_clock_exec, CHANGE);

    sprintf(buffer2, "%s\n", state_to_string(state));
    Serial.write(buffer2);
}

static char *state_to_string(State s) {
    switch (s) {
    case WaitingForProgram: return "WaitingForProgram";
    case WaitingForAck: return "WaitingForAck";
    case SendingLowSize: return "SendingLowSize";
    case SendingHighSize: return "SendingHighSize";
    case SendingBytes: return "SendingBytes";
    }
}

void loop() {
    if (prev_state != state) {
        sprintf(buffer2, "%s -> %s\n", state_to_string(prev_state), state_to_string(state));
        Serial.write(buffer2);

        prev_state = state;
    }

    if (did_write_to_bus && state == SendingBytes) {
        did_write_to_bus = false;

        sprintf(buffer2, "%02x (%d)\n", write_bus_value, n_program_bytes - state_sending_bytes_count);
        Serial.write(buffer2);
    }

    if (state == WaitingForProgram) {
        Serial.println("Waiting on Intel HEX..");
        Serial.setTimeout(30000);

        n_program_bytes = 0;
        bool successful_read = false;

        while (true) {
            int read = Serial.readBytesUntil('\n', buffer, BUFFER_CAP);
            char tmp[5];

            if (read > 10 && buffer[0] == ':') {
                // :SSAAAARRBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBCC
                tmp[0] = buffer[7];
                tmp[1] = buffer[8];
                tmp[2] = '\0';
                uint8_t record_type = strtoul(tmp, NULL, 16);

                if (record_type == 0x00) {
                    tmp[0] = buffer[1];
                    tmp[1] = buffer[2];
                    tmp[2] = '\0';
                    uint8_t size = strtoul(tmp, NULL, 16);

                    tmp[0] = buffer[3];
                    tmp[1] = buffer[4];
                    tmp[2] = buffer[5];
                    tmp[3] = buffer[6];
                    tmp[4] = '\0';
                    uint16_t address = strtoul(tmp, NULL, 16);

                    if (read < 2 + 4 + 2 + size * 2 + 1) {
                        Serial.println("size > line length");
                        break;
                    }

                    tmp[0] = buffer[read - 2];
                    tmp[1] = buffer[read - 1];
                    tmp[2] = '\0';
                    uint8_t expected_checksum = strtoul(tmp, NULL, 16);

                    uint8_t checksum = size + ((address >> 8) & 0xff) + (address & 0xff) + record_type;

                    sprintf(buffer2, "S: %d, A: 0x%04x\n", size, address);
                    Serial.write(buffer2);

                    if (address != n_program_bytes) {
                        Serial.println("Out of order");
                        break;
                    } else if (n_program_bytes + size >= PROGRAM_SIZE_CAP) {
                        Serial.println("Too big");
                        break;
                    } else {
                        for (int i = 0; i < size; ++i) {
                            tmp[0] = buffer[i * 2 + 9];
                            tmp[1] = buffer[i * 2 + 9 + 1];
                            tmp[2] = '\0';
                            uint8_t read_byte = strtoul(tmp, NULL, 16);

                            loaded_program[n_program_bytes++] = read_byte;
                            checksum += read_byte;
                        }

                        checksum = ~checksum + 1;

                        if (checksum != expected_checksum) {
                            Serial.println("Checksum doesn't match");
                            break;
                        }
                    }
                } else if (record_type == 0x01) {
                    successful_read = true;
                    break;
                } else {
                    Serial.println("Unsupported record");
                    break;
                }
            } else {
                Serial.println("Unknown input");
                break;
            }
        }

        if (successful_read) {
            sprintf(buffer2, "Loaded %d bytes\n", n_program_bytes);
            Serial.write(buffer2);

            state = WaitingForAck;
        } else {
            Serial.println("Try again");
        }

        Serial.setTimeout(1000);
    }
}
