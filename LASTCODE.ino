#include "FirebaseESP8266.h" //파이어베이스 라이브러리
#include <ESP8266WiFi.h> //ESP8266와이파이 라이브러리
#include <SimpleTimer.h> //심플타이머 라이브러리
#include <DHT11.h> //온습도센서 라이브러리

#define FIREBASE_HOST ""                                              // 파이어베이스 데이터베이스 주소
#define FIREBASE_AUTH ""                                              // 파이어베이스 데이터베이스 비밀번호
#define WIFI_SSID ""                                                  // 공유기 ID
#define WIFI_PASSWORD ""                                              // 공유기 

int dht = 12; //D6번
DHT11 dht11(dht);
float temp, humi; //온도 습도
int get_water;
String pump;

FirebaseData firebaseData;

SimpleTimer TH1; //온습도 쓰레드
SimpleTimer WOF; //수위레벨센서
SimpleTimer WATER; // 물주기
int pin1 = 2; //D4
int pin2 = 14; //D5

void setup() {
  Serial.begin(115200); //115200으로 통신
  //-----------------------------------
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);

  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
  //------------------------------------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300); //와이파이 접속
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  TH1.setInterval(10000, fn1);
  WOF.setInterval(1000, fn2);
  WATER.setInterval(1000, fn3);
   

}

void loop() {
  TH1.run();
  WOF.run();
  WATER.run();

}
void fn1(){
  String readTemp, readHumi;
  dht11.read(humi, temp);
  readTemp = String(temp);
  readHumi = String(humi);

  Firebase.setString(firebaseData, "/water/temp", readTemp);
  Firebase.setString(firebaseData, "/water/humi", readHumi);

  delay(1500);
}
void fn2(){
  get_water = analogRead(A0);

  Serial.print("water");
  Serial.println(get_water);

  if(Firebase.getString(firebaseData, "/water/level")) {
    String level = firebaseData.stringData();
    Serial.print("Firebase level : ");
    Serial.println(level);

      if(get_water>10&&get_water<1025) {         // 물 없음:  get_water>10&&get_water<1025 범위는 테스트 해보고 조절 할 것
            Serial.print("Now water state: ");
            Serial.println("N");

            if(level=="Y") {
                if (Firebase.setString(firebaseData, "/water/level", "N")) {
                    Serial.println("PUSH WATER STATE PASSED");
                    Serial.println("PATH: " + firebaseData.dataPath());
                    Serial.println("VALUE: " + firebaseData.stringData());
                    Serial.println();                           
                } else {
                    Serial.println("PUSH WATER STATE FAILED");
                    Serial.println("REASON: " + firebaseData.errorReason());  
                }
            } 
        } else if(get_water<10&&get_water>0) {     // 물 있음
            Serial.print("Now water state: ");
            Serial.println("Y");  

            if(level=="N") {
                if (Firebase.setString(firebaseData, "/water/level", "Y")) {
                        Serial.println("PUSH WATER STATE PASSED");
                        Serial.println("PATH: " + firebaseData.dataPath());
                        Serial.println("VALUE: " + firebaseData.stringData());
                        Serial.println();                           
                } else {
                    Serial.println("PUSH WATER STATE FAILED");
                    Serial.println("REASON: " + firebaseData.errorReason());  
                }  
            }
        }         
    } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
    }    
}

void fn3() {
  if(Firebase.getString(firebaseData, "/water/pump")) {
        pump = firebaseData.stringData(); 
        Serial.println(pump);             // \"N\"

        pump = pump.charAt(2);
        
        Serial.println(pump);

        if(pump == "Y") {
          Serial.println("water on");
           digitalWrite(pin1, HIGH);
           digitalWrite(pin2, LOW);
           delay(2000);  

          
        }else if(pump == "N"){
          Serial.println("water off");
           digitalWrite(pin1, LOW);
           digitalWrite(pin2, LOW);
           delay(2000);
        }
        
  } else {
    Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
  }
}
