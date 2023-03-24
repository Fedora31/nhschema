
#define NAVBUFSIZE 512

enum NAVTYPE{
	NAVDIR,
	NAVFILE
};

typedef struct Entry{
	enum NAVTYPE type;
	char name[NAVBUFSIZE];
	char val[NAVBUFSIZE];
	char *link;
	char *start;
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
int naventry(const Entry *, const char *, Entry *);
int navopene(const Entry *, const char *, Entry *);
int navbreak(Entry *);

int navnextentry(char **, Entry *);
int navjump(char **);
