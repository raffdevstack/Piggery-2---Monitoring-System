#define BLYNK_TEMPLATE_ID "TMPL6oTAALs3i"
#define BLYNK_TEMPLATE_NAME "Backyard Piggery"
#define BLYNK_AUTH_TOKEN "9m5XLGdZ6dQTm8fXzHY88WwK9jQ1NfwC"
#define BLYNK_PRINT Serial
#define DHT11_PIN D6    
#define MQ135_PIN A0 
#define RELAY_LIGHT D5
#define RELAY_FAN D7
#define BUZZER D8

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> // better lcd communication
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT11.h>
#include <MQ135.h>

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
float correctedPPM = 0;
bool beeping = false;

BlynkTimer timer;
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT11 dht(DHT11_PIN);
MQ135 mq135_sensor(MQ135_PIN);

// put function declarations here:
int myFunction(int, int);
void lcdPrinter(int, int, String);
void dispStartScreen();
void connectToWifi();
void lcdNotifier(String);
void connectToBlynk();
void readSensors();
void displaySensorPlaceholders();
void displayAppData();
void sendDataToBlynk();
double calculateHeatIndexCelsius(double, double);
void automateLightAndFan();
void odorAlarm();
void buzzerBeeper(int , int , unsigned long );

void setup() {
    // put your setup code here, to run once:
    // int result = myFunction(2, 3);

    Serial.begin(115200);
    delay(10);

    pinMode(RELAY_LIGHT, OUTPUT);  // Set GPIO14 as output
    pinMode(RELAY_FAN, OUTPUT);    // Set GPIO13 as output
    pinMode(BUZZER, OUTPUT);    // Set GPIO13 as output

    // Start with relays AND buzzer off
    digitalWrite(RELAY_LIGHT, HIGH);  // Start with the relay OFF for my ACTIVE LOW RELAY
    digitalWrite(RELAY_FAN, HIGH); // Start with the relay OFF for my ACTIVE LOW RELAY
    digitalWrite(BUZZER, LOW);

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

    timer.setInterval(20000L, connectToWifi);
    timer.setInterval(2000L, readSensors);
    timer.setInterval(2000L, sendDataToBlynk);
    timer.setInterval(5000L, displaySensorPlaceholders);
    timer.setInterval(5000L, displayAppData);
    timer.setInterval(5000L, automateLightAndFan);
    timer.setInterval(10000L, odorAlarm);

}

void loop() {
    // put your main code here, to run repeatedly:

    yield();

    unsigned long start = millis();

    if (Blynk.connected()) {
        Blynk.run();
        yield();
    }

    timer.run();
    yield();

    // Optional: Debug long iterations
    unsigned long duration = millis() - start;
    if (duration > 100) {  // Log if loop takes more than 100ms
        Serial.printf("Long loop: %lums\n", duration);
    }

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
            blynk_connected_state = false;
        }

        // try to reconnect
        WiFi.reconnect();
        yield();
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

    yield();

    if (!Blynk.connected()) { // not connected

        if (blynk_connected_state) {
            lcdNotifier("Blynk lost connection!");
            if (WiFi.disconnect()){
                wifi_connected_state = false;
                blynk_connected_state = false;
            }
            yield();
            return;
        } else {
            lcdNotifier("Blynk not connected!");
        }

        blynk_connected_state = false;
        
        if (Blynk.connect()) {
            yield();
            lcdNotifier("Blynk reconnected!");
            blynk_connected_state = true;
        } else {
            yield();
            lcdNotifier("Blynk failed reconn!");
            blynk_connected_state = false;
        }
        
        return;
    }

    // connected blynk

    if (!blynk_connected_state){
        lcdNotifier("Blynk connected!");
    }

    blynk_connected_state = true;
}

void readSensors() {

    yield();

    int result = dht.readTemperatureHumidity(temperature, humidity);

    if (result != 0) {
        lcdNotifier("DHT11 error!");
    } else {
        heat_index_celsius = calculateHeatIndexCelsius(temperature, humidity); 
    }

    // mq135 sensor
    mq135_sensor.getRZero();
    mq135_sensor.getCorrectedRZero(temperature, humidity);
    mq135_sensor.getResistance();
    mq135_sensor.getPPM();
    
    correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);

    if (correctedPPM > 0) {  // Check for valid reading
        odor_level = round(correctedPPM);
    }

}

