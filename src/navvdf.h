/*
 * requires: stack.h
 */

#define BUFSIZE 512

enum NAVTYPE{
	NAVDIR,
	NAVFILE
};

typedef struct Entry{
	enum NAVTYPE type;
	char name[BUFSIZE];
	char val[BUFSIZE];
}Entry;

typedef struct Pos{
	char *start;
	char *p;
}Pos;

int navto(char *, char **, const char *);
int navopen(char *, char **, const char *);
int navnext(char **);
int navjump(char **);
char * navgetname(char *, char *);
int navreturn(char *, char **);
int navscan(char *, Stack *);
int navnextentry(char **, Entry *);
int naventry(Entry *, const char *, char *);
