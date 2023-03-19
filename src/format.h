/*
 * requires: navvdf.h, parser.h
 */

#define CLASSFILTER 0xff

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

typedef struct Classstr{
	char *name; /*default name, should be printed to stdout*/
	char *sname; /*as it appears in the item schema*/
	char *fname; /*as it appears in filenames*/
}Classstr;

int formatclasses(const Entry *, Classdata *);
int formatpaths(const Entry *, const Classdata *);
int getclassid(const char *);
const Classstr * getclassstr(int);
void printclassb(unsigned int);
unsigned int formatpaths2(const Entry *, unsigned int, char [CLASSCOUNT][NAVBUFSIZE]);

const Classinfo * getclass_b(unsigned int);
const Classinfo * getclass_i(unsigned int);
const Classinfo * getclass_n(const char *);
const Bodyinfo * getbody_n(const char *);
const Bodyinfo * getbody_b(unsigned int);
void printbody_b(unsigned int);
