//Includes
#include <PubSubClient.h>
#include <MKRGSM.h>
#include "DHT.h"

//Var declaration
//SIM-internet connection
GSMClient net;
GPRS gprs;
GSM gsmAccess;

const char pin[]      = "";
const char apn[]      = "internet";
const char login[]    = "";
const char password[] = "";

//MQTT connection
PubSubClient client(net);
const String serialNumber = "proto_1";
const String mqtt_server = "192.168.99.100";
//const String mqtt_server = "broker.shiftr.io";
const String topic = "/prototype/" + serialNumber;

//DHT sensor PIN declaration
#define DHTPIN 2  //DHT is pinned on 2
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

void connect() {
//SIM not connected
    bool connected = false;
    Serial.print("Connecting to cellular network.");

//SIM connecting
    while (!connected) {
    if ((gsmAccess.begin(pin) == GSM_READY) &&
        (gprs.attachGPRS(apn, login, password) == GPRS_READY)) {
        //SIM connected
        connected = true;
        Serial.print("Connected to cellular network.");
    }
    else {
        //If SIM doesn't connect
        Serial.print(".");
        delay(1000);
        }
    }
}

void connectMQTT() {
//SIM not connected
    bool MQTTconnected = false;
    Serial.println("Connecting to MQTT broker.");

//SIM connecting
    while (!MQTTconnected) {
        client.setServer(mqtt_server.c_str(), 8080);
        client.connect(mqtt_server.c_str());

    if (client.connected()) {
        //SIM connected
        MQTTconnected = true;
        Serial.print("MQTT connection state: ");
        Serial.println(client.state());
    }
    else {
        //If SIM doesn't connect
        Serial.print(".");
        delay(1000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    connect();
    //Connect to Docker MQTT
    connectMQTT();
    //Start DHT 11
    dht.begin();
}

void loop() {
    delay(10000);

    //Get DHT values
    float humidty = dht.readHumidity();
    float temperature = dht.readTemperature();

    //Create JSON out of values and send it.
    const String json = "{\"temperature\": " + String(temperature, 2) + ", \"humidity\": " + String(humidty) + " }";
    Serial.println(json);
    client.publish(topic.c_str(), json.c_str());
    client.loop();

    //Check if MQTT connection is holding.
    Serial.print("MQTT connection state: ");
    Serial.println(client.state());

    //Reconnect if MQTT connection is lost.
    if (!client.connected()) {
    Serial.println("MQTT disconnected! Trying reconnect.");
    client.connect("whatever");
  }
   
}