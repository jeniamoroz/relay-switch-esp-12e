#include <PubSubClient.h>
#include <ESP8266WiFi.h>

void callback(char* topic, byte* payload, unsigned int length);

#define client_name "Switch 1"
#define wifi_ssid "SSID"
#define wifi_password "PSK"

#define mqtt_server "HOST"
#define mqtt_user "USER"
#define mqtt_password "PASSWORD"

#define switch_topic "switch1"
#define switch_topic_confirm "switch1/confirm"
#define switch_pin 5 //D1

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, callback, wifiClient);

void setup() {
  //initialize the switch as an output and set to HIGH (on)
  pinMode(switch_pin, OUTPUT); // Relay Switch 1
  digitalWrite(switch_pin, HIGH);
  //start the serial line for debugging
  Serial.begin(115200);
  Serial.println("Restarting.");
  delay(100);
  //start wifi subsystem
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();
  //wait a bit before starting the main loop
  delay(2000);
}


void loop() {
  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == WL_CONNECTED) {
    reconnect();
  }
  //maintain MQTT connection
  client.loop();
  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10);
}

void callback(char* topic, byte* payload, unsigned int length) {
  //convert topic to string to make it easier to work with
  String topicStr = topic;
  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

  if (topicStr == switch_topic) {
    //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
    if (payload[0] == '1') {
      digitalWrite(switch_pin, HIGH);
      client.publish(switch_topic_confirm, "1");
    }
    //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
    else if (payload[0] == '0') {
      digitalWrite(switch_pin, LOW);
      client.publish(switch_topic_confirm, "0");
    }
  }
}


void reconnect() {
  //attempt to connect to the wifi if connection is lost
  if (WiFi.status() != WL_CONNECTED) {
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if (WiFi.status() == WL_CONNECTED) {
    // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect(client_name, mqtt_user, mqtt_password)) {
        Serial.println("\tMQTT Connected");
        client.publish(switch_topic_confirm, "1");
        client.subscribe(switch_topic);
      }

      // otherwise print failed for debugging
      else {
        Serial.println("\tFailed. Reconnecting in 10 seconds");
        delay(10000);
      }
    }
  }
}
