#include "stdio.h" 
#include "stdlib.h"
#include "pthread.h"
#include "unistd.h"

typedef struct sensor{
    int id;
    int interval;
}sensor;

void* readSensor(void* context){
    sensor* sensor = context;
    while(1){
        printf("reading reading sensor : %d \n", sensor->id);  
        sleep(sensor->interval);
    }
}

int main(){
    sensor sensors[] = {
        {1, 2},
        {2, 3},
        {3, 4},
        {4, 5}
    };

    int size = sizeof(sensors)/sizeof(sensors[0]);
    pthread_t thread[size];
    int threadNums[size];
    
    for(int i = 0; i < size; i++){
        if(pthread_create(&thread[i], NULL, readSensor, &sensors[i]) != 0){
            printf("failed to create thread! \n");
            exit(EXIT_FAILURE);
        }
    }
    
    for(int i = 0; i < size; i++){
        pthread_join(thread[i], NULL);
    }

    return 0;
}
