#include <Servo.h>

Servo myServo;

void setup() {
  // Start serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Attach the servo signal line to Pin 9
  myServo.attach(9);
  
  // Move servo to initial position (0 degrees)
  myServo.write(0);
}

void loop() {
  // Check if serial data is coming from the browser
  if (Serial.available() > 0) {
    // Read string until line break (\n)
    String input = Serial.readStringUntil('\n');
    input.trim();

    // Parse the input string into an integer degree (0 - 180)
    int angle = input.toInt();

    // Constrain the input within safe servo limits
    if (angle >= 0 && angle <= 180) {
      myServo.write(angle);
    }
  }
}