#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "str.h"
#include "navvdf.h"
#include "format.h"
#include "parser.h"

/*the prefab entry is here to speed things up*/
static Entry prefabs;
static char *header = "hat;class;equip;date;update;path";
static char sep = ';';
static char insep = '|';

static int ishat(const Entry *);
static int printhat(const Entry *);


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

	printf("%s\n", header);
	for(lp = p.p; navnextentry(&lp, &e) == 0;){
		if(ishat(&e))
			if(printhat(&e) < 0){
				getentry(&e, "name", &e);
				fprintf(stderr, "err: couldn't parse entry of hat \"%s\"\n", e.val);
				return -1;
			}
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
printhat(const Entry *hat)
{
	Entry e;
	char *lp;
	Classdata cdata;

	/*name*/

	if(getentry(hat, "name", &e) < 0)
		return -1;
	printf("%s%c", e.val, sep);

	/*classes*/

	if(formatclasses(&e, &cdata) < 0)
		return -1;

	/*equip regions*/

	/*TODO: I think there can be multiple equip regions?*/
	if(getentry(hat, "equip_region", &e) < 0)
		printf("None%c", sep);
	else
		printf("%s%c", e.val, sep);

	/*date*/

	if(getentry(hat, "first_sale_date", &e) < 0)
		printf("None%c", sep);
	else
		printf("%s%c", e.val, sep);

	/*update*/

	printf("None%c", sep);

	/*path*/

	if(formatpaths(hat) < 0)
		return -1;


	printf("\n");

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
			fprintf(stderr, "warn: unknown prefab \"%s\"\n", prefab);
			continue;
		}
		if(getentry(&e, name, res) >= 0)
			return 0; /*found in a prefab*/
	}

	/*if we get here, the entry wasn't found anywhere*/
	return -1;
}
