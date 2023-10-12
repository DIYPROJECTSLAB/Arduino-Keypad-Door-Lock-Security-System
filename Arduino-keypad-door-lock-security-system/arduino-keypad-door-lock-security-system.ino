
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>  // Include the Servo library
#include "SafeState.h"
#include "icons.h"

/* Define the I2C LCD object */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

/* SafeState stores the secret code in EEPROM */
SafeState safeState;

/* Define the buzzer and LED pins */
const int buzzerPin = 8;   // Change this to your buzzer control pin
const int ledOnPin = 9;   // Change this to your LED "ON" indicator pin
const int ledOffPin = 10;  // Change this to your LED "OFF" indicator pin
/* Define the servo motor */
Servo lockServo;
const int servoPin = 6;  // Change this to your servo control pin

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledOnPin, OUTPUT);
  pinMode(ledOffPin, OUTPUT);
  
  lcd.init();
  lcd.backlight();

  lockServo.attach(servoPin);  // Attach the servo to the specified pin

  showStartupMessage();
}

void loop() {
  if (safeState.locked()) {
    safeLockedLogic();
  } else {
    safeUnlockedLogic();
  }
}

void lock() {
  lockServo.write(0);  // Lock position (adjust as needed)
  safeState.lock();
  digitalWrite(ledOnPin, LOW);
  digitalWrite(ledOffPin, HIGH);
}

void unlock() {
  lockServo.write(90);  // Unlock position (adjust as needed)
  digitalWrite(ledOnPin, HIGH);
  digitalWrite(ledOffPin, LOW);
}

void showStartupMessage() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  delay(10);

  lcd.setCursor(0, 1);
  String message = "Security System";
  for (byte i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    delay(70);
  }
  delay(500);
}

// The rest of your code remains the same, as it already handles servo operations.
String inputSecretCode() {
  lcd.setCursor(5, 1);
  lcd.print("[____]");
  lcd.setCursor(6, 1);
  String result = "";
  while (result.length() < 4) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      lcd.print('*');
      result += key;
    }
  }
  return result;
}

void showWaitScreen(int delayMillis) {
  lcd.setCursor(2, 1);
  lcd.print("[..........]");
  lcd.setCursor(3, 1);
  for (byte i = 0; i < 10; i++) {
    delay(delayMillis);
    lcd.print("=");
  }
}

bool setNewCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter new code:");
  String newCode = inputSecretCode();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirm new code");
  String confirmCode = inputSecretCode();

  if (newCode.equals(confirmCode)) {
    safeState.setCode(newCode);
    return true;
  } else {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Code mismatch");
    lcd.setCursor(0, 1);
    lcd.print(" Try Again!");
    delay(2000);
    return false;
  }
}

void showUnlockMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Granted");
  lcd.setCursor(0, 1);
  lcd.print("Welcome");
  delay(2000);
  unlock();
  unlockbuzz();
}

void safeUnlockedLogic() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(" Enter # to lock");

  bool newCodeNeeded = true;

  if (safeState.hasCode()) {
    lcd.setCursor(0, 1);
    lcd.print("  A = new code");
    newCodeNeeded = false;
  }

  char key = keypad.getKey();
  while (key != 'A' && key != '#') {
    key = keypad.getKey();
  }

  bool readyToLock = true;
  if (key == 'A' || newCodeNeeded) {
    readyToLock = setNewCode();
  }

  if (readyToLock) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Wait... ");
    safeState.lock();
    lock();
    showWaitScreen(100);
  }
}

void safeLockedLogic() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Your Password");

  String userCode = inputSecretCode();
  bool unlockedSuccessfully = safeState.unlock(userCode);
  showWaitScreen(200);

  if (unlockedSuccessfully) {
    showUnlockMessage();
    unlock();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Get Out of Here!");
    lcd.setCursor(0, 1);
   
    unlockbuzz();
    showWaitScreen(1000);
  }
}
void unlockbuzz() {
  // Add your buzzer logic here
  // For example, play a sound
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
  delay(100);
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
  delay(100);
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
}
void incorrect() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied");
  lcd.setCursor(0, 1);
  lcd.print("Try Again!");
  unlockbuzz();
  showWaitScreen(1000);
} 
