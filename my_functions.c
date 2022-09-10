// Copyright Dinuta Eduard-Stefan 311CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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

void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void copy_pixel(pixel_t *a, pixel_t *b)
{
	a->r = b->r;
	a->g = b->g;
	a->b = b->b;
}

// dynamic allocation of a string
char *alloc_string(int size)
{
	char *str = (char *)malloc(size * sizeof(char));
	if (!str) {
		fprintf(stderr, "malloc failed");
		return NULL;
	}
	return str;
}

// dynamic allocation of a pixel matrix
pixel_t **alloc_matrix(int lin, int col)
{
	pixel_t **a = (pixel_t **)malloc(lin * sizeof(pixel_t *));
	if (!a) {
		fprintf(stderr, "malloc failed");
		return NULL;
	}
	for (int i = 0; i < lin; i++) {
		a[i] = (pixel_t *)malloc(col * sizeof(pixel_t));
		if (!a[i]) {
			for (int j = 0; j < i; j++)
				free(a[j]);
			free(a);
			fprintf(stderr, "malloc failed");
			return NULL;
		}
	}

	return a;
}

void free_matrix(matrix_t M)
{
	for (int i = 0; i < M.lin; i++)
		free(M.a[i]);
	free(M.a);
}

// builds a number with all the digits starting from where
// the cursor is in the file, until finding a non-digit character
int get_number(FILE *f, unsigned char *ch)
{
	int nr = 0;
	while (!isdigit(*ch) && !feof(f))
		fread(ch, sizeof(unsigned char), 1, f);
	while (isdigit(*ch)) {
		nr = nr * 10 + *ch - '0';
		fread(ch, sizeof(unsigned char), 1, f);
	}

	return nr;
}

// ignoring comment lines
void jump_comm(FILE *f, unsigned char *ch)
{
	fread(ch, sizeof(unsigned char), 1, f);
	while (*ch == '#') {
		unsigned char tmp;
		fread(&tmp, sizeof(unsigned char), 1, f);
		while (tmp != '\n')
			fread(&tmp, sizeof(unsigned char), 1, f);
		fread(ch, sizeof(unsigned char), 1, f);
	}
}

// extracting the matrix from an image file, in order to process the image
// and apply filters
void get_matrix(char *filename, matrix_t *M)
{
	// open the image
	FILE *f = fopen(filename, "rb");
	if (!f) {
		printf("Failed to load %s\n", filename);
		return;
	}
	unsigned char ch;

	// reading the type
	jump_comm(f, &ch);
	fread(&ch, sizeof(unsigned char), 1, f);
	M->type = ch - '0';
	fread(&ch, sizeof(unsigned char), 1, f);

	// reading the dimensions and allocating the pixel matrix
	jump_comm(f, &ch);
	M->col = get_number(f, &ch);
	M->lin = get_number(f, &ch);
	M->a = alloc_matrix(M->lin, M->col);
	if (!M->a)
		exit(-1);

	// initialize the matrix
	for (int i = 0; i < M->lin; i++)
		for (int j = 0; j < M->col; j++)
			M->a[i][j] = (pixel_t){0, 0, 0};

	// reading the maximum values
	jump_comm(f, &ch);
	M->max = get_number(f, &ch);

	// reading the matrix depending on the image type
	// for grayscale we only need to read one value, which we will store
	// in the r field
	jump_comm(f, &ch);
	//printf("%c", ch);
	if (M->type == 5 || M->type == 6) {
		ungetc(ch, f);
		unsigned char r, g, b;
		for (int i = 0; i < M->lin; i++)
			for (int j = 0; j < M->col; j++) {
				fread(&r, sizeof(unsigned char), 1, f);
				M->a[i][j].r = (double)r;
				if (M->type == 6) {
					fread(&g, sizeof(unsigned char), 1, f);
					M->a[i][j].g = (double)g;
					fread(&b, sizeof(unsigned char), 1, f);
					M->a[i][j].b = (double)b;
				}
			}
	} else {
		ungetc(ch, f);
		unsigned char r, g, b;
		for (int i = 0; i < M->lin; i++)
			for (int j = 0; j < M->col; j++) {
				fscanf(f, "%hhu", &r);
				M->a[i][j].r = (double)r;
				if (M->type == 3) {
					fscanf(f, "%hhu", &g);
					M->a[i][j].g = (double)g;
					fscanf(f, "%hhu", &b);
					M->a[i][j].b = (double)b;
				}
			}
	}
	printf("Loaded %s\n", filename);
}

