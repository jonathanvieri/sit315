#include <iostream>
#include <cstdlib>
#include <chrono>
#include <time.h>
#include <CL/cl.h>
#include "mpi.h"

using namespace std::chrono;
using namespace std;

// Variables
const int SIZE = 400;
int **Matrix1, **Matrix2, **Matrix3;

// OpenCL Variables
cl_mem bufM1, bufM2, bufM3; 
cl_device_id device_id;
cl_context context;
cl_program program;
cl_kernel kernel;
cl_command_queue queue;
cl_event event = NULL;
int err;


// Functions

// Function to initialize the matrices and fill it with random number
void init_matrix(int **&matrix, int rows, int cols, bool initialize);

// Function for the head to do the calculation
void head(int total_processes);

// Function for the node to do the calculation
void node(int total_processes, int rank);

// Function to print the matrix
void print_matrix(int **matrix, int rows, int cols);


// OpenCL Function

// This function will search for a GPU associated with the first available platform
// It will first check the platform and then check if it can access the GPU
// If it's unable to access the GPU it will try and get the CPU
// If it got the CPU it will return its device id, else it will exit
cl_device_id create_device();

// This function is used to set up OpenCL device, context, queue, and kernel
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);

// This function is used to create and compile a OpenCL program
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);

// This function is used to set up the kernel memory
// It will create and write to a buffer object from host memory
void setup_kernel_memory();

// This function is used to copy kernel arguments
void copy_kernel_args();

// This function is used to free the memory for device, kernel, queue, etc
void free_memory();

// This function is used to run OpenCL
void startOpenCL(int num_rows);

