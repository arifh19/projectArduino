#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <Ultrasonic.h>
#define echoPin 4 //Echo Pin
#define trigPin 5 //Trigger Pin

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(10,39,52,217);
//IPAddress ip(10,33,107,219);
IPAddress gateway(10,39,52,254);
//IPAddress gateway(10,33,107,254);
IPAddress subnet(255,255,255,0);
IPAddress dns2(10,13,10,13);
IPAddress server(10,33,109,93);
Servo servo1;
Ultrasonic ultrasonic(trigPin,echoPin);
int posisi = 90;
int pinSuhu = A5;
int pinPotensio = A4;
int pinFan = 6;
int pinLDR= A2;
int pinLampu= 3;
int pinLED= 8;
int pinBuzzer = 7;
unsigned int nilai;
String suhu_str;
char suhuC[50];
long suhu;
boolean x= true;
boolean y= true;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  String topik = topic;
  if(topik=="/arifgozi/smartfan/fan"){
    Serial.print("] ");
    for (int i=0;i<length;i++) {
      if((char)payload[i]=='1'){
        x= false;
        digitalWrite(pinLED, HIGH);
        digitalWrite(pinFan, HIGH);
        digitalWrite(pinBuzzer, HIGH);
        delay(500);
        digitalWrite(pinBuzzer, LOW);
      }else if((char)payload[i]=='0'){
        x=true;
        digitalWrite(pinLED, LOW);
        digitalWrite(pinFan, LOW);
        digitalWrite(pinBuzzer, HIGH);
        delay(500);
        digitalWrite(pinBuzzer, LOW);
      }
      Serial.print((char)payload[i]);
    }
  }else if(topik=="/arifgozi/smartfan/lamp1"){
      Serial.print("] ");
      for (int i=0;i<length;i++) {
        if((char)payload[i]=='1'){
          y= false;
          digitalWrite(pinLampu, HIGH);
          digitalWrite(pinBuzzer, HIGH); 
          delay(500);
          digitalWrite(pinBuzzer, LOW);
        }else if((char)payload[i]=='0'){
          y=true;
          digitalWrite(pinLampu, LOW);
          digitalWrite(pinBuzzer, HIGH);
          delay(500);
          digitalWrite(pinBuzzer, LOW);
        }
        Serial.print((char)payload[i]);
      }
  }

  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("desi","hello world");
      // ... and resubscribe
      client.subscribe("/arifgozi/smartfan/fan");
      client.subscribe("/arifgozi/smartfan/lamp1");
      //client.subscribe("/arifgozi/smartfan/door");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void temp(){
  suhu = analogRead(pinSuhu);
  suhu = (suhu/1024.0)*5000;
  suhu = suhu/10;
  delayMicroseconds(200);
  suhu_str = String(suhu);
  suhu_str.toCharArray(suhuC, suhu_str.length()+ 1);
  client.publish("/arifgozi/smartfan/temp",suhuC);
  delay(500);
}

void fan(){
  if(x){
      if(suhu>28){
        digitalWrite(6, HIGH);
        client.publish("/arifgozi/smartfan/fan1","1");
        delay(10);
      }else if(suhu<=28){
        digitalWrite(6, LOW);
        client.publish("/arifgozi/smartfan/fan1","0");
        delay(10);
      }
      delay(200);
  }
}

void door(){
  digitalWrite (trigPin, HIGH); //mengirim suara
  delayMicroseconds(10); //selama 10 mikro detik
  digitalWrite (trigPin, LOW); //berhenti mengirim suara
  float jarak = pulseIn(echoPin, HIGH); //membaca data dan di masukkan ke variabel jarak
  jarak=jarak/1000000; //konversi mikro detik ke detik
  jarak=jarak*330/2; //data mentah di ubah ke dalam meter
  jarak=jarak*100; //mengubah data ke dalam centi meter
  delay(500); //delay 500ms
  if(jarak > 7){
    while(posisi<180){
        servo1.write(posisi);
        posisi++;
        client.publish("/arifgozi/smartfan/door","2");
        delay(10);
    }
    client.publish("/arifgozi/smartfan/door","1");
  }
  else{
    while(posisi>90){
      servo1.write(posisi);
      posisi--;
      client.publish("/arifgozi/smartfan/door","3");
      delay(10);
    }
    client.publish("/arifgozi/smartfan/door","0");
  }
  
}

void lamp(){
  nilai=analogRead(pinLDR);
  nilai=100-nilai;
  if(y){
    if(nilai<=0||nilai>=255){
      analogWrite(pinLampu,0);
      client.publish("/arifgozi/smartfan/lamp","0");
    }else{
      analogWrite(pinLampu,nilai);
      client.publish("/arifgozi/smartfan/lamp","1");
    }
  }
}
void setup()
{
  Serial.begin(9600);

  client.setServer(server, 1883);
  client.setCallback(callback);
  pinMode(pinLED, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinFan, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pinLampu, OUTPUT);
  servo1.attach(9);
  Ethernet.begin(mac, ip, dns2, gateway, subnet);
  // Allow the hardware to sort itself out
  delay(1500);
  //fan(true);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  temp();
  fan();
  door();
  lamp();
  client.loop();
}
