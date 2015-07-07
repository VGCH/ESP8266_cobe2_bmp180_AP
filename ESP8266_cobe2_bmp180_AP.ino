//Мини метеостанция с веб сервером
// V.G.C
// https://twitter.com/generator_cher
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include <EEPROM.h>
#define DHTPIN 12 //GPIO12
#define DHTTYPE DHT22   // DHT 22 
DHT dht(DHTPIN, DHTTYPE,15);

const char* ssid = "Your Wi-Fi Net"; // Имя сети для подключения в режиме клиента
const char* ssid2 = "Wi-Fi-METEOSTATION(1313131313)"; // Имя сети в режиме точки доступа
const char* password = "Your pass"; // Пароль сети в режиме клиента
const char* password2 = "1313131313"; // Пароль сети в режиме точки доступа

#define ONE_WIRE_BUS 14  // DS18B20 pin GPIO14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
ESP8266WebServer server(80);
//#define REPORT_INTERVAL 60
const int led = 13;
Adafruit_BMP085 bmp;
void setup(void)
{
  Serial.begin(9600);
  Wire.pins(0, 2);// устанавливаем пины SDA,SCL для i2c
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
  
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  // Connect to WiFi network
  // Выбираем режим Wi-Fi
  // Выбор режимов выполняется изменением состояния 13 пина.
  // если 1, то точка доступа
  // если 0, то режим клиента
  if (digitalRead(13)==1)
  {
  WiFi.mode(WIFI_AP); //Режим Wi-Fi Точка доступа
  WiFi.softAP(ssid2, password2);//Поднятие точки доступа
  Serial.println("");
  Serial.println("AP mode"); //Говорим что мы в режиме точки доступа
  }
  else
  {
   WiFi.mode(WIFI_STA); //Режим Wi-Fi клиент
   WiFi.begin(ssid, password); //Подключение в режиме клиента
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Client mode");// Говорим что мы в режиме клиент
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  } 
    server.on("/", handle_root);
    server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });
  server.begin();
  Serial.println("HTTP server started");
  //dht22 sensor
  Serial.println("DHTxx test!");
  dht.begin();
 
}

void handle_root() {
  float temp;
  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.println(temp);
  } while (temp == 85.0 || temp == (-127.0));
  //dht22
  
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  float hi = dht.computeHeatIndex(f, h);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hi);
  Serial.println(" *F");
  float ti = ((hi-32)/2)+(((hi-32)/2)/10);
  Serial.println(analogRead(0));
  float ur = analogRead(0);
  float u = ((ur*5.7));
  float up =((u*23.8)/1000);
  Serial.println(u);
  
 //bmp180
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");
    
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure()/133.3);
  Serial.println(" mm");
  
  float pressure = (bmp.readPressure()/133.3);
  float temp180 = (bmp.readTemperature());
  
  digitalWrite(led, 1);
  //Веб сервер
  server.send(200, "text/html", "<!DOCTYPE html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta name=\"viewport\" content=\"width=400px\"><title>Wi-Fi ESP8266 Метеостанция</title>"
  "<style type=\"text/css\">body\{background-color: #7D8EE2;color:#FFF;}"
"a {color:#73B9FF;}"
".blockk {border:solid 1px #2d2d2d;text-align:center;background:#0059B3;padding:10px 10px 10px 10px;-moz-border-radius: 5px;-webkit-border-radius: 5px;border-radius: 5px;}"
".blockk{border:double 2px #000000;-moz-border-radius: 5px;-webkit-border-radius: 5px;border-radius: 5px;}"
"</style><style type=\"text/css\" media=\'(min-width: 810px)\'>body\{font-size:18px;}.blockk \{width: 400px;}</style>"
  "<style type=\"text/css\" media=\'(max-width: 800px) and (orientation:landscape)\'>body\{font-size:8px;}</style><meta http-equiv=\"REFRESH\" content=\"60\"></head><body><center><div class=\"blockk\">Мини Метеостанция на ESP8266<br><hr><b>DHT22:</b><br>Температура: "+String(t)+" &deg;C. Влажность (отн.): "+String(h)+" %.<br>Ощущаемая температура: "+String(ti)+" &deg;C.<br><hr><b>BMP180:</b><br>Температура: "+String(temp180)+" &deg;C.<br> Давление(атм.): "+String(pressure)+" мм.рт.ст.<br><hr><b>DS18B20:</b><br>Температура: " +String(temp)+ " °C <br><hr><b>Питание (аккумулятор):</b><br>Напряжение: " +String(u)+ " mV <br>Уровень заряда: " +String(up)+ " % <br><hr></div></center></body></html>");
  digitalWrite(led, 0);
}

void loop(void)
{ 
 server.handleClient(); 
 
}

