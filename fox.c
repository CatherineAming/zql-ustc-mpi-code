#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <string.h>

#define TAG_SCATTER_A 0
#define TAG_SCATTER_B 1
#define TAG_GATHER_C 2
#define TAG_UPSHIFT 3

int get_index(int row, int col, int len)
{
    return row*len+col;
}

void getAB(double **A, double **B, int dg)
{
    int val = 0;
	int i,j;
    for(i=0; i<dg; i++){
        for(j=0; j<dg; j++){
            A[i][j] = val;
            B[i][j] = val+1;
            val ++;
        }
    }
    print_matrix_2d(A, dg, dg);
    print_matrix_2d(B, dg, dg);
}

void print_matrix(double* matrix, int h, int w)
{	int i;
    for(i=0; i<h; i++){
		int j;
        for(j=0; j<w; j++){
            printf("%4.2f ",matrix[get_index(i, j, w)]);
        }
        printf("\n");
    }
}

void print_matrix_2d(double **matrix, int h, int w)
{	int i;
    for(i=0; i<h; i++){
		int j;
        for(j=0; j<w; j++){
            printf("%4.2f ",matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void scatterAB(double **A, double **B, int size, int dl, int ts){
    int i, t;
    int is, ie, js, je;
    double *tmpa, *tmpb;
    int dl2 = dl*dl;
    tmpa = malloc(dl2*sizeof(double));
    tmpb = malloc(dl2*sizeof(double));
    for (i=1; i<size; i++){
        js = i%ts*dl;
        je = (i%ts+1)*dl;
        is = i/ts*dl;
        ie = (i/ts+1)*dl;
        t = 0;
		int k;
        for(k=is; k<ie; k++){
			int v;
            for(v=js; v<je; v++){
                tmpa[t] = A[k][v];
                tmpb[t] = B[k][v];
                t ++;
            }
        }
        MPI_Send(tmpa, dl2, MPI_DOUBLE, i, TAG_SCATTER_A, MPI_COMM_WORLD);
        MPI_Send(tmpb, dl2, MPI_DOUBLE, i, TAG_SCATTER_B, MPI_COMM_WORLD);
    }
    free(tmpa);
    free(tmpb);
}

void gatherC(double **C, int size, int dl, int ts){
    int i, t;
    int is, ie, js, je;
    double *tmp;
    MPI_Status status;
    int dl2 = dl*dl;
    tmp = malloc(dl2*sizeof(double));
    for (i=1; i<size; i++){
        MPI_Recv(tmp, dl2, MPI_DOUBLE, i, TAG_GATHER_C, MPI_COMM_WORLD, &status);
        js = i%ts*dl;
        je = (i%ts+1)*dl;
        is = i/ts*dl;
        ie = (i/ts+1)*dl;
        t = 0;
		int k;
        for(k=is; k<ie; k++){
			int v;
            for(v=js; v<je; v++){
                C[k][v] = tmp[t];
                t ++;
            }
        }
    }
    free(tmp);
}

void mult(double *a, double *b, double *c, int dl)
{	int i,j,k;
    for(i=0; i<dl; i++){
        for(j=0; j<dl; j++){
            for(k=0; k<dl; k++){
                c[get_index(i, j, dl)] += a[get_index(i, k, dl)] * b[get_index(k, j, dl)];
            }
        }
    }
}

// void mult_2d(double *a, double *b, double *c, int dl)
// {
//     for(int i=0; i<dl; i++){
//         for(int j=0; j<dl; j++){
//             for(int k=0; k<dl; k++){
//                 c[i][j] += a[i][k] * b[k][j];
//             }
//         }
//     }
// }

void fox(int dg)
{
    int rank, size, ts, dl, dl2;
    int col, row, row_rank, col_rank;
    double *block_a, *block_b, *tmpc, *tmp;
    MPI_Status status;
    MPI_Comm comm_row, comm_col;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    ts = sqrt(size);
    dl = dg/ts;
    dl2 = dl*dl;

    // 将每一个进程分到不同的row和col中
    row = rank/ts;
    col = rank%ts;
    MPI_Comm_split(MPI_COMM_WORLD, row, col, &comm_row);
    MPI_Comm_split(MPI_COMM_WORLD, col, row, &comm_col);

    
    // MPI_Comm_rank(comm_row, &row_rank);
    // MPI_Comm_rank(comm_col, &col_rank);
    // printf("%d %d %d\n",rank, row_rank, col_rank);

    block_a = malloc(dl2*sizeof(double));
    block_b = malloc(dl2*sizeof(double));
    tmpc = malloc(dl2*sizeof(double));
    tmp = malloc(dl2*sizeof(double));

    int i;
    // p0 获取矩阵A,B;向所有处理器发送矩阵
    if (rank ==0){
        double **A, **B;
        A = (double **)malloc( dg * sizeof(double*) );
        B = (double **)malloc( dg * sizeof(double*) );
        for(i=0; i<dg; i++){
            A[i] = (double *)malloc( dg * sizeof(double) );
            B[i] = (double *)malloc( dg * sizeof(double) );
        }
        getAB(A, B, dg);
        scatterAB(A, B, size, dl, ts);
        int t =0;
		int k,v;
        for(k=0; k<dl; k++){
            for(v=0; v<dl; v++){
                block_a[t] = A[k][v];
                block_b[t] = B[k][v];
                t ++;
            }
        }
    }
    else {// 各处理器接受rank0发送的数据
        MPI_Recv(block_a, dl2, MPI_DOUBLE, 0, TAG_SCATTER_A, MPI_COMM_WORLD, &status);
        MPI_Recv(block_b, dl2, MPI_DOUBLE, 0, TAG_SCATTER_B, MPI_COMM_WORLD, &status);
    }
    printf("%d origin a, b\n", rank);
    print_matrix(block_a, dl, dl);
    print_matrix(block_b, dl, dl);
    printf("\n");
    // print_matrix(tmp_a, dl, dl);
    // print_matrix(block_a, dl, dl);
    // free(A);
    // free(B);
    

    // fox
    memset(tmpc, 0, dl2*sizeof(double));
	//int i;
    for(i=0; i<ts; i++){
        if(col == row+i){
            memcpy(tmp, block_a, dl2*sizeof(double));
        }
        MPI_Bcast(tmp, dl2, MPI_DOUBLE, (row+i)%ts, comm_row);
        mult(tmp, block_b, tmpc, dl);
        // if(rank == 1){
        //     print_matrix(tmp, dl, dl);
        //     print_matrix(block_b, dl, dl);
        //     printf("\n");
        // }
        MPI_Sendrecv(block_b, dl2, MPI_DOUBLE, (row-1+ts)%ts, 
                     TAG_UPSHIFT, 
                     tmp, dl2, MPI_DOUBLE, (row+1)%ts, TAG_UPSHIFT, 
                     comm_col, &status);
        memcpy(block_b, tmp, dl2*sizeof(double));
    }

    //gather results
    if(rank == 0){
        double **C;
        C = (double **)malloc( dg * sizeof(double*) );
		int i,k,v;
        for(i=0; i<dg; i++){
            C[i] = (double *)malloc( dg * sizeof(double) );
        }
        int t = 0;
        for(k=0; k<dl; k++){
            for(v=0; v<dl; v++){
                C[k][v] = tmpc[t];
                t ++;
            }
        }
        gatherC(C, size, dl, ts);
        print_matrix_2d(C, dg, dg);
    }else{
        MPI_Send(tmpc, dl2, MPI_DOUBLE, 0, TAG_GATHER_C, MPI_COMM_WORLD);
    }
} 

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    fox(4);
    MPI_Finalize();
    return 0;
}