void displaySensorPlaceholders() {

    lcdPrinter(0,0, "Odr:");

    if (odor_level > 99)
        lcdPrinter(7, 0, "ppm");
    else if (odor_level > 9)
        lcdPrinter(6, 0, "ppm");
    else if (odor_level > 0)
        lcdPrinter(5, 0, "ppm");

    lcdPrinter(12,0, "w");
    lcdPrinter(14,0, "b");

    // second row
    lcdPrinter(0,1, "t:");
    lcdPrinter(4,1, "C");
    lcdPrinter(6,1, "h:");
    lcdPrinter(10,1, "%");
    lcdPrinter(12,1, "i:");
}

void displayAppData() {
    lcdPrinter(4,0, String(odor_level));

    int w_state = 0;
    int b_state = 0;

    if (wifi_connected_state) 
        w_state = 1;
    else
        w_state = 0;

    if (blynk_connected_state)
        b_state = 1;
    else 
        b_state = 0;
    
    lcdPrinter(13,0, String(w_state));
    lcdPrinter(15,0, String(b_state));

    // second row
    lcdPrinter(2,1, String(temperature));
    lcdPrinter(8,1, String(humidity));
    lcdPrinter(14,1, String(heat_index_celsius));
}

void sendDataToBlynk() {

    yield();
    if (!Blynk.connected()) {
        return;
    }

    // connected to blynk zone

    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, heat_index_celsius);
    Blynk.virtualWrite(V3, odor_level);
    yield();
}

double calculateHeatIndexCelsius(double temperatureC, double humidity) {
    // Ensure inputs are valid
    if (temperatureC < 0 || humidity < 0 || humidity > 100) {
        Serial.println("Temperature must be non-negative and humidity must be between 0 and 100.");
    }

    // No heat index calculation for temperatures below 27°C
    if (temperatureC < 27) {
        return temperatureC;
    }

    // Convert Celsius to Fahrenheit
    double temperatureF = (temperatureC * 9.0) / 5.0 + 32.0;

    // Heat index formula in Fahrenheit
    double T = temperatureF;
    double RH = humidity;

    double heatIndexF = 
        -42.379 +
        2.04901523 * T +
        10.14333127 * RH -
        0.22475541 * T * RH -
        6.83783e-3 * T * T -
        5.481717e-2 * RH * RH +
        1.22874e-3 * T * T * RH +
        8.5282e-4 * T * RH * RH -
        1.99e-6 * T * T * RH * RH;

    // Convert the result back to Celsius
    double heatIndexC = ((heatIndexF - 32.0) * 5.0) / 9.0;

    // Round to 2 decimal places
    return round(heatIndexC * 100.0) / 100.0;
}

void automateLightAndFan() {

    // ---------- FOR LIGHTS

    // when it is dark
    // wait for the sensor

    // when it is cold
    if (heat_index_celsius > 10 && heat_index_celsius < 25) {
        digitalWrite(RELAY_LIGHT, LOW); // turn on
    } else {
        digitalWrite(RELAY_LIGHT, HIGH); // turn off
    }

    // ---------- FOR FANS
    
    if (heat_index_celsius > 45) {
        digitalWrite(RELAY_FAN, LOW); // turn on
    } else {
        digitalWrite(RELAY_FAN, HIGH); // turn off
    }
}

void odorAlarm() {

    if (correctedPPM > 100 && !beeping) {
        buzzerBeeper(500,500,5000);
    }

}

void buzzerBeeper(int beepDuration, int pauseDuration, unsigned long autoStopDuration) {
    beeping = true;
    unsigned long startTime = millis(); // Record the start time
    while (millis() - startTime < autoStopDuration) {
        // Beep ON
        digitalWrite(BUZZER, HIGH);
        delay(beepDuration);
        
        // Beep OFF
        digitalWrite(BUZZER, LOW);
        delay(pauseDuration);
    }
    // Ensure the buzzer is turned off after the autostop period
    digitalWrite(BUZZER, LOW);
    beeping = false;
}

