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
static unsigned int getbodys(const Pos *);
static int setbitc(unsigned int);


int
updater_print(const Pos *pos)
{
	Mdl m = {0};
	Entry *e;
	Entry *hat = pos->p[pos->i];
	int i;
	Pos lpos = *pos;

	strncpy(m.suffix, hat->name, NAVBUFSIZE-1);

	/*if the hat doesn't possess bodygroup entries, create a vtx instead*/
	if(!(m.bodyb |= getbodys(pos)))
		m.vtx = 1;

	if(navopen2(hat, "item_name", &e) == 0)
		strncpy(m.qc, e->val, NAVBUFSIZE-1);
	else
		strncpy(m.qc, "#NONAME", NAVBUFSIZE-1);

	m.classb |= getclasses(hat);
	m.new |= getpaths(hat, &m);

	/*Some hats use the same model for multiple classes.
	 *As it is impossible to use bodygroups of multiple classes
	 *on the same model, I just create a vtx instead.
	 *See the hat "Honest Halo" in the schema for an example.
	 */
	if(m.solemodel && setbitc(m.classb) > 1){
		m.force1vtx = 1;
		m.vtx = 1;
	}

	if(navto2(&lpos, "visuals/styles") < 0){
		output(&m);
		return 0;
	}
	e = lpos.p[lpos.i];

	for(i = 0; i < e->childc; i++){
		Entry *child;

		m.new |= getpaths(e->childs[i], &m);

		if(m.solemodel && setbitc(m.classb) > 1){
			m.force1vtx = 1;
			m.vtx = 1;
		}

		if(navopen2(e->childs[i], "name", &child) == 0)
			strncpy(m.qc, child->val, NAVBUFSIZE-1);
		else
			sprintf(m.qc, "#NONAME_STYLE%d", i); /*damn you c89*/

		output(&m);

		m.new = 0; /*do not move*/
	}

	return 0;
}

static unsigned int
getbodys(const Pos *pos)
{
	Entry *e;
	unsigned int mask = 0;
	Pos lpos = *pos;
	int i;

	/*This gets the bodygroup of all the styles at once too, since some
	 *styles only toggle a bodygroup on without providing a new model, so
	 *there's no possibility to create an entry for each style.
	 */

	/*This also brings the problem that the code can't toggle a bodygroup
	 *on and off, even if the style does use a different model. Improvements
	 *can be done here, but I think the code is complex enough as it is.
	 */

	if(navto2(&lpos, "visuals/player_bodygroups") == 0){
		e = lpos.p[lpos.i];
		for(i = 0; i < e->childc; i++)
			if(e->childs[i]->val[0]-'0' == 1)
				mask |= getbody_n(e->childs[i]->name)->mask;
		lpos = *pos;
	}

	if(navto2(&lpos, "visuals/styles") == 0){
		Entry *child;
		e = lpos.p[lpos.i];
		for(i = 0; i < e->childc; i++)
			if(navopen2(e->childs[i], "additional_hidden_bodygroups", &child) == 0){
				int j;
				for(j = 0; j < child->childc; j++)
					mask |= getbody_n(child->childs[j]->name)->mask;
			}
		}
	return mask;
}

/*Warning: some obscure hats like "The Grandmaster" are NOT
 *compatible with this function, and with the code in general.
 *Improvements are needed.
 */
static unsigned int
getpaths(const Entry *p, Mdl *m)
{
	Entry *e;
	unsigned int mask = 0;
	int i;

	/*"model_player_per_class" takes precedence over "model_player".
	 *"Team Captain" and the "Prinny Pouch" include (and I wonder why)
	 *both entries, and the paths in "model_player" are either wrong or
	 *incomplete.
	 */
	if(navopen2(p, "model_player_per_class", &e) == 0){
		for(i = 0; i < e->childc; i++)
			mask |= formatpaths(e->childs[i], m->classb, m->paths);
		return mask;
	}

	/*Should copy the path in the right slot.
	 *This assumes that entries with a "model_player" entry are
	 *for one class only, hence it simply returns classb as the mask.
	 */
	if(navopen2(p, "model_player", &e) == 0 && strlen(e->val) > 0){ /*strlen bc Web Easteregg Medal*/
		strncpy(m->paths[getclass_b(m->classb)->id], e->val, NAVBUFSIZE-1);
		m->solemodel = 1;
		return m->classb;
	}
	return 0;
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
