#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>

int arr[24];
unsigned long long num;
int *value; 
pthread_t thread; 

long long cek(int value[]){
    for(int i=0; i<24; i++){
        if(value[i] == 0 || arr[i] == 0){
            printf("0\t");
        }
        else if(value[i] < arr[i]){
            int batas = value[i];
            long long hasil = value[i];
            for(int x = 1; x<batas; x++){
                hasil *= (batas-x);
            }
            printf("%lld\t", hasil);
        }
        else if(value[i] >= arr[i]){
            int batas = arr[i];
            int batas2 = value[i];
            long long hasil = value[i];
            for(int x = 1; x<batas; x++){
                hasil *= (batas2-x);
            }
            printf("%lld\t", hasil);
        }
        if(i > 0 && (i+1)%6 == 0){
            printf("\n");
        }
    }
}

void main(){
    key_t key = 1234;
    int *value;
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    value = shmat(shmid, NULL, 0);

    printf("HASIL SOAL2 A: \n");
    for(int i=0;i< 4;i++){
        for(int j=0;j<6;j++){
            printf("%d\t", value[i*6+j]);
        }
        printf("\n");
    }

    printf("INPUT MATRIX B: \n");
    for(int i=0; i<24;i++){
        scanf("%d",&arr[i]);
    }

    printf("Hasil Faktorial Matrix A terhadap Matrix B:\n");
    cek(value);
    shmdt(value);
}
