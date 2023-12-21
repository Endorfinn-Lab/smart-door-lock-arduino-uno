#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>
#include <EEPROM.h>

#define SERVO_PIN 3
#define BUZZER_PIN 12
#define BUTTON_PIN 13
#define RESET_BUTTON_PIN 1
#define GREEN_LED_PIN 10
#define RED_LED_PIN 11
#define LCD_RS 8
#define LCD_EN 9
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define NUM_KEYS 12
#define ROWS 4
#define COLS 3

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Servo myServo;

const int PASSWORD_LENGTH = 4;
char correctPassword[PASSWORD_LENGTH + 1];
char enteredPassword[PASSWORD_LENGTH + 1];

int attempts = 0;

void readPasswordFromEEPROM() {
  for (int i = 0; i < PASSWORD_LENGTH; i++) {
    correctPassword[i] = EEPROM.read(i);
  }
  correctPassword[PASSWORD_LENGTH] = '\0';
}

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Masukkan Kode:");

  pinMode(SERVO_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  myServo.attach(SERVO_PIN);

  // Membaca password dari EEPROM saat program dimulai
  readPasswordFromEEPROM();
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    tone(BUZZER_PIN, 1000, 100);
    lcd.setCursor(strlen(enteredPassword), 1);
    lcd.print('*');

    if (strlen(enteredPassword) < PASSWORD_LENGTH) {
      enteredPassword[strlen(enteredPassword)] = key;
      enteredPassword[strlen(enteredPassword) + 1] = '\0';
    }

    if (strlen(enteredPassword) == PASSWORD_LENGTH) {
      if (strcmp(enteredPassword, correctPassword) == 0) {
        unlockDoor();
        delay(3000);
        lockDoor();
      } else {
        alarm();
        attempts++;

        if (attempts == 3) {
          longAlarm();
          attempts = 0;
        }
        resetPassword();
      }
    }
  }

  if (digitalRead(BUTTON_PIN) == LOW) {
    unlockDoor();
    delay(3000);
    lockDoor();
  }

  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    resetPasswordFromButton();
  }
}

void unlockDoor() {
  myServo.write(0);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  lcd.clear();
  lcd.print("Terbuka");
}

void lockDoor() {
  myServo.write(90);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  lcd.clear();
  lcd.print("Kode:");
  resetPassword();
}

void alarm() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 2000, 200);
    delay(500);
  }
}

void longAlarm() {
  for (int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, 2000, 500);
    delay(500);
  }
}

void resetPassword() {
  memset(enteredPassword, 0, sizeof(enteredPassword));
}

void resetPasswordFromButton() {
  lcd.clear();
  lcd.print("Reset Kode");
  delay(1000);
  lcd.clear();

  // Input password baru
  lcd.print("Kode Baru:");
  delay(1000);
  resetPassword();

  while (strlen(enteredPassword) < PASSWORD_LENGTH) {
    char key = keypad.getKey();
    if (key) {
      lcd.setCursor(strlen(enteredPassword), 1);
      lcd.print('*');
      enteredPassword[strlen(enteredPassword)] = key;
      enteredPassword[strlen(enteredPassword) + 1] = '\0';
      delay(500);
    }
  }

  // Simpan password baru ke EEPROM
  for (int i = 0; i < PASSWORD_LENGTH; i++) {
    EEPROM.write(i, enteredPassword[i]);
  }

  // Membaca password dari EEPROM setelah mengubah password
  readPasswordFromEEPROM();

  lcd.clear();
  lcd.print("Kode Reset");
  delay(2000);
  lcd.clear();
  lcd.print("Masukkan Kode:");
  resetPassword();
}
