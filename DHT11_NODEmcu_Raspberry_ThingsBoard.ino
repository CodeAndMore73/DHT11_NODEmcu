#include <DHT11.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SSID wifi";
const char* password = "Contraseña wifi";

#define TOKEN "TOKEN de nuestro sensor"

DHT11 dht11(D0);

char thingsboardServer[] = "IP Servidor ThingsBoard (Raspberry)";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup() {
  Serial.begin(9600);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;

}

void loop() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      Serial.print("Intentando conectar a la red wifi: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid, password);
      delay(500);
    }
    Serial.println("Conectado a la red wifi");
  }

  if ( !client.connected() ) {
    reconnect();
  }

  //Cada minuto capturas los datos de temperatura y humedad del sensor
  if ( millis() - lastSend > 60000 ) { 
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  client.loop();
}

void InitWiFi()
{
  // Intentamos conectarnos a la red wifi

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

}

void reconnect() {
  // Bucle hasta reconectar
  while (!client.connected()) {
    Serial.print("Connecting to Thingsboard node ...");
    // Intentando conectar al dispositivo (clientId, username, password)
    if ( client.connect("IDCliente", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Esperamos 10 segundos
      delay( 10000 );
    }
  }
}

void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Recuperando los datos de temperatura y humedad.");

  // Leer los datos tarda alrededor de  250 milisegundos
  int err;
  float h, t;
       if((err = dht11.read(h,t)) == 0)    // Si devuelve 0 es que ha leido bien
          {
             Serial.print("Temperatura: ");
             Serial.print(t);
             Serial.print(" Humedad: ");
             Serial.print(h);
             Serial.println();
          }
             else
          {
             Serial.println();
             Serial.print("Error Num :");
             Serial.print(err);
             Serial.println();
          }


  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");

  String temperature = String(t);
  String humidity = String(h);


  // Mensajes de depuración
  Serial.print( "Enviando temperatura y humedad : [" );
  Serial.print( temperature ); Serial.print( "," );
  Serial.print( humidity );
  Serial.print( "]   -> " );

  // Preparamos el string JSON payload
  String payload = "{";
  payload += "\"temperatura\":"; payload += temperature; payload += ",";
  payload += "\"humedad\":"; payload += humidity;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );
  delay(60000);            //Recordad que solo lee una vez por minuto
  
}