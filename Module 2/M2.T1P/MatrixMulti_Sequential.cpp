#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <fstream>
#include <omp.h>

using namespace std::chrono;
using namespace std;

const int N = 400;

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

void multiply_Matrix(int matrix1[][N], int matrix2[][N], int matrix3[][N])
{
    for (int i = 0; i < N; ++i)
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
}

int main()
{
    srand(time(0));

    // Create the matrices
    int matrix1[N][N];
    int matrix2[N][N];
    int matrix3[N][N];

    // Filling the matrices
    fill_Matrix(matrix1);
    fill_Matrix(matrix2);

    // Printing the matrices
    //print_Matrix(matrix1);
    //print_Matrix(matrix2);

    // Starts the timer
    auto start = high_resolution_clock::now();

    // Multiplying the matrix
    multiply_Matrix(matrix1, matrix2, matrix3);

    // Get the current time when its stopped
    auto stop = high_resolution_clock::now();

    // Printing the result matrix
    //print_Matrix(matrix3);

    // Returns the time it takes for the function to run
    auto duration = duration_cast<microseconds>(stop - start);

    printf("Time taken by the function: %d microseconds", duration.count());

    ofstream myFile;
    myFile.open("Sequential_Results.txt");

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