// verify if the coordinates for SELECT command are valid
int verify(int x1, int y1, int x2, int y2, matrix_t M)
{
	if (x1 == x2 || y1 == y2)
		return 0;
	if (x1 < 0 || x1 >= M.lin)
		return 0;
	if (x2 <= 0 || x2 > M.lin)
		return 0;
	if (y1 < 0 || y1 >= M.col)
		return 0;
	if (y2 <= 0 || y2 > M.col)
		return 0;
	return 1;
}

// transpose a matrix
void transpose(int *l1, int *c1, int *l2, int *c2, matrix_t *M)
{
	// building a temporary matrix to store the transpose
	pixel_t **tmp = alloc_matrix(*c2 - *c1, *l2 - *l1);
	if (!tmp) {
		free_matrix(*M);
		exit(-1);
	}
	for (int i = *c1; i < *c2; i++)
		for (int j = *l1; j < *l2; j++)
			copy_pixel(&tmp[i - *c1][j - *l1], &M->a[j][i]);

	// if we transpose the whole matrix, the dimensions change and we
	// need to replace it; it not we can just copy the tmp matrix and
	// modify it
	if (*l2 - *l1 == M->lin && *c2 - *c1 == M->col) {
		free_matrix(*M);
		M->a = tmp;
		M->lin = *c2 - *c1;
		M->col = *l2 - *l1;
		*l1 = *c1 = 0;
		*l2 = M->lin;
		*c2 = M->col;
	} else {
		for (int i = *l1; i < *l2; i++)
			for (int j = *c1; j < *c2; j++)
				copy_pixel(&M->a[i][j], &tmp[i - *l1][j - *c1]);

		for (int i = 0; i < *c2 - *c1; i++)
			free(tmp[i]);
		free(tmp);
	}
}

void swap_pixel(pixel_t *a, pixel_t *b)
{
	pixel_t tmp = *a;
	*a = *b;
	*b = tmp;
}

// reversing the rows of a matrix
void reverse_rows(int l1, int c1, int l2, int c2, matrix_t M)
{
	for (int i = l1; i < l2; i++)
		for (int j = c1; j < (c1 + c2) / 2; j++)
			swap_pixel(&M.a[i][j], &M.a[i][c2 - j + c1 - 1]);
}

// rotating a pixel matrix 90 degrees 'number' times
// in the given coordinates
void rotate_90(int *l1, int *c1, int *l2, int *c2, matrix_t *M, int number)
{
	// if the whole image is not selected and the coordinates do not make a
	// square they are not valid
	if (*l2 - *l1 != *c2 - *c1 && *l2 - *l1 != M->lin && *c2 - *c1 != M->col) {
		printf("The selection must be square\n");
		return;
	}

	// base case
	if (number == 0)
		return;

	// if number is positive we will rotate the matrix by 90 degrees
	// if it is negative we rotate it by -90 degrees
	if (number > 0) {
		// to rotate by 90 degrees we first transpose then reverse the rows
		transpose(l1, c1, l2, c2, M);
		reverse_rows(*l1, *c1, *l2, *c2, *M);
		rotate_90(l1, c1, l2, c2, M, number - 1);
	} else  {
		// to rotate by -90 degrees we first reverse the rows then transpose
		reverse_rows(*l1, *c1, *l2, *c2, *M);
		transpose(l1, c1, l2, c2, M);
		rotate_90(l1, c1, l2, c2, M, number + 1);
	}
}

// rotating the matrix by an angle multiple of 90 in interval [-360, 360]
void rotation(int *l1, int *c1, int *l2, int *c2, matrix_t *M, char *line)
{
	char *command = alloc_string(NMAX);
	if (!command) {
		free_matrix(*M);
		exit(-1);
	}

	int angle = 0;
	if (sscanf(line, "%s %d", command, &angle) != 2) {
		printf("Invalid command\n");
	} else if (angle % 90 != 0 || angle > 360 || angle < -360) {
		printf("Unsupported rotation angle\n");
	} else if (!M->a) {
		printf("No image loaded\n");
	} else {
		rotate_90(l1, c1, l2, c2, M, angle / 90);
		printf("Rotated %d\n", angle);
	}
	free(command);
}

// cropping the image to the current selection
void crop(int *l1, int *c1, int *l2, int *c2, matrix_t *M)
{
	// building a temporary matrix containing the current selection
	pixel_t **tmp = alloc_matrix(*l2 - *l1, *c2 - *c1);
	if (!tmp) {
		free_matrix(*M);
		exit(-1);
	}
	for (int i = *l1; i < *l2; i++)
		for (int j = *c1; j < *c2; j++)
			copy_pixel(&tmp[i - *l1][j - *c1], &M->a[i][j]);

	// replacing the old matrix with the new one and changing the
	// dimensions
	free_matrix(*M);
	M->a = tmp;
	M->lin = *l2 - *l1;
	M->col = *c2 - *c1;
	*l1 = 0;
	*c1 = 0;
	*l2 = M->lin;
	*c2 = M->col;
	printf("Image cropped\n");
}

