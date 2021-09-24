#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>
#include <fstream>
#include <pthread.h>

using namespace std::chrono;
using namespace std;

// Variables
const int N = 400;
#define MAX_THREADS 16

// Matrix Global Variables
int matrix1[N][N];
int matrix2[N][N];
int matrix3[N][N];
int thread_num = 0;
int partition_size = N / MAX_THREADS;

// Function to fill a Matrix with random values between 1-99
void fill_Matrix(int matrix[][N])
{
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            // Fill the matrix with random value between 1-99
            matrix[i][j] = rand() % 100;
        }
    }
}

// Function to print a Matrix
void print_Matrix(int matrix[][N])
{
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void *multiplyMatrix_pthread(void *args)
{
    // Get the current thread and the partition size
    int thread = thread_num++;

    for (int i = thread * partition_size; i < (thread + 1) * partition_size; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            matrix3[i][j] = 0;

            for (int k = 0; k < N; ++k)
            {
                matrix3[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    return NULL;
}

int main()
{
    // To randomize the rand() function
    srand(time(0));

    // Filling the matrices
    fill_Matrix(matrix1);
    fill_Matrix(matrix2);

    // Print the matrices
    // printf("Matrix 1: \n");
    // print_Matrix(matrix1);
    // printf("Matrix 2: \n");
    // print_Matrix(matrix2);

    // Starts the timer
    auto start = high_resolution_clock::now();

    // Initializing the threads based on number of MAX_THREADS
    pthread_t matrix_thread[MAX_THREADS];

    // Create the threads
    for (int i = 0; i < MAX_THREADS; ++i)
    {
        pthread_create(&matrix_thread[i], NULL, &multiplyMatrix_pthread, NULL);
    }

    // Join the threads
    for (int i = 0; i < MAX_THREADS; ++i)
    {
        pthread_join(matrix_thread[i], NULL);
    }

    // Get the current time when its stopped
    auto stop = high_resolution_clock::now();

    // Returns the time it takes for the function to run
    auto duration = duration_cast<microseconds>(stop - start);

    //print_Matrix(matrix3);

    printf("Time taken by the function: %d microseconds", duration.count());

    ofstream myFile;
    myFile.open("PThread_Results.txt");

    // Writing the result matrix to a file
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            myFile << matrix3[i][j] << "\t";
        }
        myFile << "\n";
    }

    myFile << "Time taken by the function: " << duration.count();

    myFile.close();

    return 0;
}