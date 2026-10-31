#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define PINO_DHT 15
#define PINO_LDR 34

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const char* MQTT_BROKER = "3.135.234.101";
const int MQTT_PORT = 1883;

// Tópicos MQTT
const char* TOPICO_SENSORES = "/TEF/vinheria001joaquim/attrs";
const char* TOPICO_COMANDO = "/TEF/vinheria001joaquim/cmd";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

DHTesp dhtSensor;


void receberMensagem(char* topic, byte* payload, unsigned int length) {

  Serial.println();
  Serial.println("Mensagem recebida pelo MQTT!");

  Serial.print("Topico: ");
  Serial.println(topic);

  Serial.print("Mensagem: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println();
}


void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando ao MQTT...");

    if (mqttClient.connect("ESP32VinheriaJoaquim")) {

      Serial.println(" conectado!");

      // ESP32 escuta mensagens enviadas pelo MyMQTT
      mqttClient.subscribe(TOPICO_COMANDO);

      Serial.print("Inscrito no topico: ");
      Serial.println(TOPICO_COMANDO);

    } else {

      Serial.print(" falhou, codigo: ");
      Serial.println(mqttClient.state());

      delay(2000);
    }
  }
}


void setup() {

  Serial.begin(115200);

  dhtSensor.setup(PINO_DHT, DHTesp::DHT22);
  pinMode(PINO_LDR, INPUT);

  Serial.print("Conectando ao Wi-Fi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  mqttClient.setCallback(receberMensagem);

  conectarMQTT();

  Serial.println("Vinheria Agnello - Sistema iniciado!");
}


void loop() {

  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();

  TempAndHumidity dados = dhtSensor.getTempAndHumidity();

  int luminosidade = analogRead(PINO_LDR);

  String mensagem = "";

  mensagem += "t|";
  mensagem += String(dados.temperature, 2);

  mensagem += "|u|";
  mensagem += String(dados.humidity, 2);

  mensagem += "|l|";
  mensagem += String(luminosidade);

  mqttClient.publish(
    TOPICO_SENSORES,
    mensagem.c_str()
  );

  Serial.println("-------------------------");

  Serial.print("Temperatura: ");
  Serial.print(dados.temperature);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(dados.humidity);
  Serial.println(" %");

  Serial.print("Luminosidade: ");
  Serial.println(luminosidade);

  Serial.print("Enviado ao MQTT: ");
  Serial.println(mensagem);

  delay(2000);
}