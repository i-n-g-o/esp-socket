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
  Serial.println(err);
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







void printConnectionInfo(remot_info* info) {
  
  for (int i=0; i<server.getMaxClients(); i++) {
    remot_info conn = info[i];  
    // check state
    if (conn.state >= ESPCONN_CONNECT && conn.remote_port > 0) {
      
      Serial.print("[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(conn.state);

      
      Serial.print(" remote_port: ");
      Serial.print(conn.remote_port);
      Serial.print(" ip: ");
      Serial.print(conn.remote_ip[3]);
      
      Serial.println();
    }
  }
}


void printEspConnInfo(struct espconn *pesp_conn_client) {
  if (pesp_conn_client->type == ESPCONN_INVALID) {
    Serial.println("ESPCONN_INVALID");
  } else if (pesp_conn_client->type == ESPCONN_TCP) {
    Serial.println("ESPCONN_TCP");

    esp_tcp* tcp = pesp_conn_client->proto.tcp;

    Serial.print("local port: ");
    Serial.println(tcp->local_port);
    Serial.print("remote port: ");
    Serial.println(tcp->remote_port);


    Serial.print("local ip: ");
    Serial.print(tcp->local_ip[0]);
    Serial.print(".");
    Serial.print(tcp->local_ip[1]);
    Serial.print(".");
    Serial.print(tcp->local_ip[2]);
    Serial.print(".");
    Serial.println(tcp->local_ip[3]);

    Serial.print("remote ip: ");
    Serial.print(tcp->remote_ip[0]);
    Serial.print(".");
    Serial.print(tcp->remote_ip[1]);
    Serial.print(".");
    Serial.print(tcp->remote_ip[2]);
    Serial.print(".");
    Serial.println(tcp->remote_ip[3]);
    
  } else if (pesp_conn_client->type == ESPCONN_UDP) {
    Serial.println("ESPCONN_UDP");

    esp_udp* udp = pesp_conn_client->proto.udp;
  }
}

