#include <ESP8266WiFi.h>
#include <ESP8266Client.h>

//------------------------------------------
const char* ssid     = "your-ssid";
const char* password = "your-ssid-password";

// create a TCP client
ESP8266Client client("ip-to-server", 9002);

unsigned long now = 0;

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

  if (client.isConnected()) {
    Serial.println("sending to server");
    client.send("ping\r\n");
    delay(1000);
  } else {
    // connect
    boolean res = client.connect();
    if (!res) {
//      Serial.println("could not connect");
    }    
  }

  delay(10);
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