int main(int argc, char **argv)
{
    srand(time(0));

    int total_processes; // Total number of processes
    int rank;            // Rank of each of the processes

    // Initializing the MPI Environment
    MPI_Init(&argc, &argv);

    // Determines the total number of processes running in parallel
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    // Determines the rank of the calling processor
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
        head(total_processes);

    else
        node(total_processes, rank);

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
    int num_rows = SIZE / total_processes;                     // Number of rows per proccess
    int broadcast_size = (SIZE * SIZE);                        // Number of elements to broadcast
    int scatter_gather_size = (SIZE * SIZE) / total_processes; // Number of element to scatter

    // Starts the timer
    auto start = high_resolution_clock::now();

    // Start distributing the data
    // Scatter matrix 1 to all the nodes
    MPI_Scatter(&Matrix1[0][0], scatter_gather_size, MPI_INT, &Matrix1, 0, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast matrix 2 to all the nodes
    MPI_Bcast(&Matrix2[0][0], broadcast_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Multiply matrix 1 and matrix 2 into matrix3 using OpenCL
    startOpenCL(num_rows);

    // Wait and gather all nodes, then put M3 data into main
    MPI_Gather(MPI_IN_PLACE, scatter_gather_size, MPI_INT, &Matrix3[0][0], scatter_gather_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Get the current time when its stopped
    auto stop = high_resolution_clock::now();

    // Returns the time it takes for the function to run
    auto duration = duration_cast<microseconds>(stop - start);

    //print_matrix(Matrix3, SIZE, SIZE);

    cout << "Time taken by function: " << duration.count() << " Microseconds" << endl;

    // Free the memory for device, kernel, queue, etc
    free_memory();
}

void node(int total_processes, int rank)
{
    // Calculate the decomposition
    int num_rows = SIZE / total_processes;                     // Number of rows per proccess
    int broadcast_size = (SIZE * SIZE);                        // Number of elements to broadcast
    int scatter_gather_size = (SIZE * SIZE) / total_processes; // Number of element to scatter

    // Initialize arrays which will receive Matrix1, Matrix2, and Matrix3
    init_matrix(Matrix1, SIZE, SIZE, true);
    init_matrix(Matrix2, SIZE, SIZE, false);
    init_matrix(Matrix3, SIZE, SIZE, false);

    // Receive Matrix1 and Matrix2 from the head
    MPI_Scatter(NULL, scatter_gather_size, MPI_INT, &Matrix1[0][0], scatter_gather_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Matrix2[0][0], broadcast_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Multiply the given matrices using OpenCL
    startOpenCL(num_rows);

    // Gather the calculated matrix
    MPI_Gather(&Matrix3[0][0], scatter_gather_size, MPI_INT, NULL, scatter_gather_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Free the memory for device, kernel, queue, etc
    free_memory();
}

void init_matrix(int **&matrix, int rows, int cols, bool initialize)
{
    // Allocate the memory to the arrays
    matrix = (int **)malloc(sizeof(int *) * rows * cols);
    int *temp_matrix = (int *)malloc(sizeof(int) * cols * rows);

    for (int i = 0; i < SIZE; ++i)
    {
        matrix[i] = &temp_matrix[i * cols];
    }

    if (!initialize)
        return;

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            matrix[i][j] = rand() % 100;
        }
    }
}

void print_matrix(int **matrix, int rows, int cols)
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

// OpenCL Functions

void free_memory()
{
    //free the buffers
    clReleaseMemObject(bufM1);
    clReleaseMemObject(bufM2);
    clReleaseMemObject(bufM3);

    //free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(Matrix1);
    free(Matrix2);
    free(Matrix3);
}

void copy_kernel_args()
{
    // The purpose of this function is to set the argument value for a specific argument of a kernel
    // It takes the following arguments:
    //  A kernel object,
    //  Argument index,
    //  Size of the argument value,
    //  Pointer to data that should be used as the argument value for argument specified by arg_index

    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SIZE);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufM1);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&bufM2);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&bufM3);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory(int num_rows)
{
    // The purpose of this function is to create a buffer object
    // It takes the following arguments:
    //  A OpenCL Context used to create the buffer object,
    //  A bit-field that is used to specify allocation and usage information such as the memory arena,
    //  Size in bytes of the buffer memory object to be allocated,
    //  Pointer to the buffer data that may already be allocated by the application,
    //  Return an appropriate error code, if set to NULL no error code will be returned

    // The second parameter accepts 10 memory flag. If the specified value for memory flag is 0,
    // it will use the default CL_MEM_READ_WRITE memory flag
    bufM1 = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * num_rows * sizeof(int), NULL, NULL);
    bufM2 = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * SIZE * sizeof(int), NULL, NULL);
    bufM3 = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE * SIZE * sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufM1, CL_TRUE, 0, SIZE * num_rows * sizeof(int), &Matrix1[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufM2, CL_TRUE, 0, SIZE * SIZE * sizeof(int), &Matrix2[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufM3, CL_TRUE, 0, SIZE * num_rows * sizeof(int), &Matrix3[0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device();
    cl_int err;

    // The purpose of this function is to create an OpenCL Context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    // The purpose of this function is to create a host or device command-queue on a specific device
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0)
    {
        perror("Couldn't create a command queue");
        exit(1);
    };

    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    };
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    /* Read program file and place content into buffer */
    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    // The purpose of this function is to create a program object for a context,
    // and loads the source code specified by text strings into the program object

    // It takes the following arguments:
    //  A valid OpenCL Context,
    //  A count pointer,
    //  Array of count pointers to optionally null-terminated character strings that make up the source code,
    //  Array with the number of chars in each string (The string length),
    //  Returns an appropriate error code, and if its NULL no error code will be returned
    program = clCreateProgramWithSource(ctx, 1,
                                        (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    /* Build program 

   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

cl_device_id create_device()
{

    cl_platform_id platform;
    cl_device_id dev;
    int err;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err < 0)
    {
        perror("Couldn't identify a platform");
        exit(1);
    }

    // Access a device
    // GPU
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        // CPU
        printf("GPU not found\n");
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    }
    if (err < 0)
    {
        perror("Couldn't access any devices");
        exit(1);
    }

    return dev;
}

void startOpenCL(int num_rows)
{
    // Set number of global variables
    size_t global[3] = {(size_t) num_rows, (size_t) SIZE, (size_t) SIZE};    

    // Setup OpenCL Device kernel
    setup_openCL_device_context_queue_kernel((char *)"./MatrixMultiply.cl", (char *)"matrix_multiply");

    // Setup the kernel memory
    setup_kernel_memory(num_rows);

    // Set kernel arguments
    copy_kernel_args();

    // Enqueue data to the kernel and wait for it to run
    clEnqueueNDRangeKernel(queue, kernel, 3, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);

    // Read the returned data
    clEnqueueReadBuffer(queue, bufM3, CL_TRUE, 0, SIZE * num_rows * sizeof(int), &Matrix3[0], 0, NULL, NULL);
}
