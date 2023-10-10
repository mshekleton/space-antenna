#include <ETH.h>
#include <WebServer.h>
#include <ESPmDNS.h>

WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Hello from wESP32!\n");
}

void handleNotFound() {
  server.send(404, "text/plain", String("No ") + server.uri() + " here!\n");
}

void handleDataEcho() {
    if(server.hasArg("plain") == false){ 
        Serial.println("No data received");
        Serial.println("Headers:");
        for (uint8_t i=0; i<server.headers(); i++){
          Serial.println(server.headerName(i) + ": " + server.header(i));
        }
        server.send(422, "text/plain", "Data unit expected."); 
        return;
    }

    String message = server.arg("plain");
    Serial.print("Received data: ");
    Serial.println(message);
    server.send(200, "text/plain", message);
}




void setup(){
  Serial.begin(9600);
  
  ETH.begin(0, -1, 16, 17, ETH_PHY_RTL8201);
  
  while (ETH.localIP().toString() == "0.0.0.0") {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(ETH.localIP());
  
  MDNS.begin("wesp32demo");

  server.on("/", handleRoot);
  server.on("/echo", HTTP_POST, handleDataEcho);
  server.onNotFound(handleNotFound);

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop(){
  server.handleClient();
}
