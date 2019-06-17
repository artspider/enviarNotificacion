#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// La IP y MAC de modulo Arduino:
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);

// LA IP del servidor MQTT:
IPAddress server(104, 45, 149, 170);

const int lightPin = 7;

// Se crea el cliente ETHERNET con la
// IP y puerto del servidor al que nos vamos
// conectar
EthernetClient ethClient;
PubSubClient client(ethClient);

void callback(char *topic, byte *payload, unsigned int length)
{
  unsigned int i = 0;
  String topico = String(topic);
  char carga = payload[0];
  Serial.print("Llego un mensaje [");
  Serial.print(topic);
  Serial.print("] ");
  for (i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  if (topico == "inTopic")
  {
    if (payload[0] == '1')
    {
      digitalWrite(lightPin, HIGH);
      client.publish("outTopic", "Light On");
    }
    if (payload[0] == '0')
    {
      digitalWrite(lightPin, LOW);
      client.publish("outTopic", "Light Off");
    }
  }
  Serial.println();
}

void reconnect()
{
  // Si nos desconectamos, intentamos reconnectarnos
  while (!client.connected())
  {
    Serial.print("Intentandoconexión a MQTT...");
    // Intentamos conectarnos
    if (client.connect("arduinoClient"))
    {
      Serial.println("conectado...");
      // Una vez conectado, publicamos en un tópico...
      client.publish("outTopic", "hello world");
      // ... y nos volvemos a subscribir
      client.subscribe("inTopic");
    }
    else
    {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" Se intentará otra vez en 5 segundos");
      // Esperamos 5 segs
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, LOW);
  // Usamos Ethernet.init(pin) para configurar el pin CS:
  Ethernet.init(10); // Ethernet Shield

  // intentamos iniciar la conexión Ethernet:
  Ethernet.begin(mac);

  // Abrimos la comunicación serie y esperamos que se abra el puerto
  Serial.begin(9600);
  while (!Serial)
  {
    ;   // No continuamos hasta que nos conectemos.
        //Usamos el puerto USB nativo (solo para pruebas, comentar en produccion)
  }

  // Verificamos que el hardware Ethernet este presente (bien conectado)
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield no encontrado.  Lo siento, no funciona sin hardware. :(");
    while (true)
    {
      delay(1); // ciclo infinito hasta que se conecte el Shield
    }
  }
  while (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Cable Ethernet no conectado o no se obtuvo dirección IP.");
    delay(500);
  }

  // Imprimimos la IP obtenida por DHCP:
  delay(1000);
  Serial.println(Ethernet.localIP());
  client.setServer(server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}