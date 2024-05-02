#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* FIREBASE_HOST ="dht11-f8fe7-default-rtdb.firebaseio.com/";
const char* FIREBASE_AUTH="AIzaSyDIyRrLe3oo3yomPvyaq_dXESagra71WDw";
const char* databaseURL="https://dht11-f8fe7-default-rtdb.firebaseio.com/ngoai_troi.json";

#define MOTION_SENSOR_PIN  19  
#define trigPin 5
#define echoPin 18
#define DHT_PIN  4
#define ldr_PIN  34
#define led1 15
#define led2 16
#define led3 21
const float gama = 0.7;
const float rl10 = 50;
int nilaiLDR=0;
#define SOUND_SPEED 0.034
long duration;
float distanceCm;

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
  pinMode(2, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode (led1, OUTPUT);
  pinMode (led2, OUTPUT);
  pinMode (led3, OUTPUT);

}

void loop(){
  delay(10);
   nilaiLDR = analogRead(ldr_PIN);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;

  // bien doc du lieu
  float doAm = dht.readHumidity();
  float nhietDo = dht.readTemperature();
   if (nhietDo > 30.0 ) {
    digitalWrite(led1, HIGH);
  } else {
    digitalWrite(led1, LOW);
    }

  if (nhietDo > 60.0 ) {
    digitalWrite(led2, HIGH);
  } else {
    digitalWrite(led2, LOW);
  } 
  
  
  // kiem tra du lieu xem co doc thanh cong kh( nan = error)
  // thanh cong dong goi sang goi chuoi JSON

  nilaiLDR = map(nilaiLDR, 4095, 0, 1024, 0); //mengubah nilai pembacaan sensor LDR dari nilai ADC arduino menjadi nilai ADC ESP32
  float voltase = nilaiLDR / 1024.*5;
  float resistansi = 2000 * voltase / (1-voltase/5);
  float kecerahan = pow(rl10*1e3*pow(10,gama)/resistansi,(1/gama));
  if (kecerahan <200 ) {
    digitalWrite(led3, HIGH);
  } else {
    digitalWrite(led3, LOW);
  } 
  
  Serial.print("lux = ");
  Serial.println(kecerahan);
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("LED State: ");  
  Serial.println(digitalRead(led3));

  Serial.printf("do am: %.2f%% , nhiet do: %.2fC\n", doAm, nhietDo);



  // send data to firebase = protocol HTTP
  String data= String("{\"nhiet do\":") + String(nhietDo) + String(",\"do am\":") + String(doAm) +  String(",\"do sang\":") + String(kecerahan)+ String("}");
  HTTPClient http;
  http.begin(databaseURL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", FIREBASE_AUTH);

  int httpResponseCode = http.PUT(data);

  if(httpResponseCode > 0){
    Serial.print("Data sent successfully, respon code: ");
    Serial.println(httpResponseCode);
  }
  else{
    Serial.print("error sending data, response code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  delay(30);
}