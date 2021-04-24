#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SHT3x.h>

#define DELAY_IN_SECONDS 10
#define NUM_OF_PROBES 30
#define THINGSPEAK_SERVER "api.thingspeak.com"
#define WRITE_API_KEY ""

SHT3x Sensor;
WiFiClient client;

const char *ssid =  "";     // replace with your wifi ssid and wpa2 key
const char *pass =  "";

struct sensorData {
  float temperature;
  float humidity;
};

void connectToWiFi(const char* ssid, const char* pass) {
  Serial.printf("\nConnecting to %s", ssid);
  
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected with IP: ");
  Serial.println(WiFi.localIP());
}

sensorData* getSensorData(int numOfProbes) {

  sensorData* data = (sensorData*) malloc(sizeof *data);

  float temp = 0;
  float hum = 0;

  for (int i = 0; i < numOfProbes; i++) {
    Sensor.UpdateData();
    temp += Sensor.GetTemperature();
    hum += Sensor.GetRelHumidity();
    if (i < (numOfProbes - 1)) {
      delay(DELAY_IN_SECONDS*1000);
    }
  }
  data->temperature = temp / numOfProbes;
  data->humidity = hum / numOfProbes;
  return data;
}

void printData(sensorData* data) {
  Serial.print("Temperature: ");
  Serial.print(data->temperature);
  Serial.println("C");
  Serial.print("Humidity: ");
  Serial.print(data->humidity);
  Serial.println("%");
}

void sendDataToServer(sensorData* data) {
  String server = THINGSPEAK_SERVER;
  String apiKey = WRITE_API_KEY;
  if (client.connect(server, 80)) {
    String postStr = apiKey + "&field1=" + data->temperature + "&field2=" + data->humidity + "\r\n\r\n";
        
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+ apiKey +"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
  
}
 
void setup() 
{
  Serial.begin(9600);
  delay(100);
  connectToWiFi(ssid, pass);
  Sensor.Begin();
}
 
void loop() 
{      
  sensorData* data = getSensorData(NUM_OF_PROBES);
  printData(data);
  sendDataToServer(data);
  free(data);
  
}
