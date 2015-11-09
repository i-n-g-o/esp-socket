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

  
  // connected, start server
  server.onConnected(onConnectCb);
  server.onDisconnected(onDisconnectCb);
  server.onReconnect(onReconnectCb);
  server.onData(onDataCb);

  // connected, start server
  sint8 err = server.start();
  if (err != ESPCONN_OK) {
    Serial.print("could not start server: ");
    Serial.println(espErrorToStr(err));
    Serial.println(espErrorDesc(err));
  }
}

void loop() {

  // get available bytes
  int bufsize = Serial.available();
  uint8 buf[bufsize];

  memset(buf, 0, bufsize);
  
  // read from serial
  for(int i=0; i<bufsize; i++) {
    int c = Serial.read();
    if (c >= 0) {
      buf[i] = (uint8)c;
    } else {
      // error? break
      bufsize = i;
      break;
    }
  }

  if (bufsize > 0) {
    // send to all clients
    sint8 res = server.sendAll((uint8*)buf, bufsize);
    if (res != ESPCONN_OK) {
      Serial.println("some error sending data");
    }
  }
  
}


void onConnectCb(ESP8266Client& client)
{
  Serial.println("client connected");
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
  
  sint8 err = client.send("OKAY\r\n");
  if (err != ESPCONN_OK) {
    Serial.print("onDataCb:: could not send data: ");
    Serial.println(espErrorToStr(err));
    Serial.println(espErrorDesc(err));
  }
}
