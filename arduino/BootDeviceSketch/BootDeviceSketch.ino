#define PIN_READ_BUS_CLOCK_EXEC 2
#define PIN_WRITE_TO_BUS 3

static bool did_write_to_bus = false;

static bool read_from_bus = false;
static uint8_t read_bus_value = 0;

static uint8_t write_bus_value = 0;
static bool write_to_bus = false;

typedef enum {
    WaitingForAck,
    SendingLowSize,
    SendingHighSize,
    SendingBytes
} State;

static State state = WaitingForAck;
static State prev_state = state;

static uint16_t state_sending_bytes_count = 0;

static const uint8_t demo_program[] = {
    /* 0x00 */ 0x01, 0x01, 0x89, 0x01, 0x02, 0x89, 0x01, 0x04, 0x89, 0x01, 0x08, 0x89, 0x01, 0x10, 0x89, 0x01,
    /* 0x10 */ 0x20, 0x89, 0x01, 0x40, 0x89, 0x01, 0x80, 0x89, 0x01, 0x40, 0x89, 0x01, 0x20, 0x89, 0x01, 0x10,
    /* 0x20 */ 0x89, 0x01, 0x08, 0x89, 0x01, 0x04, 0x89, 0x01, 0x02, 0x89, 0x01, 0x01, 0x89, 0x92, 0x00, 0x90};
static const uint8_t demo_size_high = sizeof(demo_program) >> 8;
static const uint8_t demo_size_low = sizeof(demo_program) & 0xff;

static void isr_read_from_bus_clock_exec() {
    read_bus_value = (PINC << 4) | (PINB & 0xf);
    read_from_bus = true;

    prev_state = state;

    switch (state) {
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
        case WaitingForAck:
            write_bus_value = 0x01;
            state = state;
            break;
        case SendingLowSize:
            write_bus_value = demo_size_low;
            state = SendingHighSize;
            break;
        case SendingHighSize:
            write_bus_value = demo_size_high;
            state = SendingBytes;
            state_sending_bytes_count = 0;
            break;
        case SendingBytes:
            write_bus_value = demo_program[state_sending_bytes_count++];

            if (state_sending_bytes_count >= sizeof(demo_program)) {
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
}

#define BUFFER_CAP 32
static char buffer[BUFFER_CAP];

static char *state_to_string(State s) {
    switch (s) {
    case WaitingForAck: return "WaitingForAck";
    case SendingLowSize: return "SendingLowSize";
    case SendingHighSize: return "SendingHighSize";
    case SendingBytes: return "SendingBytes";
    }
}

void loop() {
    if (prev_state != state) {
        sprintf(buffer, "%s -> %s\n", state_to_string(prev_state), state_to_string(state));
        Serial.write(buffer);

        prev_state = state;
    }

    if (did_write_to_bus && state == SendingBytes) {
        did_write_to_bus = false;

        sprintf(buffer, "%02x (%d)\n", write_bus_value, sizeof(demo_program) - state_sending_bytes_count);
        Serial.write(buffer);
    }
}
