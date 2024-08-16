/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, x, y, x1, x2, x3, x4, x5, x6, x7, x8;
    if (M == 32 && N == 32)
    {
        for (i = 0; i < M; i += 8)
        {
            for (j = 0; j < N; j += 8)
            {
                if (i == j)
                {
                    for (x = i; x < 8 + i; x++)
                    {
                        x1 = A[x][j], x2 = A[x][j + 1], x3 = A[x][j + 2], x4 = A[x][j + 3], x5 = A[x][j + 4], x6 = A[x][j + 5], x7 = A[x][j + 6], x8 = A[x][j + 7];
                        B[x][j] = x1, B[x][j + 1] = x2, B[x][j + 2] = x3, B[x][j + 3] = x4, B[x][j + 4] = x5, B[x][j + 5] = x6, B[x][j + 6] = x7, B[x][j + 7] = x8;
                    }
                    for (x = i; x < 8 + i; x++)
                    {
                        for (y = j; y < j + 8; y++)
                            if (x + 1 <= y)
                            {
                                x1 = B[x][y];
                                B[x][y] = B[y][x];
                                B[y][x] = x1;
                            }
                    }
                }
                else
                {
                    for (x = i; x < 8 + i; x++)
                        for (y = j; y < 8 + j; y++)
                            B[y][x] = A[x][y];
                }
            }
        }
    }
    else if (M == 64 && N == 64)
    {
        for (i = 0; i < M; i += 8)
        {
            for (j = 0; j < N; j += 8)
            {
                // 开始讨论一个8*8的分块,先把前8*4中的4*4先转置,然后再把后4*4挪到转置
                // 块里的前8*4的后4*4分块中
                for (x = i; x < i + 4; x++)
                {
                    // x1=A[x][j],x2=A[x][j+1],x3=A[x][j+2],x4=A[x][j+3];
                    // B[j][x]=x1,B[j+1][x]=x2,B[j+2][x]=x3,B[j+3][x]=x4;
                    // //开始转移后4*4
                    // //x=4, x4=A[4][7]-->B[7][4]-->B[3][8]
                    // x1=A[x][j+4],x2=A[x][j+5],x3=A[x][j+6],x4=A[x][j+7];
                    // B[j][x+4]=x1,B[j+1][x+4]=x2,B[j+2][x+4]=x3,B[j+3][x+4]=x4;
                    x1 = A[x][j], x2 = A[x][j + 1], x3 = A[x][j + 2], x4 = A[x][j + 3];
                    x5 = A[x][j + 4], x6 = A[x][j + 5], x7 = A[x][j + 6], x8 = A[x][j + 7];
                    B[j][x] = x1, B[j + 1][x] = x2, B[j + 2][x] = x3, B[j + 3][x] = x4;
                    B[j][x + 4] = x5, B[j + 1][x + 4] = x6, B[j + 2][x + 4] = x7, B[j + 3][x + 4] = x8;
                }
                for (y = j; y < j + 4; y++)
                {
                    x1 = A[i + 4][y], x2 = A[i + 5][y], x3 = A[i + 6][y], x4 = A[i + 7][y];
                    x5 = B[y][i + 4], x6 = B[y][i + 5], x7 = B[y][i + 6], x8 = B[y][i + 7];

                    B[y][i + 4] = x1, B[y][i + 5] = x2, B[y][i + 6] = x3, B[y][i + 7] = x4;
                    // 把存在B中的右上角的数字往左下角挪,减去偏移量
                    B[y + 4][i] = x5, B[y + 4][i + 1] = x6, B[y + 4][i + 2] = x7, B[y + 4][i + 3] = x8;
                }
                for (x = i + 4; x < i + 8; x++)
                // for(y=j+4;y<j+8;y++)
                //   B[x][y]=A[y][x];
                {
                    x1 = A[x][j + 4], x2 = A[x][j + 5], x3 = A[x][j + 6], x4 = A[x][j + 7];
                    B[j + 4][x] = x1, B[j + 5][x] = x2, B[j + 6][x] = x3, B[j + 7][x] = x4;
                }
            }
        }
    }
    else if (M == 61 && N == 67)
    {
        for (i = 0; i < N; i += 17)
            for (j = 0; j < M; j += 17)
            {
                for (x = i; x < i + 17 && x < N; x++)
                {
                    for (y = j; y < j + 17 && y < M; y++)
                        B[y][x] = A[x][y];
                }
            }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}
