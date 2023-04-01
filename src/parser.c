#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "arg.h"
#include "str.h"
#include "navvdf.h"
#include "updater.h"
#include "custom.h"
#include "parser.h"
#include "format.h"

static int ishat(const Pos *);


int
parse(char *start)
{
	Tree *t;
	int i;
	Pos pos;

	if((t = navgentree(start, 2048)) == NULL){
		fprintf(stderr, "fatal: couldn't create tree\n");
		return -1;
	}

	pos_init(&pos, t);
	navto2(&pos, "/items_game/items");

	if(arg_getcmode()){
		custom_printheader();
		for(i = 0; navtoi(&pos, i++) >= 0; navto2(&pos, ".."))
			if(ishat(&pos))
				if(custom_print(&pos) < 0)
					fprintf(stderr, "err: couldn't parse hat \"%s\"\n", pos.p[pos.i]->name);
		return 0;
	}

	for(i = 0; navtoi(&pos, i++) >= 0; navto2(&pos, ".."))
		if(ishat(&pos))
			if(updater_print(&pos) < 0)
				fprintf(stderr, "err: couldn't parse hat \"%s\"\n", pos.p[pos.i]->name);

	return 0;
}

/*Return a mask with the found classes.
 *Warning: if no class is found, the
 *bits will be all set, as per the wiki
 *documentation.
 */
unsigned int
getclasses(const Entry *hat)
{
	Entry *e;
	int i;
	unsigned int mask = 0;

	/*The bits will also be all set if the entry is empty.*/

	if(navopen2(hat, "used_by_classes", &e) == 0)
		for(i = 0; i < e->childc; i++)
			mask |= getclass_n(e->childs[i]->name)->mask;

	return i != 0 ? mask : ~(mask & 0);
}

unsigned long long int
getequips(const Entry *hat)
{
	Entry *e;
	int i;
	unsigned long long int mask = 0;

	if(navopen2(hat, "equip_region", &e) == 0)
		mask |= getequip_n(e->val)->mask;

	if(navopen2(hat, "equip_regions", &e) == 0)
		for(i = 0; i < e->childc; i++)
			mask |= getequip_n(e->childs[i]->name)->mask;

	return mask;
}

static int
ishat(const Pos *p)
{
	Pos lp = *p;
	if(navto2(&lp, "item_slot")<0)
		return 0;
	if(strcmp(lp.p[lp.i]->val, "head") == 0 || strcmp(lp.p[lp.i]->val, "misc") == 0)
		return 1;
	return 0;
}
