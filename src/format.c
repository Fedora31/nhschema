#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <stack.h>
#include <strstack.h>
#include "str.h"
#include "navvdf.h"
#include "parser.h"
#include "format.h"

typedef struct Classstr{
	char *name; /*default name, should be printed to stdout*/
	char *sname; /*as it appears in the item schema*/
	char *fname; /*as it appears in filenames*/
}Classstr;

static Classstr classstr[] = {
	{"Scout",    "scout",    "scout"   },
	{"Soldier",  "soldier",  "soldier" },
	{"Pyro",     "pyro",     "pyro"    },
	{"Demo",     "demoman",  "demo"    },
	{"Engineer", "engineer", "engineer"},
	{"Heavy",    "heavy",    "heavy"   },
	{"Medic",    "medic",    "medic"   },
	{"Sniper",   "sniper",   "sniper"  },
	{"Spy",      "spy",      "spy"     },
};

static int getclassid(const char *);
static int getpaths(const Entry *, Stack *);


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
		if((class = getclassid(e.name)) == -1)
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
			printf(classstr[i].name);
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
getclassid(const char *name)
{
	int i;
	for(i = 0; i < CLASSCOUNT; i++){
		if(strcmp(classstr[i].name, name) == 0 ||
			strcmp(classstr[i].sname, name) == 0 ||
			strcmp(classstr[i].fname, name) == 0
		)
			return i;
	}

	/*Ugly check needed because Dr's Dapper Topper has
	 *a wrong schema name ("Demoman" instead of "demoman")
	 */
	if(strcmp(name, "Demoman") == 0)
		return DEMO;
	return -1;
}
