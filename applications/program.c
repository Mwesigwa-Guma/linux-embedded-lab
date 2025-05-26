#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "unistd.h"

typedef struct sensor{
    int data;
}sensor;

void* readSensor(sensor sensor){
    while(1){
        printf("sensor %d reading \n", sensor.data);  
        sleep(3);
    }
}

int main(){
    sensor sensors[] = {
        {1},
        {2},
        {3},
        {4}
    };

    int size = sizeof(sensors)/sizeof(sensors[0]);
    pthread_t thread[size];
    int threadNums[size];
    
    for(int i = 0; i < size; i++){
        printf("create thread %d, \n", i);
        if(pthread_create(&thread[i], NULL, readSensor(sensors[i]), &threadNums[i]) != 0){
            printf("failed to create thread! \n");
            exit(EXIT_FAILURE);
        }
    }
    
    for(int i = 0; i < size; i++){
        pthread_join(thread[i], NULL);
    }

    return 0;
}
