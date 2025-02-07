#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#define ADDRESS     "tcp://192.168.0.171:1883"  // Direccion del broker MQTT
#define CLIENTID    "BeagleBoneClient"          // ID del cliente MQTT
#define TOPIC       "test/topic"                // Tema (topic) MQTT
#define PAYLOAD     "Saluda Rome desde BeagleBone!"    // Mensaje a enviar
#define QOS         1                           // Calidad de Servicio (QoS)
#define TIMEOUT     1000L                       // Tiempo de espera (ms)

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Crear cliente MQTT
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Configurar opciones de conexion
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    // Conectar al broker
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Error conectando al broker: %d\n", rc);
        return rc;
    }
    printf("Conectado al broker MQTT en %s\n", ADDRESS);

    // Publicar mensaje
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    printf("Mensaje enviado: %s\n", PAYLOAD);

    // Esperar confirmacion de la entrega
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Mensaje entregado con código %d\n", rc);

    // Desconectar y limpiar
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return 0;
}
