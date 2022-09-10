// Copyright Dinuta Eduard-Stefan 311CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "my_functions.h"

#define NMAX 100

int main(void)
{
	char *filename = alloc_string(NMAX);
	if (!filename)
		return -1;
	char *line = alloc_string(NMAX);
	if (!line)
		return -1;
	char *command = alloc_string(NMAX);
	if (!command)
		return -1;
	int l1 = 0, c1 = 0, l2 = 0, c2 = 0;
	matrix_t M;
	M.a = NULL;

	// reading the commands from the user
	// and displaying coresponding messages
	fgets(line, NMAX, stdin);
	if (sscanf(line, "%s", command) == 0)
		return -1;
	while (strcmp(command, "EXIT") != 0) {
		if (strcmp(command, "LOAD") == 0) {
			if (sscanf(line, "%s %s", command, filename) != 2)
				return -1;
			if (M.a) {
				free_matrix(M);
				M.a = NULL;
			}
			get_matrix(filename, &M);
			l1 = 0;
			c1 = 0;
			l2 = M.lin;
			c2 = M.col;
		} else if (strcmp(command, "SELECT") == 0) {
			selection(&l1, &c1, &l2, &c2, M, line);
		} else if (strcmp(command, "ROTATE") == 0) {
			rotation(&l1, &c1, &l2, &c2, &M, line);
		} else if (strcmp(command, "CROP") == 0) {
			if (!M.a)
				printf("No image loaded\n");
			else
				crop(&l1, &c1, &l2, &c2, &M);
		} else if (strcmp(command, "SAVE") == 0) {
			char *ascii = alloc_string(NMAX);
			int arg = sscanf(line, "%s %s %s", command, filename, ascii);
			if (!M.a) {
				printf("No image loaded\n");
			} else {
				if (arg == 3)
					save(M, filename, 1);
				else
					save(M, filename, 0);
			}
			free(ascii);
		} else if (strcmp(command, "APPLY") == 0) {
			apply_filter(l1, c1, l2, c2, M, line);
		} else {
			printf("Invalid command\n");
		}
		//print(l1, c1, l2, c2, M);
		fgets(line, NMAX, stdin);
		if (sscanf(line, "%s", command) == 0)
			return -1;

		// closing the program and freeing the resoursces
		if (strcmp(command, "EXIT") == 0) {
			if (M.a)
				free_matrix(M);
			else
				printf("No image loaded");
		}
	}
	free(filename);
	free(command);
	free(line);

	return 0;
}