// saving the modified image in the system with the name 'filename'
void save(matrix_t M, char *filename, int ascii)
{
	// ascii specifies the type it should be saved with
	// ascii = 1 is text format, ascii = 0 is binary
	if (ascii == 1) {
		// open the file in write text format
		FILE *f = fopen(filename, "wt");
		if (!f) {
			fprintf(stderr, "failed to load %s\n", filename);
			free_matrix(M);
			exit(-1);
		}

		// printing the type
		if (M.type == 1 || M.type == 4)
			fprintf(f, "P1\n");
		if (M.type == 2 || M.type == 5)
			fprintf(f, "P2\n");
		if (M.type == 3 || M.type == 6)
			fprintf(f, "P3\n");

		// printing the dimensions and the maximum value
		fprintf(f, "%d %d\n", M.col, M.lin);
		if (M.type == 2 || M.type == 5 || M.type == 3 || M.type == 6)
			fprintf(f, "%d\n", M.max);

		// printing the pixel matrix
		int r, g, b;
		for (int i = 0; i < M.lin; i++) {
			for (int j = 0; j < M.col; j++) {
				r = (int)round(M.a[i][j].r);
				fprintf(f, "%d ", r);
				if (M.type == 3 || M.type == 6) {
					g = (int)round(M.a[i][j].g);
					b = (int)round(M.a[i][j].b);
					fprintf(f, "%d ", g);
					fprintf(f, "%d ", b);
				}
			}
			fprintf(f, "\n");
		}
		fclose(f);
	} else {
		// open the file in write binary format
		FILE *f = fopen(filename, "wb");
		if (!f) {
			fprintf(stderr, "failed to load %s\n", filename);
			free_matrix(M);
			exit(-1);
		}

		// printing the type
		if (M.type == 1 || M.type == 4)
			fwrite("P4\n", sizeof(unsigned char), 3, f);
		if (M.type == 2 || M.type == 5)
			fwrite("P5\n", sizeof(unsigned char), 3, f);
		if (M.type == 3 || M.type == 6)
			fwrite("P6\n", sizeof(unsigned char), 3, f);

		// printing the dimensions and the maximum value
		fprintf(f, "%d %d\n", M.col, M.lin);
		if (M.type == 2 || M.type == 5 || M.type == 3 || M.type == 6)
			fprintf(f, "%d\n", M.max);

		// printing the pixel matrix
		unsigned char r, g, b;
		for (int i = 0; i < M.lin; i++) {
			for (int j = 0; j < M.col; j++) {
				r = (unsigned char)round(M.a[i][j].r);
				fwrite(&r, sizeof(unsigned char), 1, f);
				if (M.type == 3 || M.type == 6) {
					g = (unsigned char)round(M.a[i][j].g);
					b = (unsigned char)round(M.a[i][j].b);
					fwrite(&g, sizeof(unsigned char), 1, f);
					fwrite(&b, sizeof(unsigned char), 1, f);
				}
			}
		}
		fclose(f);
	}
	printf("Saved %s\n", filename);
}

// verify if a cell with coordinates x and y is inside a matrix
// with dimensions lin and col
int is_in_matrix(int x, int y, int lin, int col)
{
	if (x < 0 || x >= lin)
		return 0;
	if (y < 0 || y >= col)
		return 0;
	return 1;
}

// clamp function to keep values in a [min, max] interval
double clamp(double x, double min, double max)
{
	if (x < min)
		x = min;
	if (x > max)
		x = max;
	return x;
}

