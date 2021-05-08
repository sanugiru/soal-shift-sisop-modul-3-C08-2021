#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
 
int main()
{
    key_t key = 1234;
    int *value;
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    value = shmat(shmid, NULL, 0);

    int hasilKali[4][6];

    int baris, kolom, tengah;
    int sum = 0;
    int mtxA[4][3];
    printf("INPUT MATRIX A: \n");
    // {{1, 2, 5},
    // {3, 4, 2},
    // {5, 6, 1},
    // {7, 8, 3}
    for(int x=0; x<4; x++){
        for(int y=0; y<3; y++){
            scanf("%d", &mtxA[x][y]);
        }
    }
    int mtxB[3][6];
    printf("INPUT MATRIX B: \n");
    // {{1, 2, 2, 3, 3, 1},
    // {4, 5, 5, 6, 6, 4},
    // {7, 8, 8, 9, 9, 7}}
    for(int x=0; x<3; x++){
        for(int y=0; y<6; y++){
            scanf("%d", &mtxB[x][y]);
        }
    }


    for (baris = 0; baris < 4; baris++) {
      for (kolom = 0; kolom < 6; kolom++) {
        for (tengah = 0; tengah < 3; tengah++) {
          sum = sum + mtxA[baris][tengah]*mtxB[tengah][kolom];
        }
 
        hasilKali[baris][kolom] = sum;
        sum = 0;
      }
    }
 
    printf("Hasil Matriks:\n");
     for (baris = 0; baris < 4; baris++) {
      for (kolom = 0; kolom < 5; kolom++){
        printf("%d\t", hasilKali[baris][kolom]);
      }
      printf("\n");
    }

    for (baris = 0; baris < 4; baris++) {
      for (kolom = 0; kolom < 5; kolom++){
      *value = hasilKali[baris][kolom];
      }
    }

    shmdt(value);
    shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
