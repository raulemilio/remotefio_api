#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  // Para sleep()
#include <stdlib.h>

volatile int contador = 0; // Contador compartido entre las tareas

// Primer tarea: Incrementa el contador indefinidamente cada 1 segundo
void* tarea1(void* arg) {
    while (1) {
        sleep(1);
        contador++;
        printf("Tarea 1: Contador = %d\n", contador);
    }
    return NULL;
}

// Segunda tarea: Incrementa el contador una sola vez
void* tarea2(void* arg) {
    sleep(1);  // Aseguramos que la tarea 2 se ejecute después de la tarea 1
    contador++;
    printf("Tarea 2: Contador = %d\n", contador);
    return NULL;
}

// Tercer tarea: Se ejecuta hasta que el contador llegue a 10
void* tarea3(void* arg) {
    while (contador < 10) {
        sleep(1);
        contador++;
        printf("Tarea 3: Contador = %d\n", contador);
    }
    printf("Tarea 3 ha terminado: El contador alcanzó 10.\n");
    return NULL;
}

int main() {
    pthread_t hilo1, hilo2, hilo3;

    // Crear los hilos
    pthread_create(&hilo1, NULL, tarea1, NULL);
    pthread_create(&hilo2, NULL, tarea2, NULL);
    pthread_create(&hilo3, NULL, tarea3, NULL);

    // Esperar a que los hilos terminen
    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);
    pthread_join(hilo3, NULL);

//    pthread_detach(hilo2);
//    pthread_detach(hilo3);


    printf("Programa terminado.\n");
    return 0;
}

