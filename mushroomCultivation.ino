
#include <Wire.h>
#include <BH1750.h>

#include "DHT.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

BH1750 lightMeter(0x23);


//*************************WIFI*******************************************
const char *ssid = "Ariful"; //ENTER YOUR WIFI SETTINGS
const char *password = "ninanur01";
//************************************************************************

String postData ; // post array that will be send to the website
String link = "http://192.168.31.93/test/"; //computer IP or the server domain | Go to cmd & run "ipconfig" command and get your IP from  "IPv4 Address" section and use it here
//************************************************************************

int count = 0;//cultivation mode count


//######******************#**********Start: Parameters**********#******************######

//Ref: IOP Conf. Series: Journal of Physics: Conf. Series 1019 (2018) 012053 doi :10.1088/1742-6596/1019/1/012053

int lightHigh = 300; //unit lux [BUZZ Alert]
int lightLow = 50; //unit lux [BUZZ Alert]

int tempHigh = 29;//unit oC [FAN]
int tempLow = 26;//unit oC [FAN]

int humidityHigh = 90;//unit % RH [AirPump]
int humidityLow = 80;//unit % RH [AirPump]


//######******************#**********End: Parameters**********#******************######

void setup(){

  Serial.begin(115200);

  pinMode(D5, OUTPUT); //BUZZER 
  pinMode(D6, OUTPUT); //SWITCH
  pinMode(D1, OUTPUT); //FAN
  pinMode(D7, INPUT); //BUTTON
  //#################DHT11--Start--####################################
  Serial.println(F("DHTxx test!"));
  dht.begin();
  //#################DHT11--End--####################################
  

  //#################LIGHT--Start--####################################
  Wire.begin(D3, D4);//LightPin
  // begin returns a boolean that can be used to detect setup problems.
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else {
    Serial.println(F("Error initialising BH1750"));
  }
  //#################LIGHT--End--####################################

  //---------------------------------------------

    connectToWiFi();

  //---------------------------------------------
}


void loop() {
  
  // Wait a few seconds between measurements.
  delay(2000);
  //#################LIGHT--Start--####################################
  
  float lux = lightMeter.readLightLevel();
  
  //#################LIGHT--End--####################################


  //check if there's a connection to WiFi or not
    if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
    }
  //---------------------------------------------

  
  cultivationMode();
  

  //#################DHT11--Start--##################################
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  //#################DHT11--End--####################################


  //#################PrintCONSOLE--START--####################################

  Serial.print(F("\nHumidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F "));
  Serial.print("  Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(1000);
  Serial.print("index: ");
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F "));
  Serial.print(lux);
  Serial.println(F("lx "));
  
  //#################PrintCONSOLE--End--####################################

  //#################sendParametersToMainLogicFunction--Start--####################################
  mainLogic(t,h,lux);
  //#################sendParametersToMainLogicFunction--Start--####################################
  

  //#################Buzz--Start--####################################
  if(lux >= 300){ 
      
      //digitalWrite(D6, HIGH);    // switch on    
      //digitalWrite(D1, HIGH);    // switch on    
       
      digitalWrite(D5, HIGH);   // buzz & switch on
      delay(1000);              // wait for a second
      digitalWrite(D5, LOW);    // buzz & switch off
                          
      SendLightAlert();
  }
  else{
    //digitalWrite(D6, LOW);    // switch off  
    //digitalWrite(D1, LOW);    // switch off  
  }
  
  //#################Buzz--End--####################################
  //digitalWrite(D6, LOW);    // switch on    
  //delay(500);      
  //digitalWrite(D6, HIGH);    // switch off  
  //delay(500);    
}

//********************Start: MainLogic******************

void updateParam(){

  //Ref:  https://improvemushroomcultivation.com/12-important-growing-factors-mushroom-farming
  
  /*
    lightLow = ;
    lightHigh = ;
  */

  if(count == 0){

    Serial.print("Default");

    tempLow = 26;
    tempHigh = 29;

    humidityLow = 80;
    humidityHigh = 90;
    
  }else if(count == 1){
    //*Yellow Oyster
    //temp:21-29|hum:90-95
    Serial.print("*Yellow Oyster");

    tempLow = 21;
    tempHigh = 29;

    humidityLow = 90;
    humidityHigh = 95;
    
  }else if(count == 2){
    //*Tree Oyster
    //temp:10-21|hum:80-85
    Serial.print("*Tree Oyster");

    tempLow = 10;
    tempHigh = 21;

    humidityLow = 80;
    humidityHigh = 85;
    
  }else{
    //*Indian Oyster
    //temp:18-24|hum:85-90
     Serial.print("*Indian Oyster");

    tempLow = 18;
    tempHigh = 24;

    humidityLow = 85;
    humidityHigh = 90;
    
  }
  
}

void mainLogic(float temp,float humi,float lx){

  updateParam();

  if(temp > tempHigh && humi > humidityHigh){

    digitalWrite(D6, LOW);    // switch off   
    digitalWrite(D1, HIGH);   // switch on   
    
  }else if(temp > tempHigh && humi < humidityLow){

    digitalWrite(D6, HIGH);    // switch on   
    digitalWrite(D1, HIGH);   // switch on   
  }else if(temp < tempLow && humi > humidityLow){

    digitalWrite(D6, LOW);    // switch off   
    digitalWrite(D1, HIGH);   // switch off   
  }else if(humi < humidityLow){

    digitalWrite(D6, HIGH);    // switch on   
    digitalWrite(D1, HIGH);   // switch on   
  }else if(humi > humidityHigh){

    digitalWrite(D6, LOW);    // switch off
    digitalWrite(D1, HIGH);   // switch on   
  }else if(temp > tempHigh){

    digitalWrite(D6, LOW);    // switch off   
    digitalWrite(D1, HIGH);   // switch on   
  }
  else{
    digitalWrite(D6, LOW);    // switch off  
    digitalWrite(D1, LOW);    // switch off  
  }
  
}

//********************End: MainLogic******************

//********************connect to the WiFi******************
void connectToWiFi(){
  WiFi.mode(WIFI_OFF); //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Connected");
  
  Serial.print(F("Connected \n"));
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //IP address assigned to your ESP

}

//************send the Lifgt_Status to the web API*************
void SendLightAlert(){

  HTTPClient http; //Declare object of class HTTPClient
  //Post Data
  postData = "light_status=true"; // JSON
  // Post methode
  
  http.begin(link); //initiate HTTP request, put your Website URL or Your Computer IP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  
  int httpCode = http.POST(postData); //Send the request
  String payload = http.getString(); //Get the response payload
 
  Serial.println(httpCode); //Print HTTP return code
  Serial.println("API Response: ");
  Serial.println(payload); //Print request response payload
  Serial.println("API Request: ");
  Serial.println(postData); //Post Data
  
  delay(1000);
  
  postData = "";
  http.end(); //Close connection
}

void cultivationMode(){
  
  if(digitalRead(D7)==HIGH){
    count++;
    if(count == 1){
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
    }
    else if(count == 2){
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
      delay(200);
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
    }
    else if(count == 3){
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
      delay(200);
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
      delay(200);
      digitalWrite(D5, HIGH); //BUZZ
      delay(200);
      digitalWrite(D5, LOW); //BUZZ
    }
    else{
      count = 0;
    }
    
  }
  else{
    digitalWrite(D5, LOW); //BUZZ
  }

  
  
}
