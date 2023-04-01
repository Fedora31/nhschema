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

/*the mask cannot be more than 64 bits!*/
static const Equipinfo equipinfo[] = {

	{0x1, "Hat",         "hat"},
	{0x2, "Hair",        "heavy_hair"},
	{0x2, "Hair",        "engineer_hair"},
	{0x4, "Whole head",  "whole_head"},
	{0x8, "Head Replacement", "pyro_head_replacement"},
	{0x8, "Head Replacement", "demo_head_replacement"},
	{0x10, "Head Skin",  "head_skin"},
	{0x20, "Headband",   "sniper_headband"},
	{0x40, "Ears",       "ears"},
	{0x80, "Face",       "face"},
	{0x100, "Moustache and Eyepatch", "demo_eyepatch"},
	{0x200, "Glasses",    "glasses"},
	{0x400, "Lenses",     "lenses"},
	{0x800, "Pipe",       "medic_pipe"},
	{0x1000, "Cigar",      "soldier_cigar"},
	{0x2000, "Beard",      "beard"},
	{0x4000, "Necklace",   "necklace"},
	{0x8000, "Collar",     "demoman_collar"},
	{0x10000, "Spikes",     "pyro_spikes"},
	{0x20000, "Towel",      "heavy_towel"},
	{0x40000, "Left Shoulder", "left_shoulder"},
	{0x80000, "Right Shoulder", "right_shoulder"},
	{0x100000, "Wings",      "pyro_wings"},
	{0x100000, "Wings",      "engineer_wings"},
	{0x100000, "Wings",      "scout_wings"},
	{0x200000, "Back",       "back"},
	{0x400000, "Backpack",   "scout_backpack"},
	{0x400000, "Backpack",   "medigun_backpack"},
	{0x800000, "Quiver",     "sniper_quiver"},
	{0x1000000, "Arms",       "arms"},
	{0x2000000, "Arm Tattoos", "arm_tattoos"},
	{0x4000000, "Left Arm",   "engineer_left_arm"},
	{0x8000000, "Bandages",   "scout_bandages"},
	{0x10000000, "Hands",      "scout_hands"},
	{0x20000000, "Gloves",     "soldier_gloves"},
	{0x20000000, "Gloves",     "medic_gloves"},
	{0x20000000, "Gloves",     "spy_gloves"},
	{0x40000000, "Sleeves",    "sleeves"},
	{0x80000000, "Shirt",      "shirt"},
	{0x100000000, "Medal",      "medal"},
	{0x200000000, "Grenades",   "grenades"},
	{0x400000000, "Bullets",    "heavy_bullets"},
	{0x400000000, "Bullets",    "sniper_bullets"},
	{0x800000000, "Flair",      "flair"},
	{0x1000000000, "Vest",       "sniper_vest"},
	{0x2000000000, "Coat",       "soldier_coat"},
	{0x2000000000, "Coat",       "spy_coat"},
	{0x4000000000, "Hip Pouch",  "heavy_hip_pouch"},
	{0x8000000000, "Hip",        "heavy_hip"},
	{0x8000000000, "Hip",        "medic_hip"},
	{0x10000000000, "Medigun Accessories", "medigun_accessories"},
	{0x20000000000, "Tail",       "pyro_tail"},
	{0x40000000000, "Belt",       "belt_misc"},
	{0x40000000000, "Belt",       "heavy_belt_back"},
	{0x40000000000, "Belt",       "demo_belt"},
	{0x40000000000, "Belt",       "engineer_belt"},
	{0x80000000000, "Legs",       "sniper_legs"},
	{0x80000000000, "Legs",       "soldier_legs"},
	{0x100000000000, "Pants",      "pants"},
	{0x100000000000, "Pants",      "scout_pants"},
	{0x200000000000, "Pocket",     "engineer_pocket"},
	{0x200000000000, "Pocket",     "heavy_pocket"},
	{0x200000000000, "Pocket",     "soldier_pocket"},
	{0x200000000000, "Pocket",     "sniper_pocket"},
	{0x200000000000, "Pocket",     "sniper_pocket_left"},
	{0x400000000000, "Feet",       "feet"},
	{0x800000000000, "Disconnected Floating Item", "disconnected_floating_item"},
	{0x1000000000000, "Zombie Body", "zombie_body"},

	{ERRMASK, "Error", "Error"}
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

const Equipinfo *
getequip_n(const char *name)
{
	int i;
	for(i = 0; equipinfo[i].mask != ERRMASK; i++)
		if(strcmp(name, equipinfo[i].name) == 0||
			strcmp(name, equipinfo[i].sname) == 0)
			break;
	return &equipinfo[i];
}

const Equipinfo *
getequip_b(unsigned long long int qmask)
{
	int i;
	for(i = 0; equipinfo[i].mask != ERRMASK; i++)
		if(qmask & equipinfo[i].mask)
			break;
	return &equipinfo[i];
}

int
getequipcount(void)
{
	int i;
	for(i = 0; equipinfo[i].mask != ERRMASK; i++);
	return i;
}

int
setbitc(unsigned int b)
{
	/*not fast, but idc*/
	int res = 0;
	for(; b; b >>= 1)
		if(b & 1)
			res++;
	return res;
}
