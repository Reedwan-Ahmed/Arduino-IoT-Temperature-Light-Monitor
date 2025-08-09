#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Initialize LCD with I2C address and size

// Sensor pins
const int LDRpin = A0;
const int LM35pin = A1;

// LEDs
const int greenLED = 7;
const int redLED = 8;

// Keypad setup (4x4)
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 9, 10, 11};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int NUM_SAMPLES = 30;
const float Vref = 5.02;

// Thresholds
int lightThreshold = 400;
float tempThreshold = 35.0;

bool settingThreshold = false;
bool settingLight = false;
bool settingTemp = false;

String inputBuffer = "";

int readAvg(int pin) {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / NUM_SAMPLES;
}

void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  char key = keypad.getKey();

  if (settingThreshold) {
    handleThresholdInput(key);
  } else {
    if (key == 'A') {
      settingThreshold = true;
      inputBuffer = "";
      lcd.clear();
      lcd.print("Set Threshold:");
      lcd.setCursor(0, 1);
      lcd.print("1:Light 2:Temp");
    } else {
      normalOperation();
    }
  }
}

void normalOperation() {
  int ldr = readAvg(LDRpin);
  int tempRaw = readAvg(LM35pin);
  float voltage = tempRaw * (Vref / 1023.0);
  float tempC = voltage * 100.0;

  // LEDs control
  digitalWrite(greenLED, ldr < lightThreshold ? HIGH : LOW);
  digitalWrite(redLED, tempC > tempThreshold ? HIGH : LOW);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("L:");
  lcd.print(ldr);
  lcd.print("/");
  lcd.print(lightThreshold);

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(tempC, 1);
  lcd.print("/");
  lcd.print(tempThreshold, 1);
  lcd.print("C");

  // **Print exact same values on Serial for Python**
  Serial.print("LDR:");
  Serial.print(ldr);
  Serial.print(",TEMP:");
  Serial.println(tempC, 1);

  delay(1000);  // 1 second delay to sync LCD and Serial
}

void handleThresholdInput(char key) {
  if (key) {
    if (!settingLight && !settingTemp) {
      if (key == '1') {
        settingLight = true;
        inputBuffer = "";
        lcd.clear();
        lcd.print("Light Threshold:");
        lcd.setCursor(0, 1);
        lcd.print(inputBuffer);
      } else if (key == '2') {
        settingTemp = true;
        inputBuffer = "";
        lcd.clear();
        lcd.print("Temp Threshold:");
        lcd.setCursor(0, 1);
        lcd.print(inputBuffer);
      } else {
        lcd.clear();
        lcd.print("Press 1 or 2");
        delay(1000);
        lcd.clear();
        lcd.print("Set Threshold:");
        lcd.setCursor(0, 1);
        lcd.print("1:Light 2:Temp");
      }
    } else {
      if (key >= '0' && key <= '9') {
        if (inputBuffer.length() < 4) {
          inputBuffer += key;
          lcd.setCursor(0, 1);
          lcd.print(inputBuffer + "    ");
        }
      } else if (key == '#') {
        if (inputBuffer.length() > 0) {
          if (settingLight) {
            lightThreshold = inputBuffer.toInt();
            settingLight = false;
          } else if (settingTemp) {
            tempThreshold = inputBuffer.toFloat();
            settingTemp = false;
          }
          settingThreshold = false;
          lcd.clear();
          lcd.print("Set Complete!");
          delay(1000);
        }
      } else if (key == '*') {
        settingLight = false;
        settingTemp = false;
        settingThreshold = false;
        lcd.clear();
        lcd.print("Cancelled");
        delay(1000);
      }
    }
  }
}
