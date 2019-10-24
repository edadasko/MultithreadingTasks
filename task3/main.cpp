#include <chrono>
#include <iostream>
#include <omp.h>

using namespace std;
const int N = 512;
const int chunk = 16;

void initMatrix(double* M) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            M[i * N + j] = rand();
}

    
int main() {
    double sum;
    int i = 0, j = 0, k = 0;
    double* matrixA = new double[N * N];
    double* matrixB = new double[N * N];
    double* matrixC = new double[N * N];
    
    initMatrix(matrixA);
    initMatrix(matrixB);
    
    auto begin = chrono::steady_clock::now();
    #pragma omp parallel for private(i, j, k, sum) schedule(static,chunk)
    for (i = 0; i < N; i++) {
        for (k = 0; k < N; k++) {
            sum = 0;
            for (j = 0; j < N; j++) {
                sum += matrixA[i * N + j] * matrixB[j * N + k];
            }
            matrixC[i * N + k] = sum;
        }
    }
    
    auto end = chrono::steady_clock::now();
    auto time = chrono::duration_cast<chrono::milliseconds>(end - begin);
    cout << "Время работы параллельного алгоритма: " << (double)time.count() / 1000 << endl;
    
    double* matrixD = new double[N * N];
    
    begin = chrono::steady_clock::now();
    for (i = 0; i < N; i++) {
        for (k = 0; k < N; k++) {
            sum = 0;
            for (j = 0; j < N; j++) {
                sum += matrixA[i * N + j] * matrixB[j * N + k];
            }
            matrixD[i * N + k]=sum;
        }
    }
    
    end = chrono::steady_clock::now();
    time = chrono::duration_cast<chrono::milliseconds>(end - begin);
    cout << "Время работы последовательного алгоритма: " << (double)time.count() / 1000 << endl;
    
    for (i = 0; i < N; i ++)
        assert(matrixC[i] == matrixD[i]);
    
    delete[] matrixA;
    delete[] matrixB;
    delete[] matrixC;
    delete[] matrixD;
}
