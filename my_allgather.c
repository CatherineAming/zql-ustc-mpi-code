#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

#define SEND_RECV_TAG 1
#define ALLGATHER_TAG 2

int MY_Allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                 void* recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm)
{
    int rank, size;
    int senddata_size, recvdata_size;

    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    MPI_Status status;

    MPI_Type_size(sendtype, &senddata_size);
    MPI_Type_size(recvtype, &recvdata_size);
    int send_offset = rank;
    int recv_offset = (size+rank-1)%size;
   
    //sendbuf = (sendtype*)sendbuf;
    //recvbuf = (recvtype*)recvbuf;
 
    int send_dest = (rank+1)%size;
    int recv_dest = (size+rank-1)%size;
    MPI_Send((char*)sendbuf, sendcount, sendtype, 
                 send_dest, ALLGATHER_TAG, comm); 
    MPI_Recv((char*)recvbuf+(recv_offset*recvdata_size*recvcount), recvcount, recvtype,
                 recv_dest, ALLGATHER_TAG, comm, &status);

    for(int i=0; i<size; i++)
    {
        send_offset = recv_offset;
        recv_offset = (recv_offset+size-1)%size;
        MPI_Send((char*)recvbuf+(send_offset*senddata_size*recvcount), sendcount, sendtype, 
                 send_dest, ALLGATHER_TAG, comm); 
        MPI_Recv((char*)recvbuf+(recv_offset*recvdata_size*recvcount), recvcount, recvtype,
                 recv_dest, ALLGATHER_TAG, comm, &status);
        
    }
}

// int main(int argc, char* argv[])
// {
//     int rank, size;
//     MPI_Status status;    

//     MPI_Init(&argc, &argv);
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);
//     //send
//     if(0==rank)
//     {
//         char* senddata = "hello!";
//         MPI_Send(&senddata, 7, MPI_CHAR, 1, SEND_RECV_TAG, MPI_COMM_WORLD);
//     }else {
//         char* recvdata;
//         MPI_Recv(&recvdata, 7, MPI_CHAR, 0, SEND_RECV_TAG, MPI_COMM_WORLD, &status);
//         printf("%s", recvdata);
//     }
//     MPI_Finalize();
// }
