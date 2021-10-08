#include <iostream>
#include <cstdlib>
#include <chrono>
#include <time.h>
#include <omp.h>
#include "mpi.h"


using namespace std;
using namespace std::chrono;

// Variables
#define MAX_THREADS 16
const int SIZE = 400;
int **Matrix1, **Matrix2, **Matrix3;


// Functions

// Function to initialize the matrices and fill it with random number
void init_matrix(int **&matrix, int rows, int cols, bool initialize);

// Function for the head to do the calculation
void head(int total_processes);

// Function for the node to do the calculation
void node(int total_processes, int rank);

// Function to multiply matrix 1 and matrix 2
void multiply(int** matrix1, int** matrix2, int** matrix3, int num_rows);

// Function to print the matrix
void print_matrix(int** matrix, int rows, int cols);

int main(int argc, char **argv)
{
    srand(time(0));

    int total_processes; // Total number of processes
    int rank;            // Rank of each of the processes

    // Setting the number of threads to use
    omp_set_num_threads(MAX_THREADS);

    // Initializing the MPI Environment
    MPI_Init(&argc, &argv);

    // Determines the total number of processes running in parallel
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    // Determines the rank of the calling processor
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) head(total_processes);
    
    else node(total_processes, rank);

    // Terminate the MPI Environment
    MPI_Finalize();
}

void head(int total_processes)
{
    init_matrix(Matrix1, SIZE, SIZE, true);
    init_matrix(Matrix2, SIZE, SIZE, true);
    init_matrix(Matrix3, SIZE, SIZE, false);

    //print_matrix(Matrix1, SIZE, SIZE);
    //print_matrix(Matrix2, SIZE, SIZE);

    // Calculate the decomposition
    int num_rows = SIZE / total_processes;  // Number of rows per proccess
    int broadcast_size = (SIZE * SIZE);  // Number of elements to broadcast
    int scatter_gather_size = (SIZE * SIZE) / total_processes; // Number of element to scatter

    // Starts the timer
    auto start = high_resolution_clock::now();

    // Start distributing the data
    // Scatter matrix 1 to all the nodes
    MPI_Scatter(&Matrix1[0][0], scatter_gather_size, MPI_INT, &Matrix1, 0, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Broadcast matrix 2 to all the nodes
    MPI_Bcast(&Matrix2[0][0], broadcast_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Use OpenMP and share Matrix1, Matrix2, Matrix3, num_rows to run parallel
    #pragma omp parallel default(none) shared(Matrix1, Matrix2, Matrix3, num_rows)
    {
        // Multiply matrix 1 and matrix 2 into matrix3
        multiply(Matrix1, Matrix2, Matrix3, num_rows);
    }

    // Wait and gather all nodes, then put M3 data into main
    MPI_Gather(MPI_IN_PLACE, scatter_gather_size, MPI_INT, &Matrix3[0][0], scatter_gather_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Get the current time when its stopped
    auto stop = high_resolution_clock::now();

    // Returns the time it takes for the function to run
    auto duration = duration_cast<microseconds>(stop - start);

    //print_matrix(Matrix3, SIZE, SIZE);

    cout << "Time taken by function: " << duration.count() << " Microseconds" << endl;
}

void node(int total_processes, int rank)
{
    // Calculate the decomposition
    int num_rows = SIZE / total_processes;  // Number of rows per proccess
    int broadcast_size = (SIZE * SIZE);  // Number of elements to broadcast
    int scatter_gather_size = (SIZE * SIZE) / total_processes; // Number of element to scatter

    // Initialize arrays which will receive Matrix1, Matrix2, and Matrix3
    init_matrix(Matrix1, SIZE, SIZE, true);
    init_matrix(Matrix2, SIZE, SIZE, false);
    init_matrix(Matrix3, SIZE, SIZE, false);

    // Receive Matrix1 and Matrix2 from the head
    MPI_Scatter(NULL, scatter_gather_size, MPI_INT, &Matrix1[0][0], scatter_gather_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Matrix2[0][0], broadcast_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Run the multiplication in parallel using OpenMP
    #pragma omp parallel default(none) shared(Matrix1, Matrix2, Matrix3, num_rows)
    {
        // Multiply the given matrices
        multiply(Matrix1, Matrix2, Matrix3, num_rows);
    }

    // Gather the calculated matrix
    MPI_Gather(&Matrix3[0][0], scatter_gather_size, MPI_INT, NULL, scatter_gather_size, MPI_INT, 0, MPI_COMM_WORLD);
}

void init_matrix(int **&matrix, int rows, int cols, bool initialize)
{   
    // Allocate the memory to the arrays
    matrix = (int **) malloc(sizeof(int*) *rows * cols);
    int* temp_matrix = (int *) malloc(sizeof(int) * cols * rows);

    for (int i = 0; i < SIZE; ++i)
    {
        matrix[i] = &temp_matrix[i * cols];
    }

    if (!initialize) return;

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            matrix[i][j] = rand() % 100;
        }
    }
}   

// Function to multiply matrix 1 and matrix 2
void multiply(int** matrix1, int** matrix2, int** matrix3, int num_rows)
{   
    #pragma omp for
    for (int i = 0; i < num_rows; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            matrix3[i][j] = 0;

            for (int k = 0; k < SIZE; ++k)
            {
                matrix3[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
}

void print_matrix(int** matrix, int rows, int cols)
{
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("--------------\n");
}