#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <stack.h>
#include <strstack.h>
#include "str.h"
#include "navvdf.h"
#include "parser.h"
#include "format.h"

#define B_HAT        0x1
#define B_HEADPHONES 0x2
#define B_DOGTAGS    0x4
#define B_BACKPACK   0x8
#define B_GRENADES   0x10
#define B_HEAD       0x20
#define B_SHOES      0x40
#define B_HANDS      0x80
#define ERRMASK 0x0

static const Classinfo classinfo[] = {
	{0x1,     "Scout",    "scout",    "scout",    SCOUT,    B_HAT | B_HEADPHONES | B_DOGTAGS | B_SHOES},
	{0x2,     "Soldier",  "soldier",  "soldier",  SOLDIER,  B_HAT | B_GRENADES},
	{0x4,     "Pyro",     "pyro",     "pyro",     PYRO,     B_BACKPACK | B_HEAD | B_GRENADES},
	{0x8,     "Demo",     "demo",     "demoman",  DEMO,     B_SHOES},
	{0x8,     "Demo",     "demo",     "Demoman",  DEMO,     B_SHOES}, /*Dr's Dapper Topper*/
	{0x10,    "Engineer", "engineer", "engineer", ENGINEER, B_HAT},
	{0x20,    "Heavy",    "heavy",    "heavy",    HEAVY,    B_HANDS},
	{0x40,    "Medic",    "medic",    "medic",    MEDIC,    B_BACKPACK},
	{0x80,    "Sniper",   "sniper",   "sniper",   SNIPER,   B_HAT},
	{0x100,   "Spy",      "spy",      "spy",      SPY,      0},

	{ERRMASK, "Error",    "Error",    "Error",    SCOUT,    0}
};

static const Bodyinfo bodyinfo[] = {
	{B_HAT,        "hat",        "hat"},
	{B_HEADPHONES, "headphones", "headphones"},
	{B_DOGTAGS,    "dogtags",    "dogtags"},
	{B_BACKPACK,   "backpack",   "backpack"},
	{B_BACKPACK,   "backpack",   "medic_backpack"},
	{B_GRENADES,   "grenades",   "grenades"},
	{B_HEAD,       "head",       "head"},
	{B_SHOES,      "shoes",      "shoes"},
	{B_SHOES,      "shoes",      "shoes_socks"},
	{B_HANDS,      "hands",      "hands"},

	{ERRMASK,      "Error",      "Error"}
};

static int getpaths(const Entry *, Stack *);
static int duppaths(Stack *, const Classdata *);


int
formatclasses(const Entry *hat, Classdata *cdata)
{
	Entry e;
	char *lp;
	int class;
	int i, j = 0;

	memset(cdata->id, 0, sizeof(int) * 9);

	/*from the wiki: "If this field is not present all classes can
	 *use the item."
	 */
	if(getentry(hat, "used_by_classes", &e) < 0){
		memset(cdata->id, 1, sizeof(int) * 9);
		printf("All classes");
		goto end;
	}

	lp = e.link;
	/*I decided to tread empty entries the same way.*/
	if(navnextentry(&lp, &e) < 0){
		memset(cdata->id, 1, sizeof(int) * 9);
		printf("All classes");
		goto end;
	}

	/*get the classes ids, and then write them or
	 *"All classes" if they are all used
	 */

	do{
		if((class = getclass_n(e.name)->id) == -1)
			return -1;
		cdata->id[class] = 1;
	}while(navnextentry(&lp, &e) == 0);

	for(i = 0; cdata->id[i] == 1 && i < 9; i++);
	if(i == 9){
		printf("All classes");
		goto end;
	}
	for(i = 0; i < 9; i++){
		if(cdata->id[i] == 1){
			if(j++ > 0)
				printf("|");
			printf("%s", getclass_i(i)->fname);
		}
	}

end:
	printf(";");
	return 0;
}

int
formatpaths(const Entry *hat, const Classdata *cdata)
{
	Entry e;
	char *lp;
	char *test;
	int i = 0;
	Stack paths;
	int res = -1;

	stack_init(&paths, 1, 24, sizeof(char) * 512);

	if(getpaths(hat, &paths) >= 0)
		res = 0;
	if(getentry(hat, "visuals", &e) == 0 && getentry(&e, "styles", &e) == 0){
		lp = e.link;
		while(navnextentry(&lp, &e) == 0)
			if(getpaths(&e, &paths) >= 0)
				res = 0;
	}

	/*if res is still -1 here, then there is truly no
	 *paths in this item
	 */
	if(res == -1)
		return res;

	duppaths(&paths, cdata);

	for(; (test = stack_getnextused(&paths, &i)) != NULL;){
		printf("PATH: %s\n", test);
	}

	stack_free(&paths);
	return 0;
}

