// File: Matrix.c
// Implementation of the Matrix ADT

#include "Matrix.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct matrix_st {
    size_t rows;
    size_t cols;
    float **data;  // 2D array: data[row-1][col-1]
};

static float **alloc_2d(size_t rows, size_t cols) {
    float **arr = malloc(rows * sizeof(float *));
    if (!arr) return NULL;

    for (size_t i = 0; i < rows; ++i) {
        arr[i] = malloc(cols * sizeof(float));
        if (!arr[i]) {
            // Clean up previously allocated rows
            for (size_t j = 0; j < i; ++j) {
                free(arr[j]);
            }
            free(arr);
            return NULL;
        }
    }
    return arr;
}

static void free_2d(float **arr, size_t rows) {
    if (!arr) return;
    for (size_t i = 0; i < rows; ++i) {
        free(arr[i]);
    }
    free(arr);
}

static bool valid_cell(const Matrix mat, size_t row, size_t col) {
    return row >= 1 && row <= mat->rows && col >= 1 && col <= mat->cols;
}

static bool valid_row(const Matrix mat, size_t row) {
    return row >= 1 && row <= mat->rows;
}

Matrix mat_create(size_t rows, size_t cols) {
    if (rows == 0 || cols == 0) return NULL;

    Matrix mat = malloc(sizeof(struct matrix_st));
    if (!mat) return NULL;

    mat->rows = rows;
    mat->cols = cols;
    mat->data = alloc_2d(rows, cols);
    if (!mat->data) {
        free(mat);
        return NULL;
    }

    // Initialize to zero
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            mat->data[i][j] = 0.0f;
        }
    }

    // If square, set to identity
    if (rows == cols) {
        for (size_t i = 0; i < rows; ++i) {
            mat->data[i][i] = 1.0f;
        }
    }

    return mat;
}

void mat_destroy(Matrix mat) {
    if (mat) {
        free_2d(mat->data, mat->rows);
        free(mat);
    }
}

void mat_init(Matrix mat, const float data[]) {
    if (!mat || !data) return;

    for (size_t i = 0; i < mat->rows; ++i) {
        for (size_t j = 0; j < mat->cols; ++j) {
            mat->data[i][j] = data[i * mat->cols + j];
        }
    }
}

Matrix mat_duplicate(const Matrix mat) {
    if (!mat) return NULL;

    Matrix dup = mat_create(mat->rows, mat->cols);
    if (!dup) return NULL;

    for (size_t i = 0; i < mat->rows; ++i) {
        memcpy(dup->data[i], mat->data[i], mat->cols * sizeof(float));
    }

    return dup;
}

bool mat_equals(const Matrix m1, const Matrix m2) {
    if (!m1 || !m2) return false;
    if (m1->rows != m2->rows || m1->cols != m2->cols) return false;

    for (size_t i = 0; i < m1->rows; ++i) {
        for (size_t j = 0; j < m1->cols; ++j) {
            if (m1->data[i][j] != m2->data[i][j]) return false;
        }
    }
    return true;
}

void mat_scalar_mult(Matrix mat, float data) {
    if (!mat) return;

    for (size_t i = 0; i < mat->rows; ++i) {
        for (size_t j = 0; j < mat->cols; ++j) {
            mat->data[i][j] *= data;
        }
    }
}

Matrix mat_mult(const Matrix m1, const Matrix m2) {
    if (!m1 || !m2 || m1->cols != m2->rows) return NULL;

    size_t M = m1->rows;
    size_t N = m2->cols;
    size_t K = m1->cols;

    Matrix result = mat_create(M, N);
    if (!result) return NULL;

    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < K; ++k) {
                sum += m1->data[i][k] * m2->data[k][j];
            }
            result->data[i][j] = sum;
        }
    }

    return result;
}

Status mat_get_cell(const Matrix mat, float *data, size_t row, size_t col) {
    if (!mat || !data) return BadRowNumber;
    if (!valid_cell(mat, row, col)) return (row < 1 || row > mat->rows) ? BadRowNumber : BadColNumber;

    *data = mat->data[row - 1][col - 1];
    return Success;
}

Status mat_get_row(const Matrix mat, float data[], size_t row) {
    if (!mat || !data) return BadRowNumber;
    if (!valid_row(mat, row)) return BadRowNumber;

    memcpy(data, mat->data[row - 1], mat->cols * sizeof(float));
    return Success;
}

Status mat_set_cell(Matrix mat, float data, size_t row, size_t col) {
    if (!mat) return BadRowNumber;
    if (!valid_cell(mat, row, col)) return (row < 1 || row > mat->rows) ? BadRowNumber : BadColNumber;

    mat->data[row - 1][col - 1] = data;
    return Success;
}

Status mat_set_row(Matrix mat, const float data[], size_t row) {
    if (!mat || !data) return BadRowNumber;
    if (!valid_row(mat, row)) return BadRowNumber;

    memcpy(mat->data[row - 1], data, mat->cols * sizeof(float));
    return Success;
}

Matrix mat_transpose(const Matrix mat) {
    if (!mat) return NULL;

    Matrix trans = mat_create(mat->cols, mat->rows);
    if (!trans) return NULL;

    for (size_t i = 0; i < mat->rows; ++i) {
        for (size_t j = 0; j < mat->cols; ++j) {
            trans->data[j][i] = mat->data[i][j];
        }
    }

    return trans;
}

void mat_print(const Matrix mat, FILE *stream) {
    if (!mat || !stream) return;

    fprintf(stream, "%zu rows, %zu columns:\n", mat->rows, mat->cols);
    for (size_t i = 0; i < mat->rows; ++i) {
        for (size_t j = 0; j < mat->cols; ++j) {
            fprintf(stream, "%8.3f", mat->data[i][j]);
        }
        fputc('\n', stream);
    }
}

