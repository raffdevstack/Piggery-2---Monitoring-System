#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> // better lcd communication
#include <ESP8266WiFi.h>

const char* ssid = "Hotspot_ko";
const char* pass = "abcdefghij";

bool wifi_connected = false;
bool blynk_connected = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// put function declarations here:
int myFunction(int, int);
void lcdPrinter(int, int, String);
void dispStartScreen();

void setup() {
    // put your setup code here, to run once:
    // int result = myFunction(2, 3);

    Serial.begin(115200);
    delay(10);

    lcd.init();
    lcd.clear();
    lcd.backlight();

    dispStartScreen();
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

}

void loop() {
    // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
    return x + y;
}

void lcdPrinter(int cursor, int row, String text) {
    lcd.setCursor(cursor, row); // Assuming you want to set the cursor to a specific column on row 0
    lcd.print(text);
}

void dispStartScreen() {
    lcdPrinter(1,0,"Welcome");
    lcdPrinter(0,1,"PIGGERY MONITOR");
}
