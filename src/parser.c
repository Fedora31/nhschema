#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "str.h"
#include "navvdf.h"
#include "updater.h"
#include "parser.h"
#include "format.h"

/*the prefab entry is here to speed things up*/
static Entry prefabs;

static int ishat(const Entry *);


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

	for(lp = p.p; navnextentry(&lp, &e) == 0;){
		if(ishat(&e)){
			if(updater_print(&e) < 0){
				getentry(&e, "name", &e);
				fprintf(stderr, "err: couldn't parse entry of hat \"%s\"\n", e.val);
				continue;
			}
		}
	}

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
	Entry e;
	char *lp;
	int i = 0;
	unsigned int mask = 0;

	/*The bits will also be all set if the entry is empty.
	 */
	if(getentry(hat, "used_by_classes", &e) == 0)
		for(lp = e.link; navnextentry(&lp, &e) == 0; i++)
			mask |= getclass_n(e.name)->mask;

	return i != 0 ? mask : ~(mask & 0);
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

/*Get the entry with the given name. This function will look at the
 *given entry for any match and will recurse into any specified prefabs
 *if it's not found. Prefabs can also contain prefabs.
 *Warning: This does not check for death loops.
 */
int
getentry(const Entry *parent, const char *name, Entry *res)
{
	Entry e;
	Entry p = *parent;
	char *prefab;
	char buf[NAVBUFSIZE] = {0};
	char *ptr = buf;

	if(naventry(&p, name, res) >= 0)
		return 0;

	/*if we get here, we need to look at prefabs*/

	if(naventry(&p, "prefab", &e) < 0)
		return -1; /*no prefab entry*/

	strncpy(buf, e.val, NAVBUFSIZE-1);
	p = prefabs;

	while((prefab = bstrtok_r(&ptr, " ")) != NULL){
		if(naventry(&p, prefab, &e) < 0){
			fprintf(stderr, "warn: unknown prefab \"%s\"\n", prefab);
			continue;
		}
		if(getentry(&e, name, res) >= 0)
			return 0; /*found in a prefab*/
	}

	/*if we get here, the entry wasn't found anywhere*/
	return -1;
}

int
getprefab(const char *name, Entry *res)
{
	return getentry(&prefabs, name, res);
}
