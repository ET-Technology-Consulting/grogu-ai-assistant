// Grogu ESP32-S3 bring-up firmware.
//
// Tests every module of the new brain, one at a time, from the serial
// monitor (115200 baud): motor (DRV8833), speaker (MAX98357A),
// microphone (INMP441) and the head touch plate.
//
// Requires the Arduino-ESP32 core 3.x (ESP_I2S library, ledcAttach API).

#include <Arduino.h>
#include <ESP_I2S.h>
#include <math.h>

#include "pins.h"

// ---------- Settings ----------

constexpr uint32_t SAMPLE_RATE = 16000;     // Hz, speaker and mic
constexpr uint32_t MOTOR_PWM_FREQ = 20000;  // Hz, above audible range
constexpr uint8_t MOTOR_PWM_BITS = 8;
constexpr int MOTOR_SPEED_STEP = 25;
constexpr uint32_t MOTOR_AUTO_STOP_MS = 3000;
constexpr float TOUCH_RATIO = 1.15f;        // touched when raw > baseline * ratio
constexpr uint32_t RECORD_SECONDS = 2;
constexpr int MIC_SHIFT = 14;               // 32-bit mic sample -> 16-bit, ~x4 gain

// ---------- State ----------

I2SClass speaker;
I2SClass mic;

int motorSpeed = 200;          // 0..255
int motorDirection = 0;        // 1 forward, -1 backward, 0 stopped
uint32_t motorStartedAt = 0;

int volumePercent = 20;        // software volume, start low to protect the speaker

uint32_t touchBaseline = 0;
bool touched = false;
uint32_t lastTouchPoll = 0;

bool meterOn = false;
uint32_t lastMeter = 0;

uint32_t lastBlink = 0;
bool ledOn = false;

// ---------- Status LED (heartbeat, proves the firmware is running) ----------

void setLed(bool on) {
#ifdef RGB_BUILTIN
  rgbLedWrite(RGB_BUILTIN, 0, on ? 24 : 0, 0);  // DevKitC RGB LED, dim green
#else
  digitalWrite(LED_BUILTIN, on ? LOW : HIGH);   // XIAO user LED
#endif
}

void updateHeartbeat() {
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    ledOn = !ledOn;
    setLed(ledOn);
  }
}

// ---------- Motor ----------

void motorApply() {
  if (motorDirection > 0) {
    ledcWrite(PIN_MOTOR_IN1, motorSpeed);
    ledcWrite(PIN_MOTOR_IN2, 0);
  } else if (motorDirection < 0) {
    ledcWrite(PIN_MOTOR_IN1, 0);
    ledcWrite(PIN_MOTOR_IN2, motorSpeed);
  } else {
    ledcWrite(PIN_MOTOR_IN1, 0);
    ledcWrite(PIN_MOTOR_IN2, 0);
  }
}

void motorRun(int direction) {
  motorDirection = direction;
  motorStartedAt = millis();
  motorApply();
  Serial.printf("motor: %s at %d/255\n",
                direction > 0 ? "forward" : direction < 0 ? "backward" : "stop",
                motorSpeed);
}

void motorStop() {
  motorDirection = 0;
  motorApply();
}

void updateMotorAutoStop() {
  if (motorDirection != 0 && millis() - motorStartedAt >= MOTOR_AUTO_STOP_MS) {
    motorStop();
    Serial.println("motor: auto-stop");
  }
}

// ---------- Speaker ----------

void writeSamples(const int16_t *samples, size_t count) {
  speaker.write((const uint8_t *)samples, count * sizeof(int16_t));
}

