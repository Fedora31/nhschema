/*
 *requires: navvdf.h
 */

#define TREEDEPTH 128
#define NAVBUFSIZE 512

enum NAVTYPE{
	NAVDIR,
	NAVFILE
};

typedef struct Tree Tree;

typedef struct Entry2{
	struct Entry2 *parent;
	struct Entry2 **childs;
	int childc;
	int childm;

	char *name;
	char *val;
	enum NAVTYPE type;
}Entry2;

typedef struct Pos2{
	Entry2 *p[TREEDEPTH];
	int i; /*pos->p[pos->i] is the current directory*/
}Pos2;


Tree * navgentree(char *, unsigned int);
void pos_init(Pos2 *, Tree *);
int navto2(Pos2 *, const char *);
int navtoi(Pos2 *, int);
Entry2 * navwd(Tree *);
int entrycontains(const Entry2 *, const char *);
Entry2 * entrygeti(const Entry2 *, int);
int navopen2(const Entry2 *, const char *, Entry2 **);
