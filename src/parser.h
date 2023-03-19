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
int getentry(const Entry *, const char *, Entry *);
int getprefab(const char *, Entry *);
int getclasses(const Entry *, Classdata *);
