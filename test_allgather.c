#include "mpi.h"
#include "stdio.h"
#include "my_allgather.c"
#include "string.h"
#include "stdlib.h"
#define PN 4

int print_array(double * arr, int h, int w)
{
    for(int i=0; i<h; i++)
    {
        for(int j=0; j<w; j++){
            printf("%lf ", arr[i*w+j]);            
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int rank, size;
    int exp_times = 1, data_len = 64; 
    double stime, etime, *senddata, *recvbuf;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    for(int i=64; i<=data_len; i*=2){

        senddata = (double*)malloc(i*sizeof(double));
        recvbuf = (double*)malloc(i*size*sizeof(double));
        for(int k=0; k<i; k++){
            senddata[k] = rank;
        }
        stime = 1000*MPI_Wtime();
        for(int j=0; j<exp_times; j++){
            // MY_Allgather(senddata, i, MPI_DOUBLE, recvbuf, i, MPI_DOUBLE, MPI_COMM_WORLD);
            MPI_Allgather(senddata, i, MPI_DOUBLE, recvbuf, i, MPI_DOUBLE, MPI_COMM_WORLD);
            printf("%d", rank);
        }
        etime = 1000*MPI_Wtime();
        if(rank == 0){
            printf("%lf\n", (etime-stime)/exp_times);
        }
        
        // if(rank == 1){
        //     print_array(recvbuf, size, data_len);
        // }
        free(senddata);
        free(recvbuf);
    }
    // //recvbuf = (int*)recvbuf;
    // printf("after process:%d\t data:", rank);
    
    // printf("\n");
    MPI_Finalize();
}
