#include <stdio.h>
#include <string.h>

#include "str.h"
#include "arg.h"
#include "navvdf.h"
#include "parser.h"
#include "format.h"
#include "custom.h"

#define MAXPATHS 128

typedef struct Mdl{
	char name[NAVBUFSIZE];
	char paths[MAXPATHS][NAVBUFSIZE];
	int pathc;
	unsigned int pmask;
	unsigned int cmask;
	unsigned long long int qmask;
}Mdl;

static void getallpaths(const Entry *, Mdl *);
static void pathsformat(const Entry *, Mdl *);
static void output(const Mdl *);
static int pathexists(const Mdl *, const char *);


void
custom_printheader(void)
{
	char sep = arg_getsep();
	fprintf(stderr, "warn: nhcustom2's database generation feature is incomplete! Check the output data!\n");
	printf("hat%cclass%cequip%cdate%cupdate%cpath\n", sep, sep, sep, sep, sep);
}

int
custom_print(const Pos *p)
{
	Mdl m = {0};
	Entry *e, *hat = p->p[p->i];

	if(navopen2(hat, "name", &e) == 0)
		strncpy(m.name, e->val, NAVBUFSIZE-1);
	else
		snprintf(m.name, NAVBUFSIZE-1, "NONAME");

	m.cmask = getclasses(hat);
	m.qmask = getequips(hat);

	getallpaths(hat, &m);

	output(&m);

	return 0;
}

static void
getallpaths(const Entry *hat, Mdl *m)
{
	int i;
	Entry *e;

	/*If the hat possesses styles, only get the paths in them, we don't
	 *care about paths outside of styles (since they're duplicates)
	 */
	if(navopen2(hat, "visuals", &e) == 0 && navopen2(e, "styles", &e) == 0){
		for(i = 0; i < e->childc; i++)
			pathsformat(e->childs[i], m);
	}else
		pathsformat(hat, m);
}

static void
pathsformat(const Entry *path, Mdl *m)
{
	Entry *paths[MAXPATHS] = {0};
	char fpaths[CLASSCOUNT][NAVBUFSIZE] = {0};
	int i, pc;

	pc = getpaths2(path, paths, MAXPATHS);

	/*If only one path was found, chances are it's a basename path. And
	 *since it's alone, there's no need to generate all the paths for all the
	 *classes since nhcustom2 does this already, but only for paths that apply
	 *to all the classes.
	 */
	if(pc == 1){
		if(!pathexists(m, paths[0]->val)){
			strncpy(m->paths[m->pathc], paths[0]->val, NAVBUFSIZE-1);
			strswapall(m->paths[m->pathc++], "%s", "[CLASS]", NAVBUFSIZE-1);
		}
		return;
	}

	for(i = 0; i < MAXPATHS && paths[i]; i++)
		formatpaths(paths[i], m->cmask, fpaths);

	for(i = 0; i < CLASSCOUNT; i++){
		if(!(m->cmask >> i & 1))
			continue;
		if(!pathexists(m, fpaths[i]))
			strncpy(m->paths[m->pathc++], fpaths[i], NAVBUFSIZE-1);
	}

}

static int
pathexists(const Mdl *m, const char *p)
{
	int i;
	for(i = 0; i < m->pathc; i++){
		if(strcmp(m->paths[i], p) == 0)
			return 1;
	}
	return 0;
}

static void
output(const Mdl *m)
{
	int i, li;
	char sep = arg_getsep();

	printf("%s%c", m->name, sep);

	if(setbitc(m->cmask) >= CLASSCOUNT)
		printf("All classes");
	else{
		for(i = 0, li = 0; i < CLASSCOUNT; i++)
			if(m->cmask >> i & 1){
				if(li)
					printf("|");
				printf("%s", getclass_i(i)->name);
				li++;
			}
	}
	printf("%c", sep);

	if(m->qmask == 0)
		printf("None");
	else
		/*-1 bc it's undefined behaviour to shift with a value >= to the
		 *number of bits of the shifter var
		 */
		for(i = sizeof(unsigned long long int)*8-1, li = 0; i >= 0; i--)
			if(m->qmask >> i & 1){
				if(li)
					printf("|");
				printf("%s", getequip_b(1LLU << i)->name); /*I'm baffled*/
				li++;
			}
	printf("%c", sep);

	/*dates are not yet implemented*/

	printf("%c", sep);

	/*updates are not yet implemented*/

	printf("%c", sep);

	for(i = 0; i < MAXPATHS && strlen(m->paths[i]) > 0; i++){
		if(i)
			printf("|");
		printf("%s", m->paths[i]);
	}

	printf("\n");

	return;
}
