#include <ESP8266WiFi.h>
#include <ESP8266TCPServer.h>

const char* ssid     = "your-ssid";
const char* password = "your-ssid-password";

// create a server on port 9001
ESP8266TCPServer server(9001);

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
  server.start();
}

void loop() {

  // stop server after 6 seconds
  delay(5000);
  // inform clients
  server.sendAll("going to stop in a second.");  
  delay(1000);

  // stop server
  Serial.print("stopping server...");  
  sint8 err = server.stop();
  if (err != ESPCONN_OK) {
    // could not stop server
    Serial.print("error: could not stop server: ");
    Serial.println(espErrorToStr(err));
    Serial.println(espErrorDesc(err));
  } else {
    Serial.println("OK");
  }

  // wait 5 seconds
  delay(5000);
  Serial.print("starting server...");  
  err = server.start();
  if (err != ESPCONN_OK) {
    // could not start server
    Serial.print("error: could not start server: ");
    Serial.println(espErrorToStr(err));
    Serial.println(espErrorDesc(err));
  } else {
    Serial.println("OK");
  }
  
}

