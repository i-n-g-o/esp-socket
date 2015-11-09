#include <ESP8266WiFi.h>
#include <ESP8266TCPServer.h>

//------------------------------------------
const char* ssid     = "your-ssid";
const char* password = "your-ssid-password";

// create a server on port 9001
ESP8266TCPServer server(9001);

//------------------------------------------
// CB forward declarations
void onConnectCb(ESP8266Client& client);
void onDisconnectCb();
void onReconnectCb(ESP8266Client& client, sint8 err);
void onDataCb(ESP8266Client& client, char *data, unsigned short length);
void onInfoCb(const char* info);
void onErrorCb(const char* error, sint8 error_num);


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

  
  // set callbacks
  server.onConnected(onConnectCb);
  server.onDisconnected(onDisconnectCb);
  server.onReconnect(onReconnectCb);
  server.onData(onDataCb);

  server.onInfo(onInfoCb);
  server.onError(onErrorCb);

  // connected, start server
  server.start();
}

void loop() {
}


void onConnectCb(ESP8266Client& client)
{
  Serial.println("client connected");

  // send something to the client
  client.send("hello client");
}

void onDisconnectCb()
{
  Serial.println("client disconnected");
}

void onReconnectCb(ESP8266Client& client, sint8 err)
{
  Serial.print("client reconnect. error: ");
  Serial.println(espErrorToStr(err));
  Serial.println(espErrorDesc(err));
}

void onDataCb(ESP8266Client& client, char *data, unsigned short length)
{
  Serial.println("data received:");
  Serial.write(data, length);

  // send something to the client
  client.send("OKAY\r\n");
}


// info callback - internal info
void onInfoCb(const char* info)
{
  Serial.print("info: ");  
  Serial.println(info);
}

// error callback - internal errors arriving here
void onErrorCb(const char* error, sint8 error_num)
{
  Serial.print("error: ");
  Serial.print(error);
  Serial.println(error_num);
}

