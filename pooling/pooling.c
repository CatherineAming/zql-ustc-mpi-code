#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include<sys/time.h>

#define SAME_PADDING 0
#define VALID_PADDING 1

#define MAX_POOLING 0
#define AVG_POOLING 1

int get_index(int row, int col, int len)
{
    return row*len+col;
}

void print_matrix(float* matrix, int h, int w)
{
    for(int i=0; i<h; i++){
        for(int j=0; j<w; j++){
            printf("%4.2f ",matrix[get_index(i, j, w)]);
        }
        printf("\n");
    }
}

float max_filting(float *input, int x, int y, int filter_h, int filter_w, int input_w)
{
    float max = 0.0;
    for(int i=x; i<filter_h+x; i++){
        for(int j=y; j<filter_w+y; j++){
            //printf("%f ", input[get_index(i, j, input_w)]);
            if(input[get_index(i, j, input_w)]>max){
                max = input[get_index(i, j, input_w)];
            }
        }
    }
    return max;
}

float avg_filting(float *input, int x, int y, int filter_h, int filter_w, int input_w)
{
    float sum = 0.0;
    for (int i=x; i<filter_h+x; i++){
        for(int j=y; j<filter_w+y; j++){
            // printf("%lf ", input[get_index(i, j, input_w)]);
            sum += input[get_index(i, j, input_w)];
        }
    }
    // printf("\n");
    return sum/(filter_h*filter_w);
}

float *pooling(float *input, int *input_shape, 
               int stride, int *kernel_shape, 
               int type, int padding)
{
    
    // omp_set_num_threads(32);
    int input_h_padded, input_w_padded, output_h, output_w;
    float *input_padded, *output;
    float (*filt) (float *input, int x, int y, int filter_h, int filter_w, int input_w);

    int input_n = input_shape[0];
    int input_h = input_shape[1];
    int input_w = input_shape[2];
    int input_c = input_shape[3];

    int kernel_h = kernel_shape[0];
    int kernel_w = kernel_shape[1];

    if(type == MAX_POOLING){
        filt = &max_filting;
    }else if(type == AVG_POOLING){
        filt = &avg_filting;
    }else{

    }
    // commpute output shape and pad shape
    if(padding == SAME_PADDING){
        output_h = ceil((float)input_h/stride);
        output_w = ceil((float)input_w/stride);
    }else{
        output_h = ceil(((float)input_h-kernel_h)/stride);
        output_w = ceil(((float)input_w-kernel_w)/stride);
    }
    input_h_padded = (output_h-1)*stride + kernel_h;
    input_w_padded = (output_w-1)*stride + kernel_w;

    output = malloc(output_w*output_h*sizeof(float));

    // padding 
    if(input_h_padded==input_h && input_w_padded==input_w){
        input_padded = input;
    }else{
        int pad_h = (input_h_padded-input_h)/2;
        int pad_w = (input_w_padded-input_w)/2;
        input_padded=(float *)malloc(input_w_padded*input_h_padded*sizeof(float));
        omp_set_num_threads(32);
        #pragma omp parallel for
            for(int i=0; i<input_h_padded; i++){
                for(int j=0; j<input_w_padded; j++){
                    if(i>=pad_h && i<pad_h+input_h && j>=pad_w && j<pad_w+input_w){
                        input_padded[get_index(i, j, input_w_padded)] = input[get_index(i-pad_h, j-pad_w, input_w)];       
                    }else{
                        input_padded[get_index(i, j, input_w_padded)]=0.0;
                    }
                }
            }
    }

    // filting
    #pragma omp parallel for
        for (int i=0; i<output_h; i++){
        int threads_num = omp_get_num_threads();
        // printf("threads num：%d \n", threads_num);
            for (int j=0; j<output_w; j++){
                output[get_index(i, j, output_w)] = (*filt)(input_padded, i*stride, j*stride, kernel_h, kernel_w, input_w_padded);
            }
        }
    // print_matrix(input_padded, input_h_padded, input_w_padded);
    // print_matrix(output, output_h, output_w);
    
    return output;

}

int main(int argc, char* argv[])
{
    int exp_times = 20;
    
    int kernel_shape[2] = {3, 3};

    printf("datasize\trunning time(s)\n");
    struct timeval start_time,end_time;
    for(int i=64; i<=4096*2; i*=2){
        int input_len = i*i;
        int input_shape[4] = {1, i, i, 1};
        float *input;
        input = (float*)malloc(input_len*sizeof(float));
        //init
        for(int i=0; i<input_len; i++){
            input[i] = i;
        }
        gettimeofday(&start_time,NULL);
        for(int i=0; i<exp_times; i++){
            float* output = pooling(input, input_shape, 1, kernel_shape, AVG_POOLING, SAME_PADDING);
        }
        gettimeofday(&end_time,NULL);
        float total_time = ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec)) * 1.0 / 1000000;
        printf("%d\t%lf\n", i, total_time/exp_times);
    }
    
    return 0;
}
