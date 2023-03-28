#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "str.h"
#include "navvdf.h"
#include "parser.h"
#include "format.h"

#define B_HAT            0x1
#define B_HEADPHONES     0x2
#define B_DOGTAGS        0x4
#define B_BACKPACK       0x8
#define B_GRENADES       0x10
#define B_HEAD           0x20
#define B_SHOES          0x40
#define B_HANDS          0x80
#define B_MEDIC_BACKPACK 0x100
#define ERRMASK 0x0

static const Classinfo classinfo[] = {
	{0x1,     "Scout",    "scout",    "scout",    SCOUT,    B_HAT | B_HEADPHONES | B_DOGTAGS | B_SHOES},
	{0x2,     "Soldier",  "soldier",  "soldier",  SOLDIER,  B_HAT | B_GRENADES},
	{0x4,     "Pyro",     "pyro",     "pyro",     PYRO,     B_BACKPACK | B_HEAD | B_GRENADES},
	{0x8,     "Demo",     "demo",     "demoman",  DEMO,     B_SHOES | B_GRENADES},
	{0x8,     "Demo",     "demo",     "Demoman",  DEMO,     B_SHOES | B_GRENADES}, /*see "Dr's Dapper Topper"*/
	{0x10,    "Engineer", "engineer", "engineer", ENGINEER, B_HAT},
	{0x20,    "Heavy",    "heavy",    "heavy",    HEAVY,    B_HANDS},
	{0x40,    "Medic",    "medic",    "medic",    MEDIC,    B_MEDIC_BACKPACK},
	{0x80,    "Sniper",   "sniper",   "sniper",   SNIPER,   B_HAT},
	{0x100,   "Spy",      "spy",      "spy",      SPY,      0},

	{ERRMASK, "Error",    "Error",    "Error",    SCOUT,    0}
};

static const Bodyinfo bodyinfo[] = {
	{B_HAT,            "hat",        "hat"},
	{B_HEADPHONES,     "headphones", "headphones"},
	{B_DOGTAGS,        "dogtags",    "dogtags"},
	{B_BACKPACK,       "backpack",   "backpack"},
	{B_MEDIC_BACKPACK, "backpack",   "medic_backpack"},
	{B_GRENADES,       "grenades",   "grenades"},
	{B_HEAD,           "head",       "head"},
	{B_SHOES,          "shoes",      "shoes"},
	{B_SHOES,          "shoes",      "shoes_socks"},
	{B_HANDS,          "hands",      "hands"},

	{ERRMASK,      "Error",      "Error"}
};

/*there must be at least 9 rows
 *help: I am a wanted criminal on the run
 */
unsigned int
formatpaths(const Entry *e, unsigned int classb, char paths[][NAVBUFSIZE])
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
			break;
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
			break;
	return &classinfo[i];
}

/*classb should only contain one class*/
const Classinfo *
getclass_b(unsigned int classb)
{
	int i;
	for(i = 0; classinfo[i].mask != ERRMASK; i++)
		if(classb & classinfo[i].mask)
			break;
	return &classinfo[i];
}

const Bodyinfo *
getbody_n(const char *name)
{
	int i;
	for(i = 0; bodyinfo[i].mask != ERRMASK; i++)
		if(strcmp(name, bodyinfo[i].name) == 0||
			strcmp(name, bodyinfo[i].sname) == 0)
			break;
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
			break;
	return &bodyinfo[i];
}
