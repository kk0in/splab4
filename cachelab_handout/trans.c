// name: 김영인, loginID: 2017-14384
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
    int bi, bj, i, j; // i: row, j: column, bi: block row, bj: block column
    int a0, a1, a2, a3, a4, a5, a6, a7; // 값을 잠깐 저장하기 위한 변수들

    // s=5, E=1, b=5 -> set 총 개수: 32, line 크기: 32byte(8 ints), direct mapped cache 
    
    // 32*32
    // 8*8 block으로 나눠서 해결한다.
    // i와 j가 같을 때, set index가 동일하기 때문에 이전의 참조에 의해 복사된 값들이 지워져서 cache miss rate가 증가한다. 따라서 둘이 같은 경우를 연산의 마지막에 따로 처리해주면 된다.
    if(M==32 && N==32) {
	for(bi = 0; bi < N; bi += 8) {
	    for(bj = 0; bj < M; bj += 8) {
		// row를 우선으로 하여 순회하는 것이 column을 우선으로 하여 순회하는 것보다 적은 cache miss를 일으킨다. 
		for(i = bi; (i<bi+8)&&(i<N); i++) {
		    for(j = bj; (j<bj+8)&&(j<M); j++) {
			// 대각선에서는 transpose를 진행하지 않고 값을 저장해둔다.
			if (i == j) {
			    a1 = A[i][j];
			    a0 = j;
			}
			else {
			    B[j][i] = A[i][j];
			}
		    }
		    // 아까 저장해놓은 값을 이용하여 대각선 성분들의 값을 채워준다.
		    if(bj == bi) {
			B[a0][a0] = a1;
		    }
		}
	    }
	}
    }

    // 61*67
    // 16*16 block으로 나누었을 때 위와 동일한 문제가 발생하므로, 위와 거의 유사한 방법으로 해결이 가능하다.
    if(M==61 && N==67) {
	for(bi = 0; bi < N; bi += 16) {
            for(bj = 0; bj < M; bj += 16) {
                for(i = bi; (i<bi+16)&&(i<N); i++) {
                    for(j = bj; (j<bj+16)&&(j<M); j++) {
                        if(i==j) {
			    a1 = A[i][j];
			    a0 = j;
                        }
                        else {
                            B[j][i] = A[i][j];
                        }
                    }
                    if(bj == bi) {
                        B[a0][a0] = a1;
                    }
                }
            }
        }
    }

    // 64*64
    // 가까운 위치에 있는 element끼리 같은 set index를 공유하는 경우가 많다
    // 8*8 block으로 나눠서 해결
    if(M==64 && N==64){
	for(bi = 0; bi < N; bi += 8) {
            for(bj = 0; bj < M; bj += 8) {
		// block 절반의 위쪽이set index가 겹치는 문제가 있어서 따로 처리
		// row 우선 순위로 하여 진행
                for(i = bi; (i<bi+4)&&(i<N); i++) {
                    a7 = A[i][bj+7];
		    a6 = A[i][bj+6];
		    a5 = A[i][bj+5];
		    a4 = A[i][bj+4];
		    a3 = A[i][bj+3];
		    a2 = A[i][bj+2];
		    a1 = A[i][bj+1];
		    a0 = A[i][bj];
                    
		    // 이 과정에서 cache freindly 위해 바로 transpose를행 요소에 대해 모두 하지 않고 절반만 진행, 나머지 절반은 오른쪽 블럭의 첫번째 열에 잠시 저장
		    // transpose 정의에 아직 맞지 않는 부분-> 뒤에서 처리해줄 예정
                    B[bj+3][i+4] = a7;
		    B[bj+2][i+4] = a6;
		    B[bj+1][i+4] = a5;
		    B[bj][i+4] = a4;
		    // transpose 정의에 맞는 부분
		    B[bj+3][i] = a3;
		    B[bj+2][i] = a2;
		    B[bj+1][i] = a1;
		    B[bj][i] = a0;
                }
		// 다시 원래 transpose 정의대로 바꿔주는 부분 + 아래 절반 블록 처리
                for(j = bj; (j<bj+4)&&(j<M); j++) {
		    // set index issue를 피하기 위해 아래 절반 블록 왼쪽 부분을 열 단위로 저장
                    a7 = A[bi+7][j];
		    a6 = A[bi+6][j];
		    a5 = A[bi+5][j];
		    a4 = A[bi+4][j];
		    // 아까 B에 저장했던 부분을 이용해 원래 transpose 정의에 맞는 자리로 재배치하기 위해 값 저장
                    a3 = B[j][bi+7];
		    a2 = B[j][bi+6];
		    a1 = B[j][bi+5];
		    a0 = B[j][bi+4];

		    // 위에서 저장한 값 이용해서 transpose 진행
                    B[j][bi+7] = a7;
		    B[j][bi+6] = a6;
		    B[j][bi+5] = a5;
		    B[j][bi+4] = a4;

		    // 원래 자리에 맞게 재배치
                    B[j+4][bi+3] = a3;
		    B[j+4][bi+2] = a2;
		    B[j+4][bi+1] = a1;
		    B[j+4][bi] = a0;

		    // 아래 절반 블록의 오른쪽 부분은 set index issue가 없어서 transpose 정의대로 바로 처리를 해주면 된다.
		    for(int k=7; k>3; k--) {
			B[j+4][bi+k] = A[bi+k][j+4];
		    }
		}
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

