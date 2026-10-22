// Pin map shared by the Seeed XIAO ESP32S3 (inside Grogu) and the
// ESP32-S3-DevKitC-1 (bench). Same GPIO numbers on both boards, so the
// firmware runs unchanged on either one.
#pragma once

// Capacitive touch plate on top of the head (blue wire, T1).
constexpr int PIN_TOUCH = 1;      // XIAO D0

// DRV8833 motor driver, first channel (motor on OUT1 / OUT2).
constexpr int PIN_MOTOR_IN1 = 2;  // XIAO D1 -> IN1
constexpr int PIN_MOTOR_IN2 = 3;  // XIAO D2 -> IN2

// MAX98357A I2S amplifier (speaker output).
constexpr int PIN_AMP_BCLK = 4;   // XIAO D3 -> BCLK
constexpr int PIN_AMP_LRC = 5;    // XIAO D4 -> LRC
constexpr int PIN_AMP_DIN = 6;    // XIAO D5 -> DIN

// INMP441 I2S microphone (L/R tied to GND -> left slot).
constexpr int PIN_MIC_SCK = 7;    // XIAO D8 -> SCK
constexpr int PIN_MIC_WS = 8;     // XIAO D9 -> WS
constexpr int PIN_MIC_SD = 9;     // XIAO D10 -> SD
