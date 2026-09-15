#include <Arduino.h>
#include <stdint.h>
#include <string.h>

// Keep the V2.0 module at its default 115200 baud. At 9600 baud, the Uno's
// single UART cannot reliably receive 39-byte frames and emit longer CSV text.
static const unsigned long SERIAL_BAUD = 115200;
static const uint8_t FRAME_HEADER = 0xAA;
static const uint8_t FRAME_SIZE = 39;
static const uint8_t SENSOR_COUNT = 18;

static uint8_t frameBuffer[FRAME_SIZE];
static uint8_t frameLength = 0;

static bool frameIsValid(const uint8_t *frame) {
  if (frame[0] != FRAME_HEADER || (frame[1] != 0x01 && frame[1] != 0x02)) {
    return false;
  }

  uint8_t checksum = 0;
  for (uint8_t i = 0; i < FRAME_SIZE - 1; ++i) {
    checksum = static_cast<uint8_t>(checksum + frame[i]);
  }
  return checksum == frame[FRAME_SIZE - 1];
}

static void emitFrame(const uint8_t *frame) {
  Serial.print(F("P,"));
  Serial.print(frame[1] == 0x01 ? F("left") : F("right"));
  for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
    const uint8_t offset = 2 + 2 * i;
    const uint16_t value =
        (static_cast<uint16_t>(frame[offset]) << 8) | frame[offset + 1];
    Serial.print(',');
    Serial.print(value);
  }
  Serial.println();
}

static void resynchronize(void) {
  for (uint8_t i = 1; i < FRAME_SIZE; ++i) {
    if (frameBuffer[i] == FRAME_HEADER) {
      frameLength = FRAME_SIZE - i;
      memmove(frameBuffer, frameBuffer + i, frameLength);
      return;
    }
  }
  frameLength = 0;
}

void setup() {
  // On Uno, RX0 receives the sensor and TX0 returns parsed CSV over USB.
  // Disconnect the sensor TX wire while uploading this sketch.
  Serial.begin(SERIAL_BAUD);
}

void loop() {
  while (Serial.available() > 0) {
    const uint8_t value = static_cast<uint8_t>(Serial.read());
    if (frameLength == 0 && value != FRAME_HEADER) {
      continue;
    }

    frameBuffer[frameLength++] = value;
    if (frameLength != FRAME_SIZE) {
      continue;
    }

    if (frameIsValid(frameBuffer)) {
      emitFrame(frameBuffer);
      frameLength = 0;
    } else {
      resynchronize();
    }
  }
}