char trans_1_desc[]="直接进行8*8的分块,不做任何优化";
void trans_1(int M,int N,int A[N][N],int B[N][N]){
    if(M==32){
        for(int i=0;i<M;i+=8)
        for(int j=0;j<N;j+=8)
        for(int m=i;m<8+i;m++)
        for(int n=j;n<8+j;n++)
        B[n][m]=A[m][n];
    }
}
char trans_2_desc[]="进行8*8的分块,并且优化对角线";
void trans_2(int M,int N ,int A[M][N],int B[M][N])
{int i,j,m,n,x1,x2,x3,x4,x5,x6,x7,x8;
    for( i=0;i<M;i+=8){
        for( j=0;j<N;j+=8){
            if(i==j){
                for(m=i;m<8+i;m++)
                {
                   x1=A[m][j];
                   x2=A[m][j+1];
                   x3=A[m][j+2];
                   x4=A[m][j+3];
                   x5=A[m][j+4];
                   x6=A[m][j+5];
                   x7=A[m][j+6];
                   x8=A[m][j+7];
                   B[j][m]=x1;
                   B[j+1][m]=x2;
                   B[j+2][m]=x3;
                   B[j+3][m]=x4;
                   B[j+4][m]=x5;
                   B[j+5][m]=x6;
                   B[j+6][m]=x7;
                   B[j+7][m]=x8;      
                }
               
            }
             else {
                    for( m=i;m<8+i;m++)
                    for( n=j;n<8+j;n++)
                    B[n][m]=A[m][n];
                }
        }
    }
}
char trans_3_desc[]="进行8*8的分块,并且优化对角线,逼近极限";
void trans_3(int M,int N ,int A[M][N],int B[M][N]){
//暂时憋不出来
int i,j,m,n,x1,x2,x3,x4,x5,x6,x7,x8;
    for( i=0;i<M;i+=8){
        for( j=0;j<N;j+=8){
            if(i==j){
                for(m=i;m<8+i;m++){
                    x1=A[m][j],x2=A[m][j+1],x3=A[m][j+2],x4=A[m][j+3],x5=A[m][j+4],x6=A[m][j+5],x7=A[m][j+6],x8=A[m][j+7];
                    B[m][j]=x1,B[m][j+1]=x2,B[m][j+2]=x3,B[m][j+3]=x4,B[m][j+4]=x5,B[m][j+5]=x6,B[m][j+6]=x7,B[m][j+7]=x8;
                }
                for(m=i;m<8+i;m++){
                    for(n=j;n<j+8;n++)
                    if(m+1<=n){
                    x1=B[m][n];
                    B[m][n]=B[n][m];
                    B[n][m]=x1;
                    }
                   
                }
            }
             else {
                    for( m=i;m<8+i;m++)
                    for( n=j;n<8+j;n++)
                    B[n][m]=A[m][n];
                }
        }
    }
}
char trans_4_desc[]="64*64的矩阵,思考分块8*4";
void trans_4(int M,int N,int A[M][N],int B[M][N]){
    int i,j,m,n;
    for( i=0;i<M;i+=8)
    for( j=0;j<N;j+=4)
    for(m=i;m<i+8;m++)
    for(n=j;n<j+4;n++)
    B[m][n]=A[n][m];
}
char trans_5_desc[]="64*64的矩阵,思考分块4*4";
void trans_5(int M,int N,int A[M][N],int B[M][N]){
    int i,j,x,x1,x2,x3,x4;
    for( i=0;i<M;i+=4)
    for( j=0;j<N;j+=4)
    for(x=i;x<i+4;x++)
    {
        x1=A[x][j],x2=A[x][j+1],x3=A[x][j+2],x4=A[x][j+3];
        B[j][x]=x1,B[j+1][x]=x2,B[j+2][x]=x3,B[j+3][x]=x4;
    }
}
char trans_6_desc[]="64*64的矩阵,思考分块8*8下以8*4为小块进行转置";
void trans_6(int M,int N,int A[M][N],int B[M][N]){
        int i,j,x,y,x1,x2,x3,x4,x5,x6,x7,x8;
        for( i=0;i<M;i+=8){
            for( j=0;j<N;j+=8){
                //开始讨论一个8*8的分块,先把前8*4中的4*4先转置,然后再把后4*4挪到转置
                //块里的前8*4的后4*4分块中
                for(x=i;x<i+4;x++){
                    // x1=A[x][j],x2=A[x][j+1],x3=A[x][j+2],x4=A[x][j+3];
                    // B[j][x]=x1,B[j+1][x]=x2,B[j+2][x]=x3,B[j+3][x]=x4;
                    // //开始转移后4*4
                    // //x=4, x4=A[4][7]-->B[7][4]-->B[3][8]
                    // x1=A[x][j+4],x2=A[x][j+5],x3=A[x][j+6],x4=A[x][j+7];
                    // B[j][x+4]=x1,B[j+1][x+4]=x2,B[j+2][x+4]=x3,B[j+3][x+4]=x4;
                    x1=A[x][j],x2=A[x][j+1],x3=A[x][j+2],x4=A[x][j+3];
                    x5=A[x][j+4],x6=A[x][j+5],x7=A[x][j+6],x8=A[x][j+7];
                    B[j][x]=x1,B[j+1][x]=x2,B[j+2][x]=x3,B[j+3][x]=x4;
                    B[j][x+4]=x5,B[j+1][x+4]=x6,B[j+2][x+4]=x7,B[j+3][x+4]=x8;
                    
                }
                for(y=j;y<j+4;y++){
                    x1=A[i+4][y],x2=A[i+5][y],x3=A[i+6][y],x4=A[i+7][y];
                    x5=B[y][i+4],x6=B[y][i+5],x7=B[y][i+6],x8=B[y][i+7];

                    B[y][i+4]=x1,B[y][i+5]=x2,B[y][i+6]=x3,B[y][i+7]=x4;
                    //把存在B中的右上角的数字往左下角挪,减去偏移量
                    B[y+4][i]=x5,B[y+4][i+1]=x6,B[y+4][i+2]=x7,B[y+4][i+3]=x8;
                }
                for(x=i+4;x<i+8;x++)
                // for(y=j+4;y<j+8;y++)
                //   B[x][y]=A[y][x];
                {
                    x1=A[x][j+4],x2=A[x][j+5],x3=A[x][j+6],x4=A[x][j+7];
                    B[j+4][x]=x1,B[j+5][x]=x2,B[j+6][x]=x3,B[j+7][x]=x4;
                }
            }
        }
}
char trans_7_desc[]="对于61*67在这里尝试各种各样的分块4*4";
void trans_7(int M, int N, int A[N][M], int B[M][N]){
    int i,j,x,y;
    for(i=0;i<N;i+=4)
    for(j=0;j<M;j+=4){
        for(x=i;x<i+4&&x<N;x++){
            for(y=j;y<j+4&&y<M;y++)
            B[y][x]=A[x][y];
        }
    }
}
char trans_8_desc[]="对于61*67在这里尝试各种各样的分块8*8";
void trans_8(int M, int N, int A[N][M], int B[M][N]){
    int i,j,x,y;
    for(i=0;i<N;i+=8)
    for(j=0;j<M;j+=8){
        for(x=i;x<i+8&&x<N;x++){
            for(y=j;y<j+8&&y<M;y++)
            B[y][x]=A[x][y];
        }
    }
}
char trans_9_desc[]="对于61*67在这里尝试各种各样的分块14*14";
void trans_9(int M, int N, int A[N][M], int B[M][N]){
    int i,j,x,y;
    for(i=0;i<N;i+=14)
    for(j=0;j<M;j+=14){
        for(x=i;x<i+14&&x<N;x++){
            for(y=j;y<j+14&&y<M;y++)
            B[y][x]=A[x][y];
        }
    }
}
char trans_10_desc[]="对于61*67在这里尝试各种各样的分块15*15";
void trans_10(int M, int N, int A[N][M], int B[M][N]){
    int i,j,x,y;
    for(i=0;i<N;i+=15)
    for(j=0;j<M;j+=15){
        for(x=i;x<i+15&&x<N;x++){
            for(y=j;y<j+15&&y<M;y++)
            B[y][x]=A[x][y];
        }
    }
}
char trans_11_desc[]="对于61*67在这里尝试各种各样的分块16*16";
void trans_11(int M, int N, int A[N][M], int B[M][N]){
    int i,j,x,y;
    for(i=0;i<N;i+=16)
    for(j=0;j<M;j+=16){
        for(x=i;x<i+16&&x<N;x++){
            for(y=j;y<j+16&&y<M;y++)
            B[y][x]=A[x][y];
        }
    }
}
char trans_12_desc[]="对于61*67在这里尝试各种各样的分块17*17";
void trans_12(int M, int N, int A[N][M], int B[M][N]){
    int i,j,x,y;
    for(i=0;i<N;i+=17)
    for(j=0;j<M;j+=17){
        for(x=i;x<i+17&&x<N;x++){
            for(y=j;y<j+17&&y<M;y++)
            B[y][x]=A[x][y];
        }
    }
}
char trans_test_desc[]="4*4的测试";
void trans_test(int M,int N,int A[N][N],int B[N][N]){
    int x1,x2,x3,x4,x5,x6,x7,x8,i;
for( i=0;i<3;i+=2){
     x1=A[i][0];
     x2=A[i][1];
     x3=A[i][2];
     x4=A[i][3];
     x5=A[i+1][0];
     x6=A[i+1][1];
     x7=A[i+1][2];
     x8=A[i+1][3];
     B[0][i]=x1;
     B[1][i]=x2;
     B[2][i]=x3;
     B[3][i]=x4;
     B[0][i+1]=x5;
     B[1][i+1]=x6;
     B[2][i+1]=x7;
     B[3][i+1]=x8;
}
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans_test, trans_test_desc);
   // registerTransFunction(trans_1, trans_1_desc);
   // registerTransFunction(trans_2, trans_2_desc); 
    //  registerTransFunction(trans_3, trans_3_desc);
   // registerTransFunction(trans_4, trans_4_desc); 
   // registerTransFunction(trans_5, trans_5_desc);
   // registerTransFunction(trans_6, trans_6_desc);
    //   registerTransFunction(trans_7, trans_7_desc);  
    //   registerTransFunction(trans_8, trans_8_desc); 
    //   registerTransFunction(trans_9, trans_9_desc); 
    //    registerTransFunction(trans_10, trans_10_desc); 
    //     registerTransFunction(trans_11, trans_11_desc); 
     //  registerTransFunction(trans_12, trans_12_desc); 


}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