// Plays a sine sweep from f0 to f1 Hz (f0 == f1 gives a plain tone).
void playSweep(float f0, float f1, uint32_t durationMs) {
  const size_t total = SAMPLE_RATE * durationMs / 1000;
  const float amplitude = 32767.0f * volumePercent / 100.0f;
  const size_t fade = SAMPLE_RATE / 100;  // 10 ms fade in/out, avoids clicks
  int16_t buf[256];
  float phase = 0;
  size_t n = 0;
  while (n < total) {
    size_t chunk = min((size_t)256, total - n);
    for (size_t i = 0; i < chunk; i++, n++) {
      float t = (float)n / total;
      float freq = f0 + (f1 - f0) * t;
      float env = 1.0f;
      if (n < fade) env = (float)n / fade;
      if (total - n < fade) env = (float)(total - n) / fade;
      buf[i] = (int16_t)(amplitude * env * sinf(phase));
      phase += 2.0f * PI * freq / SAMPLE_RATE;
      if (phase > 2.0f * PI) phase -= 2.0f * PI;
    }
    writeSamples(buf, chunk);
  }
}

void playSilence(uint32_t durationMs) {
  int16_t buf[256] = {0};
  size_t total = SAMPLE_RATE * durationMs / 1000;
  while (total > 0) {
    size_t chunk = min((size_t)256, total);
    writeSamples(buf, chunk);
    total -= chunk;
  }
}

// Three rising chirps: a placeholder "baby babble" until real TTS arrives.
void playChirp() {
  playSweep(600, 1400, 90);
  playSilence(40);
  playSweep(800, 1800, 90);
  playSilence(40);
  playSweep(700, 2200, 140);
  playSilence(50);
}

// ---------- Microphone ----------

// Reads mic samples and converts them to 16-bit. Returns the count read.
size_t readMic(int16_t *out, size_t count) {
  int32_t raw[256];
  size_t done = 0;
  while (done < count) {
    size_t want = min((size_t)256, count - done);
    size_t bytes = mic.readBytes((char *)raw, want * sizeof(int32_t));
    size_t got = bytes / sizeof(int32_t);
    if (got == 0) break;
    for (size_t i = 0; i < got; i++) {
      int32_t s = raw[i] >> MIC_SHIFT;
      out[done + i] = (int16_t)constrain(s, -32768, 32767);
    }
    done += got;
  }
  return done;
}

void updateMeter() {
  if (!meterOn || millis() - lastMeter < 100) return;
  lastMeter = millis();
  int16_t buf[256];
  size_t n = readMic(buf, 256);
  if (n == 0) {
    Serial.println("mic: no data (check SCK/WS/SD wiring)");
    return;
  }
  double mean = 0;
  for (size_t i = 0; i < n; i++) mean += buf[i];
  mean /= n;
  double sum = 0;
  for (size_t i = 0; i < n; i++) {
    double v = buf[i] - mean;
    sum += v * v;
  }
  int rms = (int)sqrt(sum / n);
  int bars = constrain(rms / 200, 0, 40);
  char line[41];
  memset(line, '#', bars);
  line[bars] = '\0';
  Serial.printf("mic %5d |%s\n", rms, line);
}

void recordAndPlay() {
  const size_t count = SAMPLE_RATE * RECORD_SECONDS;
  int16_t *buf = (int16_t *)malloc(count * sizeof(int16_t));
  if (!buf) {
    Serial.println("record: out of memory");
    return;
  }
  // Drop stale samples buffered while the mic was idle.
  int16_t scratch[256];
  for (int i = 0; i < 8; i++) readMic(scratch, 256);

  Serial.printf("record: talking for %u s... ", (unsigned)RECORD_SECONDS);
  size_t n = readMic(buf, count);
  Serial.printf("got %u samples, playing back\n", (unsigned)n);

  // Remove DC offset and apply the software volume.
  double mean = 0;
  for (size_t i = 0; i < n; i++) mean += buf[i];
  mean /= (n ? n : 1);
  for (size_t i = 0; i < n; i++) {
    int32_t v = (int32_t)((buf[i] - mean) * volumePercent / 20);
    buf[i] = (int16_t)constrain(v, -32768, 32767);
  }
  writeSamples(buf, n);
  playSilence(100);
  free(buf);
}

// ---------- Touch ----------

void calibrateTouch() {
  // The first read starts the touch sensor; readings right after are far too
  // high until it settles, so discard it and wait before sampling.
  touchRead(PIN_TOUCH);
  delay(100);
  uint64_t sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += touchRead(PIN_TOUCH);
    delay(10);
  }
  touchBaseline = sum / 16;
  Serial.printf("touch: baseline %u (do not touch the head while calibrating)\n",
                (unsigned)touchBaseline);
}

