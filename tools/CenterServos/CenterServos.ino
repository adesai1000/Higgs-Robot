// Moves the servos on GPIO 13 and 14 to 90 degrees and holds them there.
// Used once per servo pair to fit the horns straight before assembly.
#include <ESP32Servo.h>

Servo servoA;
Servo servoB;

void setup() {
  Serial.begin(115200);

  servoA.setPeriodHertz(50);  servoA.attach(13, 500, 2400);
  servoB.setPeriodHertz(50);  servoB.attach(14, 500, 2400);

  servoA.write(90);
  servoB.write(90);
  Serial.println("Servos on GPIO 13 and 14 centered at 90");
}

void loop() {
  // Nothing to do. The ESP32's PWM hardware keeps sending the 90-degree
  // pulse on its own, so the servos keep holding center.
}
