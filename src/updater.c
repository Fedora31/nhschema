#include <stdio.h>
#include <string.h>

#include "navvdf.h"
#include "updater.h"
#include "parser.h"
#include "format.h"

typedef struct Mdl{
	char fn[NAVBUFSIZE];
	char qc[NAVBUFSIZE];
	int class;

	char paths[CLASSCOUNT][NAVBUFSIZE];
	unsigned int classb;
	unsigned int bodyb;
	unsigned int new;
}Mdl;

static void output(const Mdl *);
static int unsigned getpaths(const Entry *, Mdl *);


int
updater_print(const Entry *hat)
{
	Mdl m = {0};
	Entry e;
	Classdata cdata;
	char *lp;

	m.new = ~(m.new & 0);

	if(getentry(hat, "visuals/player_bodygroups", &e) < 0)
		return 0;
	for(lp = e.link; navnextentry(&lp, &e) == 0; )
		if(e.val[0]-'0' == 1)
			m.bodyb |= getbody_n(e.name)->mask;

	if(getentry(hat, "item_name", &e) == 0)
		strncpy(m.qc, e.val, NAVBUFSIZE-1);

	if(getentry(hat, "used_by_classes", &e) < 0){
		fprintf(stderr, "err: couldn't get classes\n");
		return -1;
	}
	for(lp = e.link; navnextentry(&lp, &e) == 0; )
		m.classb |= getclass_n(e.name)->mask;

	m.new |= getpaths(hat, &m);

	if(getentry(hat, "visuals/styles", &e) < 0)
		output(&m);

/*	int i;
	for(i = 0; i < CLASSCOUNT; i++)
		printf("PATH: %s\n", m.paths[i]);
*/
	return 0;
}

static unsigned int
getpaths(const Entry *p, Mdl *m)
{
	Entry e;
	Entry child;
	char *lp;
	unsigned int mask = 0;


	/*Should copy the path in the right slot.
	 *This assumes that entries with a "model_player" entry are
	 *for one class only, hence it simply returns classb as the mask.
	 */
	if(getentry(p, "model_player", &e) == 0){
		strncpy(m->paths[getclass_b(m->classb)->id], e.val, NAVBUFSIZE-1);
		return m->classb;
	}
	if(getentry(p, "model_player_per_class", &e) == 0)
		for(lp = e.link; navnextentry(&lp, &child) == 0; )
			mask |= formatpaths2(&child, m->classb, m->paths);
	return mask;
}

static void
output(const Mdl *m)
{
	int i;
	unsigned int remainder;

	for(i = 0; i < CLASSCOUNT; i++){
		/*Only print classes that:
		 *1. are used by the hat (classb)
		 *2. have recently changed (new)
		 */
		if((m->classb & m->new) >> i & 1){
			/*check if the bodyparts are relevant to the current class*/
			remainder = m->bodyb & getclass_i(i)->bodymask;
			if(!remainder)
				continue;
			printf("%s %s %s ", m->paths[i], m->qc, getclass_i(i)->fname);
			printbody_b(remainder);
			printf("\n");
		}
	}






	/*printf("%s %s ", m->fn, m->qc);
	printclassb(m->classb);
	printbodyb(m->bodyb);
	printf("\n");*/
}
