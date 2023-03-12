#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "navvdf.h"
#include "parser.h"

int
parse(char *start)
{
	Pos p;
	Entry e;
	char *lp;
	p.start = start;
	p.p = start;

	if(navto(&p, "/items_game/qualities") < 0){
		fprintf(stderr, "err: couldn't open \"items_game/items\"\n");
		return -1;
	}

	navgetwd(&p, &e);
	printf("%s\n", e.name);

	for(lp = p.p; navnextentry(&lp, &e) == 0;)
		printf("Entry: %s\n", e.name);

	return 0;
}
