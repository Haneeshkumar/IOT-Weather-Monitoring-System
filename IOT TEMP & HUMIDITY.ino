#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <DHT.h>

// Wi-Fi credentials
#define WIFI_SSID "realme 8"
#define WIFI_PASS "hanish101"

// Adafruit IO credentials
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "your_username"
#define AIO_KEY "your_secret_key"

// DHT11 setup
#define DHTPIN 2 // GPIO pin for the DHT11 sensor (D4 on NodeMCU)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Create an MQTT client
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// MQTT feeds
Adafruit_MQTT_Publish temperatureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidityFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

void setup() {
    Serial.begin(115200);
    delay(10);

    // Connect to Wi-Fi
    connectWiFi();

    // Initialize DHT sensor
    dht.begin();

    Serial.println("System Ready");
}

void loop() {
    // Ensure Wi-Fi connection
    checkWiFi();

    // Ensure MQTT connection
    if (!mqtt.connected()) {
        connectMQTT();
    }
    mqtt.processPackets(10000); // Handle MQTT packets

    // Read temperature and humidity from the sensor
    float temperature = readDHTTemperature();
    float humidity = readDHTHumidity();

    // Debugging: Print sensor readings
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("C, Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    // Publish data to Adafruit IO
    if (!temperatureFeed.publish(temperature)) {
        Serial.println("Failed to send temperature!");
    } else {
        Serial.print("Temperature sent: ");
        Serial.println(temperature);
    }

    if (!humidityFeed.publish(humidity)) {
        Serial.println("Failed to send humidity!");
    } else {
        Serial.print("Humidity sent: ");
        Serial.println(humidity);
    }

    // Ensure MQTT connection stays active
    if (!mqtt.ping()) {
        Serial.println("MQTT ping failed, reconnecting...");
        mqtt.disconnect();
        connectMQTT();
    }

    // Wait before next reading
    delay(10000); // 10 seconds
}

// Function to connect to Wi-Fi
void connectWiFi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");
}

// Function to check and reconnect Wi-Fi if needed
void checkWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnecting Wi-Fi...");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nWi-Fi reconnected!");
    }
}

// Function to connect to MQTT
void connectMQTT() {
    Serial.println("Connecting to Adafruit IO...");
    int8_t ret;
    while ((ret = mqtt.connect()) != 0) {
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT in 5s...");
        delay(5000);
    }
    Serial.println("Connected to Adafruit IO!");
}

// Function to read temperature with retries
float readDHTTemperature() {
    float temp = NAN;
    for (int i = 0; i < 3; i++) { // Retry 3 times
        temp = dht.readTemperature();
        if (!isnan(temp)) break;
        delay(200);
    }
    return temp;
}

// Function to read humidity with retries
float readDHTHumidity() {
    float hum = NAN;
    for (int i = 0; i < 3; i++) {
        hum = dht.readHumidity();
        if (!isnan(hum)) break;
        delay(200);
    }
    return hum;
}
