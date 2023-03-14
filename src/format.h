/*
 * requires: navvdf.h
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

int formatclasses(const Entry *, Classdata *);
int formatpaths(const Entry *, const Classdata *);
