#include <Bluepad32.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
ControllerPtr pad = nullptr;
Servo legLeft;
Servo legRight;

// ---- robot state: the single source of truth ----
int  leftAngle  = 90;
int  rightAngle = 90;
int  lookX = 0;               // eye offset in pixels, -10..10
int  lookY = 0;               // -6..6
bool eyesOpen = true;

// ---- timers ----
unsigned long eyesLastChange = 0;
unsigned long lastFrame = 0;
const int FRAME_MS = 33;      // redraw the face ~30 times a second
unsigned long lastPrint = 0;
const int PRINT_MS = 200;     // print servo angles 5 times a second

void onConnected(ControllerPtr ctl)    { Serial.println("Controller connected");    pad = ctl; }
void onDisconnected(ControllerPtr ctl) { Serial.println("Controller disconnected"); pad = nullptr; }

int applyDeadzone(int axis) {
  const int DEADZONE = 40;
  return (abs(axis) < DEADZONE) ? 0 : axis;
}

int stickToAngle(int axis) {
  return map(applyDeadzone(axis), -512, 511, 0, 180);
}

// ---- input: reads the controller, writes state, nothing else ----
void readInput() {
  if (!(BP32.update() && pad && pad->isConnected() && pad->hasData())) return;

  leftAngle  = stickToAngle(pad->axisX());
  rightAngle = stickToAngle(pad->axisRX());

  // eyes follow whichever stick is being pushed
  int x = applyDeadzone(pad->axisX());
  int y = applyDeadzone(pad->axisY());
  if (x == 0 && y == 0) {
    x = applyDeadzone(pad->axisRX());
    y = applyDeadzone(pad->axisRY());
  }
  lookX = map(x, -512, 511, -10, 10);
  lookY = map(y, -512, 511, -6, 6);
}

// ---- outputs: render state to hardware ----
void updateServos() {
  legLeft.write(leftAngle);
  legRight.write(rightAngle);
}

void updateBlink(unsigned long now) {
  int wait = eyesOpen ? 3000 : 150;
  if (now - eyesLastChange >= wait) {
    eyesOpen = !eyesOpen;
    eyesLastChange = now;
  }
}

// prints the angles so you can read off leg poses in the Serial Monitor
void printAngles(unsigned long now) {
  if (now - lastPrint < PRINT_MS) return;
  lastPrint = now;
  Serial.printf("hip (13): %3d   knee (14): %3d\n", leftAngle, rightAngle);
}

void drawFace() {
  display.clearDisplay();
  if (eyesOpen) {
    display.fillCircle(40 + lookX, 40 + lookY, 12, SSD1306_WHITE);
    display.fillCircle(88 + lookX, 40 + lookY, 12, SSD1306_WHITE);
  } else {
    display.fillRect(28 + lookX, 38, 24, 4, SSD1306_WHITE);
    display.fillRect(76 + lookX, 38, 24, 4, SSD1306_WHITE);
  }
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  BP32.setup(&onConnected, &onDisconnected);
  BP32.enableNewBluetoothConnections(true);

  legLeft.setPeriodHertz(50);   legLeft.attach(13, 500, 2400);
  legRight.setPeriodHertz(50);  legRight.attach(14, 500, 2400);

  updateServos();
  drawFace();
}

void loop() {
  unsigned long now = millis();
  readInput();
  updateServos();
  printAngles(now);
  updateBlink(now);
  if (now - lastFrame >= FRAME_MS) {
    lastFrame = now;
    drawFace();
  }
}