void updateTouch() {
  if (millis() - lastTouchPoll < 50) return;
  lastTouchPoll = millis();
  uint32_t raw = touchRead(PIN_TOUCH);
  bool now = raw > touchBaseline * TOUCH_RATIO;
  if (now == touched) return;
  touched = now;
  Serial.printf("touch: %s (raw %u)\n", touched ? "ON" : "off", (unsigned)raw);
  if (touched) {
    // Small reaction so the touch test is visible on camera.
    playChirp();
    motorRun(1);
    delay(400);
    motorStop();
  }
}

// ---------- Serial commands ----------

void printHelp() {
  Serial.println();
  Serial.println("Grogu bring-up commands:");
  Serial.println("  f / b / s   motor forward / backward / stop (auto-stop 3 s)");
  Serial.println("  + / -       motor speed up / down");
  Serial.println("  p           beep (440 Hz)");
  Serial.println("  g           Grogu chirp");
  Serial.println("  v / V       volume down / up");
  Serial.println("  m           toggle mic level meter");
  Serial.println("  r           record 2 s, then play it back");
  Serial.println("  t           print raw touch value");
  Serial.println("  c           recalibrate touch");
  Serial.println("  h           this help");
  Serial.println();
}

void handleCommand(char c) {
  switch (c) {
    case 'f': motorRun(1); break;
    case 'b': motorRun(-1); break;
    case 's': motorRun(0); break;
    case '+':
      motorSpeed = min(255, motorSpeed + MOTOR_SPEED_STEP);
      motorApply();
      Serial.printf("motor: speed %d/255\n", motorSpeed);
      break;
    case '-':
      motorSpeed = max(0, motorSpeed - MOTOR_SPEED_STEP);
      motorApply();
      Serial.printf("motor: speed %d/255\n", motorSpeed);
      break;
    case 'p':
      Serial.println("speaker: beep");
      playSweep(440, 440, 500);
      break;
    case 'g':
      Serial.println("speaker: chirp");
      playChirp();
      break;
    case 'v':
      volumePercent = max(5, volumePercent - 5);
      Serial.printf("volume: %d%%\n", volumePercent);
      break;
    case 'V':
      volumePercent = min(100, volumePercent + 5);
      Serial.printf("volume: %d%%\n", volumePercent);
      break;
    case 'm':
      meterOn = !meterOn;
      Serial.printf("mic meter: %s\n", meterOn ? "on" : "off");
      break;
    case 'r': recordAndPlay(); break;
    case 't':
      Serial.printf("touch: raw %u, baseline %u, threshold %u\n",
                    (unsigned)touchRead(PIN_TOUCH), (unsigned)touchBaseline,
                    (unsigned)(touchBaseline * TOUCH_RATIO));
      break;
    case 'c': calibrateTouch(); break;
    case 'h': printHelp(); break;
    default: break;  // ignore newlines and unknown keys
  }
}

// ---------- Setup / loop ----------

void setup() {
  Serial.begin(115200);
  delay(1500);  // give the USB serial monitor time to attach

#ifndef RGB_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif

  ledcAttach(PIN_MOTOR_IN1, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(PIN_MOTOR_IN2, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  motorStop();

  speaker.setPins(PIN_AMP_BCLK, PIN_AMP_LRC, PIN_AMP_DIN);
  if (!speaker.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT,
                     I2S_SLOT_MODE_MONO, I2S_STD_SLOT_BOTH)) {
    Serial.println("speaker: I2S init failed");
  }

  mic.setPins(PIN_MIC_SCK, PIN_MIC_WS, -1, PIN_MIC_SD);
  if (!mic.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_32BIT,
                 I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("mic: I2S init failed");
  }

  calibrateTouch();
  printHelp();
}

void loop() {
  while (Serial.available()) handleCommand((char)Serial.read());
  updateHeartbeat();
  updateMotorAutoStop();
  updateTouch();
  updateMeter();
}
