
#define BUFSIZE 512

enum NAVTYPE{
	NAVDIR,
	NAVFILE
};

typedef struct Entry{
	enum NAVTYPE type;
	char name[BUFSIZE];
	char val[BUFSIZE];
	char *link;
}Entry;

typedef struct Pos{
	char *start;
	char *p;
}Pos;

int navto(Pos *, const char *);
int navopen(Pos *, const char *);
int navnext(Pos *);
int navgetwd(const Pos *, Entry *);
int navreturn(Pos *);
int naventry(const Pos *, const char *, Entry *);

int navnextentry(char **, Entry *);
int navjump(char **);
