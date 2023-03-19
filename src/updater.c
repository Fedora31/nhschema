#include <stdio.h>
#include <string.h>

#include "str.h"
#include "navvdf.h"
#include "updater.h"
#include "parser.h"
#include "format.h"

typedef struct Mdl{
	char qc[NAVBUFSIZE];
	char paths[CLASSCOUNT][NAVBUFSIZE];
	char suffix[NAVBUFSIZE];
	unsigned int classb;
	unsigned int bodyb;
	unsigned int new;
	int solemodel;
	int vtx;
	int force1vtx;
}Mdl;

static void output(const Mdl *);
static unsigned int getpaths(const Entry *, Mdl *);
static unsigned int getbodys(const Entry *);
static int setbitc(unsigned int);


int
updater_print(const Entry *hat)
{
	Mdl m = {0};
	Entry e;
	char *lp;

	strncpy(m.suffix, hat->name, NAVBUFSIZE-1);

	/*if the hat doesn't possess bodygroup entries, create a vtx instead*/
	if(!(m.bodyb |= getbodys(hat)))
		m.vtx = 1;

	if(getentry(hat, "item_name", &e) == 0)
		strncpy(m.qc, e.val, NAVBUFSIZE-1);
	/*printf("ITEM: %s\n", e.val);*/

	if(getentry(hat, "used_by_classes", &e) < 0){
		fprintf(stderr, "err: couldn't get classes\n");
		return -1;
	}
	for(lp = e.link; navnextentry(&lp, &e) == 0; )
		m.classb |= getclass_n(e.name)->mask;

	m.new |= getpaths(hat, &m);

	/*Some hats use the same model for multiple classes.
	 *As it is impossible to use bodygroups of multiple classes
	 *on the same model, I just treat them as regular hats.
	 *See the hat "Honest Halo" in the schema for an example.*/
	if(m.solemodel && setbitc(m.classb) > 1){
		m.force1vtx = 1;
		m.vtx = 1;
	}

	if(getentry(hat, "visuals/styles", &e) < 0){
		output(&m);
		return 0;
	}

	for(lp = e.link; navnextentry(&lp, &e) == 0; ){
		m.new |= getpaths(&e, &m);

		if(m.solemodel && setbitc(m.classb) > 1){
			m.force1vtx = 1;
			m.vtx = 1;
		}

		if(getentry(&e, "name", &e) == 0)
			strncpy(m.qc, e.val, NAVBUFSIZE-1);

		output(&m);

		m.new = 0; /*do not move*/
	}

	return 0;
}

static unsigned int
getbodys(const Entry *p)
{
	Entry e;
	char *lp;
	char *lp2;
	char *prefab;
	char *ptr = e.val;
	unsigned int mask = 0;

	/*I get the bodygroup of all the styles at once too, since some styles
	 *only toggle a bodygroup on without providing a new model, so there's
	 *no possibility to create an entry for each style.
	 */

	if(getentry(p, "visuals/player_bodygroups", &e) == 0)
		for(lp = e.link; navnextentry(&lp, &e) == 0; )
			if(e.val[0]-'0' == 1)
				mask |= getbody_n(e.name)->mask;

	if(getentry(p, "visuals/styles", &e) == 0)
		for(lp = e.link; navnextentry(&lp, &e) == 0; )
			if(getentry(&e, "additional_hidden_bodygroups", &e) == 0)
				for(lp2 = e.link; navnextentry(&lp2, &e) == 0; )
					mask |= getbody_n(e.name)->mask;

	if(getentry(p, "prefab", &e) < 0)
		return mask;

	/*recursively get any other bodygroups that might be in prefabs
	 *Some hats do this (e.g. chill chullo)
	 */
	while((prefab = bstrtok_r(&ptr, " ")) != NULL){
		Entry child;
		if(getprefab(prefab, &child) < 0)
			continue;
		mask |= getbodys(&child);
	}

	return mask;
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
	if(getentry(p, "model_player", &e) == 0 && strlen(e.val) > 0){ /*strlen bc Web Easteregg Medal*/
		strncpy(m->paths[getclass_b(m->classb)->id], e.val, NAVBUFSIZE-1);
		m->solemodel = 1;
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
	unsigned int new = m->new;

	/*this force to only print the first new path*/
	if(m->force1vtx)
		new = getclass_b(new)->mask;
	if(m->vtx){
		for(i = 0; i < CLASSCOUNT; i++)
			if(new >> i & 1)
				printf("%s vtx\n", m->paths[i]);
		return;
	}

	for(i = 0; i < CLASSCOUNT; i++){
		/*Only print classes that:
		 *1. are used by the hat (classb)
		 *2. have recently changed (new)
		 */
		if((m->classb & new) >> i & 1){
			/*check if the bodyparts are relevant to the current class*/
			remainder = m->bodyb & getclass_i(i)->bodymask;
			if(!remainder){
				printf("%s vtx\n", m->paths[i]);
				continue;
			}
			printf("%s %s_%s.qc %s animation ", m->paths[i], m->qc, m->suffix, getclass_i(i)->fname);
			printbody_b(remainder);
			printf("\n");
		}
	}
}

static int
setbitc(unsigned int b)
{
	/*not fast, but idc*/
	int res = 0;
	for(; b; b >>= 1)
		if(b & 1)
			res++;
	return res;
}
