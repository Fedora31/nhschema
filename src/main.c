#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "navvdf.h"
#include "navvdf2.h"
#include "parser.h"


static int loadf(FILE *, char **);


int
main(void)
{
	char *in;
	if(loadf(stdin, &in) < 0){
		fprintf(stderr, "err: could not read standard input\n");
		return 1;
	}
	if(parse(in) < 0){
		fprintf(stderr, "err: parser failed\n");
		return 1;
	}

	free(in);
	return 0;
}

int
loadf(FILE *f, char **s)
{
	int size, check;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);

	*s = malloc(size+1);
	if((check = fread(*s, 1, size, f))<size){
		fprintf(stderr, "loadf(): err: fread() read less bytes than expected: should be %d, got %d\n", size, check);
		return -1;
	}
	(*s)[size] = '\0';

	return 0;
}
