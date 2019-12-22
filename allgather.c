#include<stdio.h>
#include<stdlib.h>
#include"mpi.h"
#include<memory.h>

//����MPI_Send��MPI_Recv��д����ʵ�ֵ�MPI_Allgather
void MPI_Allgather_my(int * senddata, int sendcount, MPI_Datatype senddatatype, int * recvdata, int recvcount,
        MPI_Datatype recvdatatype, MPI_Comm comm)
{
    int rank, size, i;
    MPI_Request request;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	memcpy(recvdata + rank * recvcount, senddata, sizeof(senddatatype)*sendcount);
    for (i = 0; i < size; i++)
    {
        if (i != rank)
        {
            MPI_Send(senddata, sendcount, senddatatype, i, rank , MPI_COMM_WORLD);
            MPI_Recv(recvdata + i * recvcount, recvcount, recvdatatype, i, i, MPI_COMM_WORLD, &status);
        }
    }
}

int main(int argc, char* argv[])
{
    int i, rank, size, tag = 1;
    int senddata, recvdata[32];
    double start_time, end_time, s_t, e_t;
    int count = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	senddata = rank + 1;
    start_time = MPI_Wtime();
    //�Լ���д��MPI_Allgather�����ܲ��� 
    MPI_Allgather_my(&senddata, count, MPI_INT, recvdata, count, MPI_INT, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    MPI_Reduce(&start_time, &s_t, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&end_time, &e_t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	for (i = 0; i < size; i++)
		printf("My rank =  %d  recv = %d\n", rank, recvdata[i]);

    if (rank == 0)
    {
        printf("myallgather : count = %d total time = %f\n", count, e_t - s_t);
    }
    MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0)
	{
		printf("--------------------\n");
	}
	//MPIԭ�е�MPI_Allgather�Ĺ��ܲ���
    start_time = MPI_Wtime();
    MPI_Allgather(&senddata, count, MPI_INT, recvdata, count, MPI_INT, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    MPI_Reduce(&start_time, &s_t, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&end_time, &e_t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	for (i = 0; i < size; i++)
		printf("My rank =  %d  recv = %d\n", rank, recvdata[i]);

    if (rank == 0)
    {
        printf("allgather : count = %d total time = %f\n", count, e_t - s_t);
    }
    MPI_Finalize();
    return 0;
}
