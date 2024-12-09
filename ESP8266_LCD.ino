#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// wifi
// const char* ssid     = "HOME-386A";        
// const char* password = "crumb3585beside"; 
// hotspot
const char* ssid     = "Kaniesx";        
const char* password = "12345678910"; 

// OpenWeatherMap API
String APIKEY        = "d896e5df16b1887343e330853c6a0d92";        
String CityID        = "4180439";                  
char servername[]    = "api.openweathermap.org";   

// weather data vars
String weatherDescription = "";
String weatherLocation = "";
String country = "";
float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

LiquidCrystal_I2C lcd(0x27, 16, 2); // lcd

WiFiClient client;

int counter = 0; // counter for updates

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // waiting for serial port
  }
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi..");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { // while loop til connected to wifi
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  lcd.clear();
  lcd.print("WiFi Connected!");
  delay(1000);
  
  fetchWeatherData(); // initial weather fetch call
}

void loop() {
  if (counter >= 60) {
    counter = 0;
    fetchWeatherData(); // update weather data
  }
  
  displayWeather();
  delay(5000);
  displayConditions();
  delay(5000); 
  counter++;
}

void fetchWeatherData() {
  Serial.println("\nStarting weather fetch.");
  
  if (!client.connect(servername, 80)) {
    Serial.println("Connection failed");
    return;
  }
  
  String url = "/data/2.5/weather?id=" + CityID + "&units=imperial&appid=" + APIKEY; // changed to imperial for fahrenheit
  Serial.println("Requesting URL: " + url);
  
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: " + String(servername));
  client.println("Connection: close");
  client.println();
  
  Serial.println("Waiting for request response");
  
  unsigned long timeout = millis();
  while (client.available() == 0) { // waiit for available data
    if (millis() - timeout > 5000) {
      Serial.println("Timeout!!");
      client.stop();
      return;
    }
  }
  
  Serial.println("Response headers:");
  char endOfHeaders[] = "\r\n\r\n"; // skip headers
  if (!client.find(endOfHeaders)) {
    Serial.println("Bad response");
    return;
  }
  
  Serial.println("JSON response:");
  String json = client.readString();
  Serial.println(json); // debug
  
  client.stop();
  
  StaticJsonDocument<2048> doc; // increase buffer
  
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  // extracting and checking values
  weatherLocation = doc["name"].as<String>();
  Serial.print("Location: "); Serial.println(weatherLocation);
  
  country = doc["sys"]["country"].as<String>();
  Serial.print("Country: "); Serial.println(country);
  
  if (doc["weather"][0]["description"]) {
    weatherDescription = doc["weather"][0]["description"].as<String>();
    Serial.print("Description: "); Serial.println(weatherDescription);
  }
  
  if (doc["main"]["temp"]) {
    temperature = doc["main"]["temp"].as<float>();
    Serial.print("Temperature: "); Serial.println(temperature);
  }
  
  if (doc["main"]["humidity"]) {
    humidity = doc["main"]["humidity"].as<float>();
    Serial.print("Humidity: "); Serial.println(humidity);
  }
  
  if (doc["main"]["pressure"]) {
    pressure = doc["main"]["pressure"].as<float>();
    Serial.print("Pressure: "); Serial.println(pressure);
  }
  
  Serial.println("Data update complete");
}

void displayWeather() {
  lcd.clear();
  
  Serial.println("\nWeather:");
  Serial.print("Location: "); Serial.println(weatherLocation);
  Serial.print("Description: "); Serial.println(weatherDescription);
  
  lcd.setCursor(0, 0);
  if (weatherLocation != "" && country != "") {
    lcd.print(weatherLocation + "," + country);
  } else {
    lcd.print("Loading...");
  }
  
  lcd.setCursor(0, 1);
  if (weatherDescription != "") {
    lcd.print(weatherDescription);
  } else {
    lcd.print("Please wait...");
  }
}

void displayConditions() {
  lcd.clear();
  
  // Debug print current values
  Serial.println("\nDisplaying Conditions:");
  Serial.print("Temp: "); Serial.println(temperature);
  Serial.print("H: "); Serial.println(humidity);
  Serial.print("P: "); Serial.println(pressure);
  
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print((char)223);
  lcd.print("F");
  
  lcd.print(" ");  // Add space between temperature and humidity
  
  lcd.print("H:");
  lcd.print(humidity, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(pressure, 0);
  lcd.print("hPa");
}