static int
getpaths(const Entry *p, Stack *s)
{
	Entry e;
	char *lp;
	int res = -1;

	if(getentry(p, "model_player", &e) == 0){
		res = 0;
		if(strstack_contain(s, e.val) == -1)
			stack_add(s, e.val);
	}
	if(getentry(p, "model_player_per_class", &e) == 0){
		lp = e.link;
		while(navnextentry(&lp, &e) == 0){
			res = 0;
			if(strstack_contain(s, e.val) == -1)
				stack_add(s, e.val);
		}
	}
	return res;
}

static int
duppaths(Stack *paths, const Classdata *cdata)
{
	int i, j;
	char *path;
	char tmp[NAVBUFSIZE] = {0};
	Stack p;
	stack_init(&p, 10, 10, NAVBUFSIZE);

	for(i = 0; (path = stack_getnextused(paths, &i)) != NULL; ){
		printf("FOUND: %s\n", path);
		if(strstr(path, "%s") == NULL)
			continue;

		for(j = 0; j < CLASSCOUNT; j++){
			if(cdata->id[j] == 0)
				continue;

			strncpy(tmp, path, NAVBUFSIZE-1);
			printf("TMP: %s\n", tmp);
			strswapall(tmp, "%s", getclass_i(j)->fname, NAVBUFSIZE-1);
			stack_add(&p, tmp);
		}
		stack_rem(paths, i-1);
	}
	for(i = 0; (path = stack_getnextused(&p, &i)) != NULL; ){
		printf("FOUND: %s\n", path);
	}

	stack_free(&p);
	return 0;
}

unsigned int
formatpaths2
(const Entry *e, unsigned int classb, char paths[CLASSCOUNT][NAVBUFSIZE])
{
	int i;
	unsigned int res = 0;

	if(strcmp(e->name, "basename")==0){
		for(i = 0; i < CLASSCOUNT; i++){
			if(classb >> i & 1 && strlen(e->val) > 0){
				strncpy(paths[i], e->val, NAVBUFSIZE-1);
				strswapall(paths[i], "%s", getclass_i(i)->fname, NAVBUFSIZE-1);
				res |= getclass_i(i)->mask;
			}
		}
		return res;
	}
	res = getclass_n(e->name)->id;
	strncpy(paths[res], e->val, NAVBUFSIZE-1);
	res = getclass_i(res)->mask;

	return res;
}

const Classinfo *
getclass_i(unsigned int id)
{
	int i;
	for(i = 0; classinfo[i].mask != ERRMASK; i++)
		if(classinfo[i].id == id)
			return &classinfo[i];
	return &classinfo[i];
}

const Classinfo *
getclass_n(const char *name)
{
	int i;
	for(i = 0; classinfo[i].mask != ERRMASK; i++)
		if(strcmp(name, classinfo[i].name) == 0||
			strcmp(name, classinfo[i].sname) == 0||
			strcmp(name, classinfo[i].fname) == 0)
			return &classinfo[i];
	return &classinfo[i];
}

/*classb should only contain one class*/
const Classinfo *
getclass_b(unsigned int classb)
{
	int i;
	for(i = 0; classinfo[i].mask != ERRMASK; i++)
		if(classb & classinfo[i].mask)
			return &classinfo[i];
	return &classinfo[i];
}


void
printclassb(unsigned int classb)
{
	int i;
	for(i = 0; classinfo[i].mask != ERRMASK; i++)
		if(classb & classinfo[i].mask)
			printf("%s ",classinfo[i].fname);
}

const Bodyinfo *
getbody_n(const char *name)
{
	int i;
	for(i = 0; bodyinfo[i].mask != ERRMASK; i++)
		if(strcmp(name, bodyinfo[i].name) == 0||
			strcmp(name, bodyinfo[i].sname) == 0)
			return &bodyinfo[i];
	return &bodyinfo[i];
}

void
printbody_b(unsigned int bodyb)
{
	int i;
	for(i = 0; bodyinfo[i].mask != ERRMASK; i++)
		if(bodyb & bodyinfo[i].mask)
			printf("%s ", bodyinfo[i].name);
}

const Bodyinfo *
getbody_b(unsigned int bodyb)
{
	int i;
	for(i = 0; bodyinfo[i].mask != ERRMASK; i++)
		if(bodyb & bodyinfo[i].mask)
			return &bodyinfo[i];
	return &bodyinfo[i];
}
