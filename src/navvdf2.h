/*
 *requires: navvdf.h
 */

typedef struct Tree Tree;

typedef struct Entry2{
	struct Entry2 *parent;
	struct Entry2 **childs;
	unsigned int childc;
	unsigned int childm;

	char *name;
	char *val;
	enum NAVTYPE type;
}Entry2;

Tree * navgentree(char *, unsigned int);
int navto2(Tree *, const char *);
const char *entryname(Tree *, int);
const char *entryval(Tree *, int);
