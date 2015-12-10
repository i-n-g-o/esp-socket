#include <ESP8266WiFi.h>
#include <ESP8266Client.h>

//------------------------------------------
const char* ssid     = "your-ssid";
const char* password = "your-ssid-password";


// create a UDP client
ESP8266Client client("ip-of-server", 8080, UDP);

//------------------------------------------
// CB forward declarations
void onDataCb(ESP8266Client& client, char *data, unsigned short length);
void onConnectCb(ESP8266Client& client);
void onDisconnectCb();
void onReconnectCb(ESP8266Client& client, sint8 err);


//------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  // set callback functions
  client.onData(onDataCb);
  client.onConnected(onConnectCb);
  client.onDisconnected(onDisconnectCb);
  client.onReconnect(onReconnectCb);
}

void loop() {

  boolean result;

  int bufsize = Serial.available();
  uint8 buf[bufsize];
  memset(buf, 0, bufsize);
  
  // read from serial
  for(int i=0; i<bufsize; i++) {
    int c = Serial.read();
    if (c >= 0) {
      buf[i] = (uint8)c;
    } else {
      bufsize = i;
      break;
    }
  }
  
  if (client.isConnected()) {

    if (bufsize > 0) {
      Serial.print("sending buffer [");
      Serial.print(bufsize);
      Serial.print("]: ");

      for (int i=0; i<bufsize; i++) {
        Serial.print(buf[i]);
        Serial.print(",");       
      }
      Serial.println();

      // blocking, waits until data is sent
      sint8 res = client.send(buf, bufsize);
      if (res != ESPCONN_OK) {
        Serial.print("error sending: ");
        Serial.println(res);
      }
    }
   
  } else {

    if (bufsize > 0) {
      Serial.print("lost data: ");
      Serial.write(buf, bufsize);
    }
    
    // connect
    result = client.connect();
  }

  delay(1);
}


//------------------------
//------------------------
// general callbacks
void onDataCb(ESP8266Client& client, char *data, unsigned short length)
{
  Serial.write(data, length);
}

//------------------------
//------------------------
// TCP callbacks
void onConnectCb(ESP8266Client& client)
{
  Serial.println("connected to server");  
}

void onDisconnectCb()
{
  Serial.println("disconnected from server");
}

void onReconnectCb(ESP8266Client& client, sint8 err)
{
  Serial.print("reconnect CB: ");
  Serial.println(espErrorToStr(err));
  Serial.println(espErrorDesc(err));
}
