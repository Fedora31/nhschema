/*
 * requires: navvdf.h, parser.h
 */

typedef struct Classinfo{
	unsigned int mask;
	char *name;    /*default name*/
	char *fname;   /*name in filepaths*/
	char *sname;   /*name in the schema*/
	unsigned int id;
	unsigned int bodymask;
}Classinfo;

typedef struct Bodyinfo{
	unsigned int mask;
	char *name;
	char *sname;
}Bodyinfo;

unsigned int formatpaths(const Entry *, unsigned int, char [CLASSCOUNT][NAVBUFSIZE]);

const Classinfo * getclass_b(unsigned int);
const Classinfo * getclass_i(unsigned int);
const Classinfo * getclass_n(const char *);
const Bodyinfo * getbody_n(const char *);
const Bodyinfo * getbody_b(unsigned int);
void printbody_b(unsigned int);
