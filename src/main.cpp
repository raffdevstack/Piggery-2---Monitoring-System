#define BLYNK_TEMPLATE_ID "TMPL6oTAALs3i"
#define BLYNK_TEMPLATE_NAME "Backyard Piggery"
#define BLYNK_AUTH_TOKEN "9m5XLGdZ6dQTm8fXzHY88WwK9jQ1NfwC"
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> // better lcd communication
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

const char* ssid = "Hotspot_ko";
const char* pass = "abcdefghij";

bool wifi_connected_state = false;
bool blynk_connected_state = false;

BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// put function declarations here:
int myFunction(int, int);
void lcdPrinter(int, int, String);
void dispStartScreen();
void connectToWifi();
void lcdNotifier(String text);

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
    lcdNotifier("connecting to wifi");

    timer.setInterval(1000L, connectToWifi);

}

void loop() {
    // put your main code here, to run repeatedly:

    if (Blynk.connected()) {
        Blynk.run();
    }

    timer.run();
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

void lcdNotifier(String text) {
    lcd.clear();
    lcdPrinter(1, 0, "Info:");
    lcdPrinter(0, 1, "-" + text);
    delay(1000);
    lcd.clear();
}

void connectToWifi() {

    if (WiFi.status() != WL_CONNECTED) { // if not connected to wifi

        if (wifi_connected_state){
            lcdNotifier("WiFi DISCONNECTED!");
            wifi_connected_state = false;
        }

        // try to reconnect
        WiFi.reconnect();
        lcdNotifier("reconnecting to wifi");

        return; // wait for the next iteration
    } 

    // code below is when connected to wifi

    if (!wifi_connected_state) {
        lcd.clear();
        lcdPrinter(0,0,"WiFi CONNECTED!");
        lcdNotifier("WiFi CONNECTED!");
        wifi_connected_state = true;
    }

}