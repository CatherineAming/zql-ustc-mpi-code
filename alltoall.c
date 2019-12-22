#include<stdio.h>
#include"mpi.h"
int main(int argc,char **argv)
{
	int rank,size,tag=1,i,j;
	int X[3][3]={{1,2,3},{4,5,6},{7,8,9}},Y[3][3]={0};
	MPI_Request request1,request2;
	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	for(i=0;i<3;++i)
	{
		if(rank==i)
		{
			for(j=0;j<3;++j)
			{
				MPI_Isend(&X[i][j],1,MPI_INT,j,tag,MPI_COMM_WORLD,&request2);
				MPI_Wait(&request2,&status);
				printf("%d send\n",i);
			}
			MPI_Irecv(&Y[i][i],1,MPI_INT,i,tag,MPI_COMM_WORLD,&request1);
			MPI_Wait(&request1,&status);
			printf("%d reveice\n",i);
		}
		else
		{
			MPI_Irecv(&Y[rank][i],1,MPI_INT,i,tag,MPI_COMM_WORLD,&request1);
			MPI_Wait(&request1,&status);
			printf("%d reveice\n",rank);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(rank==0)
	{
		for(i=0;i<3;++i)
		{
			printf("%d %d %d\n",Y[i][0],Y[i][1],Y[i][2]);
		}
	}
	MPI_Finalize();
	return 0;  
}
