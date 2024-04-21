#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Optus_B8E72A";
const char* password = "corns39526ys";

const char* mqtt_server = "mqtt-dashboard.com";
const int mqtt_port = 1883;
const char* topic = "SIT210/wave"; 

const int trigPin = 3;
const int echoPin = 2;
const int ledPin = 10;

WiFiClient espClient;
PubSubClient client(espClient);

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void connectMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT Broker...");
    if (client.connect("ArduinoWaveDetector")) {
      Serial.println("Connected");
      client.subscribe(topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

int waveCount = 0; // Counter for waves

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (message == "wave") {
    // Increment wave counter
    waveCount = waveCount+1;
    
    // Blink the LED in wave pattern
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
      delay(200);
    }

    if (waveCount >= 10) {
      Serial.println("Sent email to Nishant 10 waves detected");
      waveCount = waveCount+1; // Reset wave counter
    }
  } else if (message == "pat") {
    // Blink the LED in pat pattern
    for (int i = 0; i < 2; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }

  }else{
    waveCount++;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  setupWiFi();
  client.setCallback(callback);
  connectMQTT();
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (back and forth)

  if (distance < 50) { // Distance threshold in centimeters
    Serial.println("Wave detected!");
    client.publish(topic, "wave detected from Nishant"); // Publish "wave" message
    // Blink the LED 3 times in wave pattern
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
      delay(200);
    }
  }
}
