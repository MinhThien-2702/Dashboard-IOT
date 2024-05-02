#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* FIREBASE_HOST ="dht11-f8fe7-default-rtdb.firebaseio.com/";
const char* FIREBASE_AUTH="AIzaSyDIyRrLe3oo3yomPvyaq_dXESagra71WDw";
const char* databaseURL="https://dht11-f8fe7-default-rtdb.firebaseio.com/phongngu.json";
#define MOTION_SENSOR_PIN  19  
#define DHT_PIN  4
#define ldr_PIN  34
#define led1 15
#define led2 16
#define led3 21
const float gama = 0.7;
const float rl10 = 50;
int nilaiLDR=0;

DHT dht(DHT_PIN, DHT22);


void setup(){

  // mở cổng giao tiếp serial để mở thông báo bên màn hình
  Serial.begin(9600);
  // kết nối wifi
  WiFi.begin(ssid, password);

  // kiểm tra wifi
  while(WiFi.status() != WL_CONNECTED){

    Serial.println("Connecting to wifi.........");
  }

  Serial.println("Connected to wifi");
  // bat dau cho cam bien doc
  dht.begin();
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode (led1, OUTPUT);
  pinMode (led2, OUTPUT);
  pinMode (led3, OUTPUT);

}

void loop(){
  delay(10);
   nilaiLDR = analogRead(ldr_PIN);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  
  // Calculate the distance

  // bien doc du lieu
  float doAm = dht.readHumidity();
  float nhietDo = dht.readTemperature();
  
  
  // kiem tra du lieu xem co doc thanh cong kh( nan = error)
  // thanh cong dong goi sang goi chuoi JSON
  int motionStateCurrent  = digitalRead(MOTION_SENSOR_PIN);
  
  nilaiLDR = map(nilaiLDR, 4095, 0, 1024, 0); //mengubah nilai pembacaan sensor LDR dari nilai ADC arduino menjadi nilai ADC ESP32
  float voltase = nilaiLDR / 1024.*5;
  float resistansi = 2000 * voltase / (1-voltase/5);
  float kecerahan = pow(rl10*1e3*pow(10,gama)/resistansi,(1/gama));

  
  Serial.print("lux = ");
  Serial.println(kecerahan);
 

  Serial.printf("do am: %.2f%% , nhiet do: %.2fC\n", doAm, nhietDo);

  // send data to firebase = protocol HTTP
  String data= String("{\"nhiet do\":") + String(nhietDo) + String(",\"do am\":") + String(doAm) + String(",\"chuyen dong\":") + String(motionStateCurrent) +  String(",\"do sang\":") + String(kecerahan)+ String("}");
  HTTPClient http;
  http.begin(databaseURL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", FIREBASE_AUTH);
  int firebase_response_code = http.GET();
  if (firebase_response_code != HTTP_CODE_OK) {
      Serial.println("Không thể lấy dữ liệu từ Firebase. Mã lỗi: " + String(firebase_response_code));
    } else {
      String firebase_response = http.getString();

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, firebase_response);
      
      int den = doc["thietbi"]["den"];
      int rem = doc["thietbi"]["rem"];
      int maylanh = doc["thietbi"]["maylanh"];

    
      digitalWrite(led1,den); 
      digitalWrite(led2,maylanh);
      digitalWrite(led3,rem);


    }
  http.end();
  delay(30);
}