#include <stdio.h>
#include <string.h>

#include "arg.h"
#include "navvdf.h"
#include "parser.h"
#include "format.h"
#include "custom.h"

#define MAXPATHS CLASSCOUNT * 2

typedef struct Mdl{
	char name[NAVBUFSIZE];
	char paths[MAXPATHS][NAVBUFSIZE];
	unsigned int pmask;
	unsigned int cmask;
	unsigned long long int qmask;
}Mdl;

static void output(const Mdl *);


void
custom_printheader(void)
{
	char sep = arg_getsep();
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

	output(&m);

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

	/*paths are not yet implemented*/

	printf("\n");

	return;
}
