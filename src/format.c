#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "navvdf.h"
#include "parser.h"
#include "format.h"

typedef struct Classstr{
	int id;
	char *name; /*default name, should be printed to stdout*/
	char *sname; /*as it appears in the item schema*/
	char *fname; /*as it appears in filenames*/
}Classstr;

static Classstr classstr[] = {
	{0, "Scout",    "scout",    "scout"   },
	{1, "Soldier",  "soldier",  "soldier" },
	{2, "Pyro",     "pyro",     "pyro"    },
	{3, "Demo",     "demoman",  "demo"    },
	{4, "Engineer", "engineer", "engineer"},
	{5, "Heavy",    "heavy",    "heavy"   },
	{6, "Medic",    "medic",    "medic"   },
	{7, "Sniper",   "sniper",   "sniper"  },
	{8, "Spy",      "spy",      "spy"     },

	{0, NULL, NULL, NULL }
};

static int getclassid(const char *);


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
formatpaths(const Entry *hat)
{
	Entry e;
	if(getentry(hat, "model_player", &e) == 0)
		printf("%s", e.val);
	else if(getentry(hat, "model_player_per_class", &e) == 0){

	}
	return 0;
}

static int
getclassid(const char *name)
{
	int i;
	for(i = 0; classstr[i].name != NULL; i++){
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
