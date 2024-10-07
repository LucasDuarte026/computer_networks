// Bibliotecas necessárias para o Projeto
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

float temperatura, umidade, pressao, altitude;

// Nome da rede e senha para conexão
const char* ssid = "Wifi_casa_2G";
const char* senha = "Ledanina";

WebServer server(80);

// Informações de acesso para rede de internet / IP Fixo
IPAddress local_IP(192, 168, 0, 100);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

void setup() {
  Serial.begin(115200);
  delay(100);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  bme.begin(0x76);

  Serial.println("Conectando a ");
  Serial.println(ssid);
  //Conecta à Rede Wifi indicada anteriormente
  WiFi.begin(ssid, senha);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado ..!");
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}

void handle_OnConnect() {
  temperatura = bme.readTemperature();
  umidade = bme.readHumidity();
  pressao = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  server.send(200, "text/html", SendHTML(temperatura, umidade, pressao, altitude));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperatura, float umidade, float pressao, float altitude) {
  String ptr = "<!DOCTYPE html><html lang=\"pt-br\">\n";
  ptr += "<head>\n";
  ptr += "<meta charset=\"UTF-8\">\n";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">\n";
  ptr += "<title>Projeto Redes de Computadores</title>\n";
  ptr += "<link href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css\" rel=\"stylesheet\">\n";
  ptr += "<style>\n";
  ptr += "body {background-color: #003D3D; color: #ffffff; font-family: 'Arial', sans-serif;}\n";
  ptr += ".navbar {background-color: #006666; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);}\n";
  ptr += ".hero {text-align: center; padding: 60px 0; background-color: #007777; color: #ffffff;}\n";
  ptr += ".hero h1 {font-size: 48px; font-weight: bold; letter-spacing: 1px;}\n";
  ptr += ".hero p {font-size: 20px; margin-bottom: 40px;}\n";
  ptr += ".info-blocks {padding: 40px 0;}\n";
  ptr += ".info-blocks .card {background-color: #009999; border: 2px solid #000000; margin-bottom: 30px; padding: 20px; text-align: center; color: #ffffff; font-size: 24px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);}\n";
  ptr += ".footer {background-color: #006666; color: #ffffff; padding: 20px 0; font-size: 14px;}\n";
  ptr += ".footer .container {display: flex; justify-content: space-between; align-items: center;}\n";
  ptr += ".footer .text-muted {color: rgba(255, 255, 255, 0.6);}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";

  // Navigation Bar
  ptr += "<nav class=\"navbar navbar-expand-lg navbar-dark bg-dark\">\n";
  ptr += "<a class=\"navbar-brand\" href=\"#\">Projeto Redes de Computadores</a>\n";
  ptr += "<button class=\"navbar-toggler\" type=\"button\" data-toggle=\"collapse\" data-target=\"#navbarNav\" aria-controls=\"navbarNav\" aria-expanded=\"false\" aria-label=\"Toggle navigation\">\n";
  ptr += "<span class=\"navbar-toggler-icon\"></span>\n";
  ptr += "</button>\n";
  
  ptr += "</nav>\n";

  // Hero Section
  ptr += "<div class=\"hero\">\n";
  ptr += "<h1>Projeto 01 - Redes de Computadores</h1>\n";
  ptr += "<p>Monitoramento das condições ambientais em tempo real</p>\n";
  ptr += "</div>\n";

  // Information Blocks
  ptr += "<div class=\"container info-blocks\">\n";
  ptr += "<div class=\"row justify-content-center\">\n";
  ptr += "<div class=\"col-md-3\">\n";
  ptr += "<div class=\"card\">\n";
  ptr += "<div class=\"card-body\">\n";
  ptr += "<p>Temperatura: " + String(temperatura) + "&deg;C</p>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";

  ptr += "<div class=\"col-md-3\">\n";
  ptr += "<div class=\"card\">\n";
  ptr += "<div class=\"card-body\">\n";
  ptr += "<p>Umidade: " + String(umidade) + "%</p>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";

  ptr += "<div class=\"col-md-3\">\n";
  ptr += "<div class=\"card\">\n";
  ptr += "<div class=\"card-body\">\n";
  ptr += "<p>Pressão: " + String(pressao) + " hPa</p>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";

  ptr += "<div class=\"col-md-3\">\n";
  ptr += "<div class=\"card\">\n";
  ptr += "<div class=\"card-body\">\n";
  ptr += "<p>Altitude: " + String(altitude) + " m</p>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";

  // Footer Section
  ptr += "<footer class=\"footer mt-auto py-3 bg-dark text-white text-center\">\n";
  ptr += "<div class=\"container\">\n";
  ptr += "<span class=\"text-muted\">Simpsons - © 2024</span>\n";
  ptr += "</div>\n";
  ptr += "</footer>\n";

  ptr += "<script src=\"https://code.jquery.com/jquery-3.5.1.slim.min.js\"></script>\n";
  ptr += "<script src=\"https://cdn.jsdelivr.net/npm/@popperjs/core@2.5.2/dist/umd/popper.min.js\"></script>\n";
  ptr += "<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js\"></script>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