// makes the operations necessary for applying a filter
void apply(int l1, int c1, int l2, int c2, matrix_t M, double ker[3][3], int x)
{
	if (!M.a) {
		printf("No image loaded\n");
		exit(-1);
	}

	// direction arrays for accessing the adjacent cells
	int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0, 0};
	int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1, 0};
	pixel_t **tmp = alloc_matrix(l2 - l1, c2 - c1);
	if (!tmp) {
		free_matrix(M);
		exit(-1);
	}

	// applying the filter found in the ker[][] parameter in the
	// temporary matrix
	for (int i = l1; i < l2; i++)
		for (int j = c1; j < c2; j++) {
			int ok = 1;
			double sum_r = 0, sum_g = 0, sum_b = 0;

			// iterating through the adjacent cells and the kernel matrix
			for (int k = 0; k < 9 && ok == 1; k++) {
				int new_i = i + dx[k];
				int new_j = j + dy[k];
				int ker_i = dx[k] + 1;
				int ker_j = dy[k] + 1;
				if (!is_in_matrix(new_i, new_j, M.lin, M.col)) {
					ok = 0;
				} else {
					sum_r += (ker[ker_i][ker_j] * (double)M.a[new_i][new_j].r);
					sum_g += (ker[ker_i][ker_j] * (double)M.a[new_i][new_j].g);
					sum_b += (ker[ker_i][ker_j] * (double)M.a[new_i][new_j].b);
				}
			}
			sum_r /= (double)x;
			sum_g /= (double)x;
			sum_b /= (double)x;
			// keeping the values in the [0, M.max] interval
			sum_r = clamp(sum_r, 0, M.max);
			sum_g = clamp(sum_g, 0, M.max);
			sum_b = clamp(sum_b, 0, M.max);
			if (ok) {
				tmp[i - l1][j - c1].r = sum_r;
				tmp[i - l1][j - c1].g = sum_g;
				tmp[i - l1][j - c1].b = sum_b;
			} else {
				copy_pixel(&tmp[i - l1][j - c1], &M.a[i][j]);
			}
		}

	// copy the new modified matrix in the old one
	for (int i = l1; i < l2; i++)
		for (int j = c1; j < c2; j++)
			copy_pixel(&M.a[i][j], &tmp[i - l1][j - c1]);

	for (int i = 0; i < l2 - l1; i++)
		free(tmp[i]);
	free(tmp);
}

// APPLY command
void apply_filter(int l1, int c1, int l2, int c2, matrix_t M, char *line)
{
	// filter kernels
	double EDGE[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
	double SHARPEN[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
	double BLUR[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
	double GAUSSIAN_BLUR[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

	char *filter = alloc_string(NMAX);
	if (!filter) {
		free_matrix(M);
		exit(-1);
	}
	char *command = alloc_string(NMAX);
	if (!command) {
		free(filter);
		free_matrix(M);
		exit(-1);
	}

	// applying the filters and printing the corespongind messages
	// or error messages
	if (!M.a) {
		printf("No image loaded\n");
	} else {
		int arg = sscanf(line, "%s %s", command, filter);
		if (arg != 2) {
			printf("Invalid command\n");
		} else {
			if (M.type == 2 || M.type == 5) {
				printf("Easy, Charlie Chaplin\n");
			} else {
				int ok = 1;
				if (strcmp(filter, "EDGE") == 0) {
					apply(l1, c1, l2, c2, M, EDGE, 1);
				} else if (strcmp(filter, "SHARPEN") == 0) {
					apply(l1, c1, l2, c2, M, SHARPEN, 1);
				} else if (strcmp(filter, "BLUR") == 0) {
					apply(l1, c1, l2, c2, M, BLUR, 9);
				} else if (strcmp(filter, "GAUSSIAN_BLUR") == 0) {
					apply(l1, c1, l2, c2, M, GAUSSIAN_BLUR, 16);
				} else {
					printf("APPLY parameter invalid\n");
					ok = 0;
				}
				if (M.a && ok)
					printf("APPLY %s done\n", filter);
			}
		}
	}
	free(filter);
	free(command);
}

// SELECT command for both ALL and for specific indexes
void selection(int *l1, int *c1, int *l2, int *c2, matrix_t M, char *line)
{
	int x1, y1, x2, y2;
	char *command = alloc_string(30);
	if (!command) {
		free_matrix(M);
		exit(-1);
	}

	int arg = sscanf(line, "%s%d%d%d%d", command, &y1, &x1, &y2, &x2);
	// if 4 coordinates are given we try to select the coresponding matrix
	// else we select the whole matrix
	if (arg == 1)  {
		if (!M.a) {
			printf("No image loaded\n");
		} else {
			*l1 = 0;
			*c1 = 0;
			*l2 = M.lin;
			*c2 = M.col;
			printf("Selected ALL\n");
		}
	} else if (arg == 5) {
		if (!M.a) {
			printf("No image loaded\n");
		} else {
			//printf("%d %d %d %d\n", l1, c1, l2, c2);
			// mmaking sure x1 < x2 and y1 < y2 is valid
			if (x1 > x2)
				swap(&x1, &x2);
			if (y1 > y2)
				swap(&y1, &y2);
			if (!verify(x1, y1, x2, y2, M)) {
				printf("Invalid set of coordinates\n");
			} else {
				*l1 = x1;
				*c1 = y1;
				*l2 = x2;
				*c2 = y2;
				printf("Selected %d %d %d %d\n", y1, x1, y2, x2);
			}
		}
	} else {
		printf("Invalid command\n");
	}
	free(command);
}
