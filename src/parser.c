#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "str.h"
#include "navvdf.h"
#include "parser.h"


static Entry prefabs;

static int ishat(const Entry *);
static int getentry(const Entry *, const char *, Entry *);


int
parse(char *start)
{
	Pos p;
	Entry e;
	char *lp;
	p.start = start;
	p.p = start;

	/*cache the prefabs dir*/
	if(navto(&p, "/items_game/prefabs") < 0){
		fprintf(stderr, "err: prefabs dir not found\n");
		return -1;
	}
	if(navgetwd(&p, &prefabs) < 0){
		fprintf(stderr, "err: can't get prefabs infos\n");
		return -1;
	}

	if(navto(&p, "/items_game/items") < 0){
		fprintf(stderr, "err: couldn't open \"items_game/items\"\n");
		return -1;
	}

	navgetwd(&p, &e);
	printf("%s\n", e.name);

	for(lp = p.p; navnextentry(&lp, &e) == 0;){
		printf("Entry: %s\n", e.name);
		if(ishat(&e))
			printf("This is a hat.\n");
	}

	return 0;
}

static int
ishat(const Entry *item)
{
	Entry e;

	if(getentry(item, "item_slot", &e) < 0)
		return 0;
	if(strcmp(e.val, "head") == 0 || strcmp(e.val, "misc") == 0)
		return 1;

	return 0;
}

static int
getentry(const Entry *parent, const char *name, Entry *res)
{
	Entry e;
	Pos p;
	char *prefab;
	char buf[NAVBUFSIZE] = {0};
	char *ptr = buf;

	p.start = parent->link;
	p.p = parent->link;

	if(naventry(&p, name, res) >= 0)
		return 0;

	/*if we get here, we need to look at prefabs*/

	if(naventry(&p, "prefab", &e) < 0)
		return -1; /*no prefab entry*/

	strncpy(buf, e.val, NAVBUFSIZE-1);
	p.start = prefabs.link;
	p.p = prefabs.link;

	while((prefab = bstrtok_r(&ptr, " ")) != NULL){
		if(naventry(&p, prefab, &e) < 0){
			fprintf(stderr, "warn: unkown prefab \"%s\"\n", prefab);
			continue;
		}
		if(getentry(&e, name, res) >= 0)
			return 0; /*found in a prefab*/
	}

	/*if we get here, the entry wasn't found anywhere*/
	return -1;
}
