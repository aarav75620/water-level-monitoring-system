#include <ESP8266WiFi.h>

#define TRIG_PIN 14  // GPIO14 (D5)
#define ECHO_PIN 12  // GPIO12 (D6)
#define GREEN_LED 4  // GPIO4 (D2)
#define RED_LED 0    // GPIO0 (D3)
#define BUZZER 13    // GPIO13 (D7)

const char* ssid = "Arka 4floor";
const char* password = "arka@2023";

WiFiServer server(80);
const int tankHeight = 40;  // Tank height in cm
const int emptyLevel = 5;   // Distance when tank is empty (in cm)

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  long duration;
  float distance, waterPercentage;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  waterPercentage = ((tankHeight - distance) / (tankHeight - emptyLevel)) * 100;
  if (waterPercentage < 0) waterPercentage = 0;
  if (waterPercentage > 100) waterPercentage = 100;

  Serial.print("Water Level: ");
  Serial.print(waterPercentage);
  Serial.println("%");

  // Control LEDs and buzzer based on water level
  if (waterPercentage >= 85) {  // High water level
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, HIGH);
  } else if (waterPercentage <= 20) {  // Low water level
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
  } else {  // Normal level
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
  }

  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client Connected");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<!DOCTYPE html><html><head><title>Water Level Monitor</title>");
    client.println("<script type='text/javascript' src='https://cdn.fusioncharts.com/fusioncharts/latest/fusioncharts.js'></script>");
    client.println("<script type='text/javascript' src='https://cdn.fusioncharts.com/fusioncharts/latest/themes/fusioncharts.theme.fusion.js'></script>");
    client.println("<script>");
    
    client.println("function renderChart(waterLevel) {");
    client.println("  FusionCharts.ready(function() {");
    client.println("    var myChart = new FusionCharts({");
    client.println("      type: 'cylinder',");
    client.println("      renderAt: 'chart-container',");
    client.println("      width: '250',");
    client.println("      height: '400',");
    client.println("      dataFormat: 'json',");
    client.println("      dataSource: {");
    client.println("        chart: {");
    client.println("          theme: 'fusion',");
    client.println("          caption: 'Real-Time Tank Level Monitoring',");
    client.println("          lowerLimit: '0',");
    client.println("          upperLimit: '100',");
    client.println("          lowerLimitDisplay: 'Empty',");
    client.println("          upperLimitDisplay: 'Full',");
    client.println("          numberSuffix: '%',");
    client.println("          chartBottomMargin: '45',");
    client.println("          valueFontSize: '11',");
    client.println("          showValue: '1'");
    client.println("        },");
    client.println("        value: waterLevel");
    client.println("      }");
    client.println("    }).render();");
    client.println("  });");
    client.println("}");
    
    client.println("</script>");
    
    client.println("</head><body onload='renderChart(" + String(waterPercentage) + ")'>");
    client.println("<h2>Water Level Monitoring</h2>");
    client.println("<div id='chart-container'></div>");
    client.println("<p>Water Level: <strong>" + String(waterPercentage) + "%</strong></p>");
    client.println("</body></html>");
    client.println();
    client.stop();
    Serial.println("Client Disconnected");
  }

  delay(3000);
}
