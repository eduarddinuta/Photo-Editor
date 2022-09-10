// Copyright Dinuta Eduard-Stefan 311CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifndef _FUNCTIONS_
#define _FUNCTIONS

#define NMAX 100

// struct for all colors of a pixel
// for grayscale only r is used
typedef struct {
	double r, g, b;
} pixel_t;

// struct for defining the image
typedef struct {
	int lin, col;
	pixel_t **a;
	int type;
	int max;
} matrix_t;

void swap(int *a, int *b);

void copy_pixel(pixel_t *a, pixel_t *b);

char *alloc_string(int size);

pixel_t **alloc_matrix(int lin, int col);

void free_matrix(matrix_t M);

void jump_comm(FILE *f, unsigned char *ch);

void get_matrix(char *filename, matrix_t *M);

int verify(int x1, int y1, int x2, int y2, matrix_t M);

void transpose(int *l1, int *c1, int *l2, int *c2, matrix_t *M);

void swap_pixel(pixel_t *a, pixel_t *b);

void reverse_rows(int l1, int c1, int l2, int c2, matrix_t M);

void rotate_90(int *l1, int *c1, int *l2, int *c2, matrix_t *M, int number);

void rotation(int *l1, int *c1, int *l2, int *c2, matrix_t *M, char *line);

void crop(int *l1, int *c1, int *l2, int *c2, matrix_t *M);

void save(matrix_t M, char *filename, int ascii);

int is_in_matrix(int x, int y, int lin, int col);

double clamp(double x, double min, double max);

void apply(int l1, int c1, int l2, int c2, matrix_t M, double ker[3][3], int x);

void apply_filter(int l1, int c1, int l2, int c2, matrix_t M, char *line);

void selection(int *l1, int *c1, int *l2, int *c2, matrix_t M, char *line);

#endif
