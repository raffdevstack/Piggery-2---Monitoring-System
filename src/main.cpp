#define BLYNK_TEMPLATE_ID "TMPL6oTAALs3i"
#define BLYNK_TEMPLATE_NAME "Backyard Piggery"
#define BLYNK_AUTH_TOKEN "9m5XLGdZ6dQTm8fXzHY88WwK9jQ1NfwC"
#define BLYNK_PRINT Serial
#define DHT11_PIN D6    

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> // better lcd communication
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT11.h>

const char* ssid = "Hotspot_ko";
const char* pass = "123456789";
// const char* ssid = "LUNA BOARDING HOUSE PISO WIFI";
// const char* pass = "";

bool wifi_connected_state = false;
bool blynk_connected_state = false;
int temperature = 0;
int humidity = 0;
double heat_index_celsius = 0;
int odor_level = 0;

BlynkTimer timer;
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT11 dht(DHT11_PIN);

// put function declarations here:
int myFunction(int, int);
void lcdPrinter(int, int, String);
void dispStartScreen();
void connectToWifi();
void lcdNotifier(String text);
void connectToBlynk();
void readSensors();
void displaySensorPlaceholders();
void displaySensorData();

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
    yield();
    delay(500);
    lcdNotifier("connecting to wifi");
    delay(2000);

    // blynk init
    Blynk.config(BLYNK_AUTH_TOKEN);

    timer.setInterval(10000L, connectToWifi);
    timer.setInterval(2000L, readSensors);
    timer.setInterval(5000L, displaySensorPlaceholders);
    timer.setInterval(2100L, displaySensorData);

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
    delay(2000);
}

void lcdNotifier(String text) {
    lcd.clear();
    lcdPrinter(1, 0, "Info:");
    lcdPrinter(0, 1, "-" + text);
    delay(2000);
    lcd.clear();
}

void connectToWifi() {

    yield();

    if (WiFi.status() != WL_CONNECTED) { // if not connected to wifi

        if (wifi_connected_state){
            lcdNotifier("WiFi DISCONNECTED!");
            wifi_connected_state = false;
        }

        // try to reconnect
        WiFi.reconnect();
        delay(1000);
        lcdNotifier("wifi reconnecting");

        return; // wait for the next iteration
    } 

    // code below is when connected to wifi

    if (!wifi_connected_state) {
        lcdNotifier("WiFi CONNECTED!");
        wifi_connected_state = true;
    }

    connectToBlynk();

}

void connectToBlynk() {

    if (!Blynk.connected()) { // not connected
        if (blynk_connected_state) {
            lcdNotifier("Blynk lost connection!");
        } else {
            lcdNotifier("Blynk not connected!");
        }

        blynk_connected_state = false;
        
        Blynk.connect(); // official blynk connector
        
        return;
    }

    // connected blynk

    if (!blynk_connected_state){
        Serial.println("Blynk online again!");
        blynk_connected_state = true;
    }
}

void readSensors() {

    int result = dht.readTemperatureHumidity(temperature, humidity);

    if (result != 0) {
        lcdNotifier("DHT11 error!");
    }
}

void displaySensorPlaceholders() {
    lcdPrinter(0,0, "Tmp:");
    lcdPrinter(6,0, "C");
    lcdPrinter(8,0, "Hmd:");
    lcdPrinter(14,0, "%");
    // second row
    lcdPrinter(0,1, "Odor Lvl:");
    lcdPrinter(12,1, "ppm");
}

void displaySensorData() {
    lcdPrinter(4,0, String(temperature));
    lcdPrinter(12,0, String(humidity));
    lcdPrinter(9,1, String(odor_level));
}