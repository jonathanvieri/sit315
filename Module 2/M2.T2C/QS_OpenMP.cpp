#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>

using namespace std::chrono;
using namespace std;

const int N = 2000000;
const int MAX_THREADS = 16;

// Function to fill the array with random value between 1 - 99
void fillArray(int array[])
{
    for (int i = 0; i < N; ++i)
    {
        array[i] = rand();
    }
}

// Function that will swap the specified two elements
void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to print the array
void printArray(int array[], int size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

// Function that will choose a pivot and put it at the correct position in the array
// It will put bigger elements to its right then put the smaller element to its left
int partition(int array[], int min, int max)
{
    int pivot = array[max];
    int i = min - 1;

    for (int j = min; j <= max - 1; j++)
    {
        if (array[j] < pivot)
        {
            i++;
            swap(&array[i], &array[j]);
        }
    }
    swap(&array[i + 1], &array[max]);

    return (i + 1); // Will return the center point of the pivot
}

// The quicksort function that implements the partition function
// It works by recursively iterating through given array and two endpoints
void quickSort(int array[], int min, int max)
{
    if (min < max)
    {
        // Get the partition index of the array
        // It will split the lower values than max to the left
        // And greater values to the right
        int index = partition(array, min, max);

#pragma omp task shared(array)
        {
            quickSort(array, min, index - 1); // Before the partition index
        }
#pragma omp task shared(array)
        {
            quickSort(array, index + 1, max); // After the partition index
        }
    }
}

int main()
{
    srand(time(0));

    // Dynamically allocate memory to the array,
    // We are doing this to prevent overflowing when the array size is too big
    int *array = (int *)malloc(N * sizeof(int *));

    omp_set_num_threads(MAX_THREADS);

    // size_t size = sizeof(array) / sizeof(array[0]); // Length of the array

    fillArray(array); // Fill the array

    // printf("Array before sorted: \n");
    // printArray(array, size); // Prints the array

    // Starts the timer
    auto start = high_resolution_clock::now();

#pragma omp parallel
    {
#pragma omp single // Initially, it needs to be run only on a single thread
        {
            quickSort(array, 0, N - 1);
        }
    }

    // Get the current time when its stopped
    auto stop = high_resolution_clock::now();

    // Returns the time it takes for the function to run
    auto duration = duration_cast<microseconds>(stop - start);

    // printf("Array after sorted: \n");
    // printArray(array, size); // Prints the array

    printf("Time taken by the function: %d microseconds", duration.count());

    return 0;
}