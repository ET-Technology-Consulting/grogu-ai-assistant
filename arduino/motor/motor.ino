/*
  Motor test for an animatronic plush
  -----------------------------------
  Drives the toy's original DC motor through an L293D H-bridge,
  controlled from the Arduino Serial Monitor.

  Board:   Arduino Mega 2560 (any Arduino with PWM pins works)
  Driver:  L293D (channel 1)
  Monitor: 9600 baud

  Commands (type a letter, then press Enter):
    f  -> run forward
    b  -> run backward
    s  -> stop
    +  -> speed up
    -  -> slow down

  Safety: the motor stops automatically after RUN_TIMEOUT_MS
  so a stalled mechanism never overheats the motor or the L293D.
*/

// L293D pins
const int PIN_ENABLE = 5;  // L293D pin 1 (EN1), must be a PWM pin
const int PIN_IN1    = 7;  // L293D pin 2 (IN1)
const int PIN_IN2    = 8;  // L293D pin 7 (IN2)

// Settings
const int SPEED_MIN = 80;                       // below this, most toy motors will not start
const int SPEED_MAX = 255;
const int SPEED_STEP = 25;
const unsigned long RUN_TIMEOUT_MS = 3000;      // auto-stop after 3 seconds

int speed = 200;
int direction = 0;                              // 1 = forward, -1 = backward, 0 = stopped
unsigned long startedAt = 0;

void drive(int dir, int pwm) {
  digitalWrite(PIN_IN1, dir > 0 ? HIGH : LOW);
  digitalWrite(PIN_IN2, dir < 0 ? HIGH : LOW);
  analogWrite(PIN_ENABLE, dir == 0 ? 0 : pwm);
  direction = dir;
  startedAt = millis();
}

void printStatus() {
  Serial.print(F("Direction: "));
  Serial.print(direction > 0 ? F("forward") : direction < 0 ? F("backward") : F("stopped"));
  Serial.print(F(" | Speed: "));
  Serial.println(speed);
}

void setup() {
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  drive(0, 0);

  Serial.begin(9600);
  Serial.println(F("Motor test ready. Commands: f = forward, b = backward, s = stop, + / - = speed"));
}

void loop() {
  // Auto-stop to protect the motor and the driver
  if (direction != 0 && millis() - startedAt > RUN_TIMEOUT_MS) {
    drive(0, 0);
    Serial.println(F("Auto-stop (timeout)"));
  }

  if (!Serial.available()) return;
  char c = Serial.read();

  switch (c) {
    case 'f': drive(1, speed);  break;
    case 'b': drive(-1, speed); break;
    case 's': drive(0, 0);      break;
    case '+': speed = min(speed + SPEED_STEP, SPEED_MAX); if (direction != 0) drive(direction, speed); break;
    case '-': speed = max(speed - SPEED_STEP, SPEED_MIN); if (direction != 0) drive(direction, speed); break;
    default: return;            // ignore newlines and unknown keys
  }
  printStatus();
}
