/*
 *requires: navvdf.h
 */

enum CLASSES{
	SCOUT,
	SOLDIER,
	PYRO,
	DEMO,
	ENGINEER,
	HEAVY,
	MEDIC,
	SNIPER,
	SPY,

	/*nothing below this*/
	CLASSCOUNT
};

typedef struct Classdata{
	int id[CLASSCOUNT];
}Classdata;

int parse(char *);
unsigned int getclasses(const Entry2 *);
