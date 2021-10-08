// The purpose of this function is to calculate a matrix multiplication
__kernel void matrix_multiply(__global const int *matrix1, __global const int *matrix2, __global int *matrix3, const int size)
{
    // Thread indentifiers
    const int global_index1 = get_global_id(0);
    const int global_index2 = get_global_id(1);
    const int global_index3 = get_global_id(2);

    // Convert the indentified thread for indexing the matrices
    const int i = (global_index1 * size) + global_index3;
    const int j = (global_index3 * size) + global_index2; 
    const int k = (global_index1 * size) + global_index2;

    // Multiply from two matrices and add the result to matrix 3
    matrix3[k] += (matrix1[i] * matrix2[j]);  
}