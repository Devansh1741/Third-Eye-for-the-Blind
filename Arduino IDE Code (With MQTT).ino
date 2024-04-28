#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Devansh2003";
const char* password = "09062003";
const char* mqtt_server = "test.mosquitto.org";

#define trigPin D5
#define echoPin D6
#define buzzer D2
int duration = 0;
int threshold = 30;
int distance = 0;
bool run = true;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if(strcmp(topic, "Dev/Led") == 0){
    if ((char)payload[0] == '1') {
      digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
      run = true;
    } else {
      digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
      run =false;
    }
  }
  else {
    threshold = 0;
    for (int i = 0; i < length; i++) {
      threshold = threshold*10 + ((char)payload[i] - '0');
    }
    // threshold = (payload[0] << 8) + payload[1];
     Serial.print("Threshold set to ");
     Serial.println(threshold);
  }


}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("Dev/Ultra", "MQTT Server is Connected");
      // ... and resubscribe
      client.subscribe("Dev/Led");
      client.subscribe("Dev/Thresh");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);

    distance = duration*0.034/2;
    if(!run) digitalWrite(buzzer, LOW);
    if(run){
      if(distance <= threshold) digitalWrite(buzzer, HIGH);
      else digitalWrite(buzzer, LOW);
    }

    snprintf (msg, MSG_BUFFER_SIZE, "Distance: %ld", distance);
    if(run){
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("Dev/Ultra", msg); 
    }
  }
}
