#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// #define WIFI_AP "OLAX_MFi_786C"
// #define WIFI_PASSWORD "81118954"
#define WIFI_AP "NBL"
#define WIFI_PASSWORD "1234567890N@ble!1234567890"
//#define WIFI_AP "IOTTEST"
//#define WIFI_PASSWORD "Aspirine"


#define TOKEN "YMSGLJg5WJ93aSnolasQ"

#define smokePin 9 // smoke
#define MAGNETICDOORSWITCH 14  // Door Sensor
#define moisture 10 //soil moisture
#define buzzer 12 // buzzer output

/////////////////////////////////////////////////////////
#include <DHT.h>
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
char thingsboardServer[] = "demo.thingsboard.io";
WiFiClient wifiClient;

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
//ThingsBoard tb(espClient);
float h, f, t,last_h,last_t;
byte Dvalue,smValue,SmokeValue,last_Dvalue,last_smValue,last_SmokeValue,Otemp,Ohumid,last_Otemp,last_Ohumid;

// We assume that all GPIOs are LOW
boolean gpioState[] = {false, false};
long lastMsg=0;
bool warningFLAG=false;
String ErrorMSG="";



void reconnect() {
  byte tt=0,bb=0;
  // Loop until we're reconnected
  while (!client.connected()) {
    delay(100);tt++;
    if(tt>10)return;
    if ( WiFi.status() != WL_CONNECTED) {
    
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        bb++;
        if(bb>10)return;
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("v1/devices/me/rpc/request/+");
      // Sending current GPIO status
      Serial.println("Sending current GPIO status ...");
      
      client.publish("v1/devices/me/telemetry","{\"testdata\":\"12345\"}");

      
      
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}


void setup() {
  delay(100);  
  Serial.begin(115200);
  dht.begin();
  lcd.init();                    
  lcd.backlight();
  //pinMode(GPIO0, INPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(smokePin,INPUT_PULLUP);
  pinMode(MAGNETICDOORSWITCH,INPUT_PULLUP);
  pinMode(moisture,INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  lcd.clear();
  lcd.setCursor(0, 0);
  client.setServer( thingsboardServer, 1883 );
  delay(1000);
  reconnect();
  
}
byte readcounte=10;
byte Dsensor,Ssensor,Smoisture;
bool Errorflag;
String ERRORMSGPRINT;



void loop(){

/////////////////////read DIGITAL IO PINS/////////////////////////////////////////  
readcounte++;
 if(readcounte%8==0) {
  if(Dsensor>5){Dvalue=0;if(readcounte>10&&smValue==1&&SmokeValue==1&&Otemp==1&&Ohumid==1)digitalWrite(buzzer,LOW); }else Dvalue=1; //Door sensor value
  if(Smoisture>5){smValue=0;if(readcounte>120) noTone(buzzer);}else smValue=1;// Flood sensor value
  if(Ssensor>5){SmokeValue=0;if(readcounte>120) noTone(buzzer);}else SmokeValue=1;// Smoke sensor value
  if(t>45){Otemp=0;if(readcounte>120) noTone(buzzer);}else Otemp=1;// Smoke sensor value
  if(h>93){Ohumid=0;;if(readcounte>120) noTone(buzzer);}else Ohumid=1;// Smoke sensor value

 Dsensor =0;
 Ssensor =0;
 Smoisture=0; 
 }
if(digitalRead(MAGNETICDOORSWITCH))Dsensor++; //for 8 times sampling
if(!digitalRead(smokePin))Ssensor++;
if(!digitalRead(moisture))Smoisture++;
  
/////////////////////read DIGITAL IO PINS///////////////////////////////////////// 

/////////////////////////////read DHT11///////////////////////////////////////////
if(readcounte>240) {
    readcounte=0;
 h = dht.readHumidity();// Read temperature as Celsius (the default)
 t = dht.readTemperature();// Read temperature as Fahrenheit (isFahrenheit = true)
 f = dht.readTemperature(true);// Check if any reads failed and exit early (to try again).
  if((Dvalue==0||smValue==0||SmokeValue==0))lcd.clear();
  else{digitalWrite(buzzer,LOW); }
  Errorflag=!Errorflag;
  if((Dvalue==1&&smValue==1&&SmokeValue==1)||(Errorflag)){
 if (isnan(h) || isnan(t) ) {Serial.println(F("Failed to read from DHT sensor!"));
                                        lcd.setCursor(0, 0); lcd.print("DTH Sensor ERROR");}
 else{    
    lcd.setCursor(0, 0);
    lcd.print("Temp = ");
    lcd.print(t);lcd.print("    ");
    lcd.setCursor(0,1);
    lcd.print("Humidity = ");
    lcd.print(h);lcd.print("    ");
  }}
  else{lcd.setCursor(0, 0); lcd.print(ERRORMSGPRINT);Serial.println(ERRORMSGPRINT);}

  if(smValue==0||SmokeValue==0||t>45||h>94)tone(buzzer, 1000);
  else if(Dvalue==0)digitalWrite(buzzer,HIGH);
}
  
/////////////////////////////read DHT11///////////////////////////////////////////

/////////////////////////chack for chsnges//////////////////////////////////////////////
if((last_h!=h)||(last_t!=t)||(Dvalue!=last_Dvalue)||(smValue!=last_smValue)||(SmokeValue!=last_SmokeValue)){

  if(SmokeValue==0)lcdPrint("Smoke Detected");
  else if(smValue==0)lcdPrint("Flood detected");
  else if(Otemp==0)lcdPrint("Over temerature!");
  else if(Ohumid==0)lcdPrint("Over humidity!");  
  else if(Dvalue==1&&last_Dvalue==0)lcdPrint("Door Closed!");
  else if(Dvalue==0)lcdPrint("Door Open!");
 /////////////////////////chack for chsnges////////////////////////////////////////////// 
DynamicJsonDocument doc(256);
char out[256];
doc["temp"] = t;
doc["humid"]   = h;
doc["DoorSensor1"] = Dvalue;
doc["SoilMoisture1"]   = smValue;
doc["SmokeSensor1"] = SmokeValue;
serializeJson(doc, out);
Serial.println( out );
if (!client.connected()) {reconnect(); }
  client.publish("v1/devices/me/telemetry", out);   
  last_t=t; last_h=h;last_Dvalue=Dvalue;last_smValue=smValue;last_SmokeValue=SmokeValue;
  }  
  client.loop();
delay(10);
}

void lcdPrint(String MGS){  
  ERRORMSGPRINT=MGS;  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(MGS);
}
