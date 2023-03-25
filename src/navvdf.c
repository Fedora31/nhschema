#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pool.h>
#include "str.h"
#include "navvdf.h"

struct Tree{
	char *buf;
	Pool pool;
	Entry2 *root;
};

enum NAVRES{
	NAVNOMATCH = -2,
	NAVISFILE = -3,
	NAVEOD = 3
};

static int fnextentry(char **, Entry2 *);
static int genentries(Tree *);
static int entryinit(Entry2 *, Entry2 *);
static int entryaddto(Entry2 *, Entry2 *);
static int mergeprefabs(Tree *);
static int navmerge(Entry2 *, Entry2 *);
static int mergeduplicates(Entry2 *);


Tree *
navgentree(char *buf, unsigned int alloc)
{
	Tree *t = calloc(1, sizeof(Tree));
	t->buf = buf;

	pool_init(&t->pool, alloc, sizeof(Entry2));
	genentries(t);
	mergeprefabs(t);
	return t;
}

void
pos_init(Pos2 *p, Tree *t)
{
	p->i = 0;
	p->p[p->i] = t->root;
}

int
navto2(Pos2 *pos, const char *path)
{
	char tmp[NAVBUFSIZE] = {0};
	char *name, *ptr = tmp;
	int res = 0, i = pos->i;

	strncpy(tmp, path, NAVBUFSIZE-1);

	/*absolute path*/
	if(tmp[0] == '/'){
		i = 0;
		ptr++;
	}

	while((name = bstrtok_r(&ptr, "/")) != NULL){
		if(strcmp("..", name) == 0){
			i -= i == 0 ? 0 : 1;
			continue;
		}
		if(i >= TREEDEPTH-1)
			return -1;
		if((res = navopen2(pos->p[i], name, &pos->p[i+1]))<0)
			break;
		i++;
	}
	if(res >= 0)
		pos->i = i;
	return res;
}

int
navtoi(Pos2 *pos, int i)
{
	if(i < 0 || i >= pos->p[pos->i]->childc)
		return -1;
	if(pos->i >= TREEDEPTH-1)
		return -1;

	pos->p[pos->i+1] = pos->p[pos->i]->childs[i];
	pos->i++;
	return 0;
}

int
navopen2(const Entry2 *e, const char *name, Entry2 **to)
{
	int i;

	/*you can't cd to a file*/
	if(e->type == NAVFILE)
		return NAVISFILE;
	for(i = 0; i < e->childc; i++){
		if(strcmp(e->childs[i]->name, name) == 0){
			*to = e->childs[i];
			return 0;
		}
	}
	return NAVNOMATCH;
}

static int
genentries(Tree *t)
{
	int res = 0, handle = 0;
	Entry2 *parent, *child;
	char *p = t->buf;
	Pool *pool = &t->pool;

	t->root = parent = pool_getslot(pool, &handle);
	entryinit(NULL, t->root);
	t->root->type = NAVDIR; /*we won't be able to cd otherwise*/

	pool_set(pool, handle++);
	child = pool_getslot(pool, &handle);
	entryinit(parent, child);

	while((res = fnextentry(&p, child)) >= 0){
		if(res == NAVEOD){
			parent = parent->parent;
			child->parent = parent;
			continue;
		}

		if(entryaddto(parent, child)<0){
			fprintf(stderr, "err: couldn't add child to parent\n");
			return -1;
		}

		if(res == NAVDIR)
			parent = child;

		pool_set(pool, handle++);
		if((child = pool_getslot(pool, &handle)) == NULL){
			fprintf(stderr, "err: couldn't create child\n");
			return -1;
		}
		if(entryinit(parent, child)<0){
			fprintf(stderr, "err: couldn't initialize child\n");
			return -1;
		}
	}
	return 0;
}

static int
mergeprefabs(Tree *t)
{
	int i, handle;
	Entry2 *curr, *preffile;
	char tmp[NAVBUFSIZE] = {0}, *ptr;
	char path[NAVBUFSIZE] = "/items_game/prefabs/";
	char *prefab;
	Pos2 pos;

	pos_init(&pos, t);

	for(i = 0, handle = 0;; i++, handle++){
		if((curr = pool_getnextused(&t->pool, &handle)) == NULL)
			break;

		/*from this point, should be done in a loop? or since it's going from start to finish, there's no need to?*/

		if(navopen2(curr, "prefab", &preffile) < 0)
			continue;

		ptr = tmp;

		strncpy(tmp, preffile->val, NAVBUFSIZE-1);
		while((prefab = bstrtok_r(&ptr, " ")) != NULL){
			/*but what if the prefab has a looong name? strncpy doesn't check the destination buffer...*/
			strcpy(strrchr(path, '/')+1, prefab);

			if((navto2(&pos, path))<0){
				fprintf(stderr, "warn: prefab \"%s\" not found\n", path);
				continue;
			}
			navmerge(pos.p[pos.i], curr);
			mergeduplicates(curr);
		}

	}
	return 0;
}

Entry2 *
entrygeti(const Entry2 *e, int i)
{
	if(i < 0 || i >= e->childc)
		return NULL;
	return e->childs[i];
}

int
entrycontains(const Entry2 *e, const char *name)
{
	int i;
	for(i = 0; i < e->childc; i++)
		if(strcmp(e->childs[i]->name, name) == 0)
			return i;
	return -1;
}

static int
mergeduplicates(Entry2 *e)
{
	/*Yes, this doesn't actually merge entries together, but rather makes it so
	 *that all duplicate entries have the same content. And yes, this results in
	 *a lot of wasted space.
	 */

	/*This function was created because some hats (e.g. The Cute Suit) have a
	 *duplicated "visuals" entry, one which contains bodygroup info, the other
	 *style info. Since the code only searches for one entry, this ensures
	 *that all the needed information is available in the same entry.
	 */

	int i, j;
	for(i = 0; i < e->childc; i++){
		if(e->childs[i]->type == NAVFILE)
			continue;

		for(j = 0; j < e->childc; j++){
			if(e->childs[j]->type == NAVFILE || j == i)
				continue;

			if(strcmp(e->childs[i]->name, e->childs[j]->name) == 0)
				navmerge(e->childs[j], e->childs[i]);
		}
	}
	return 0;
}

static int
navmerge(Entry2 *from, Entry2 *to)
{
	int i, res;
	Entry2 *curr;

	/*Files that have the same name as other entries are simply added in the list.
	 *Since they are added after the original content, they won't be returned by
	 *entrycontains() or other functions that only check the name.
	 */

	/*printf("MERGE %s TO %s\n", from->name, to->name);*/

	if(from == to){
		fprintf(stderr, "warn: tried to merge entry \"%s\" on itself\n", from->name);
		return 0;
	}

	for(i = 0; (curr = entrygeti(from, i)) != NULL; i++){
		if((res = entrycontains(to, curr->name)) < 0 || curr->type == NAVFILE){
			if(entryaddto(to, curr)<0){
				fprintf(stderr, "err: navmerge(): couldn't add child to parent\n");
				return -1;
			}
		}else
			if(navmerge(curr, entrygeti(to, res)) < 0)
				return -1;
	}
	return 0;
}

static int
entryinit(Entry2 *parent, Entry2 *e)
{
	e->parent = parent;
	e->childc = 0;
	e->childm = 5;
	e->name = NULL;
	e->val = NULL;
	e->childs = NULL;
	e->type = NAVFILE;
	if((e->childs = malloc(sizeof(Entry2*) * e->childm)) == NULL)
		return -1;
	return 0;
}

static int
entryaddto(Entry2 *parent, Entry2 *child)
{
	if(parent->childc >= parent->childm){
		parent->childm += 5;
		if((parent->childs = realloc(parent->childs, parent->childm * sizeof(Entry2**))) == NULL)
			return -1;
	}
	parent->childs[parent->childc++] = child;
	return 0;
}

static int
fnextentry(char **p, Entry2 *e)
{
	/*TODO: this should not rely on double quotes to be present*/

	char *name, *val;
	int dq = 0, new = 1;

	/*4 consecutive double quotes means the entry is a NAVFILE.
	 *encountering an opening brace means the entry is a NAVDIR.
	 */

	for(; (*p)[0] != '}' && (*p)[0] != '\0'; (*p)++){

		if((*p)[0] == '"'){
			dq++;
			new = 1;
		}

		if(dq == 1 && new){
			name = (*p)+1;
			new = 0;
		}

		if(dq == 2 && new){
			strchr(name, '"')[0] = '\0';
			e->name = name;
			new = 0;
		}

		if(dq == 3 && new){
			val = (*p)+1;
			new = 0;
		}

		if(dq == 4){
			/*NAVFILE*/
			/*get over the last quote for any future calls*/
			(*p)++;
			strchr(val, '"')[0] = '\0';
			e->val = val;
			e->type = NAVFILE;
			return NAVFILE;
		}

		if((*p)[0] == '{'){
			/*NAVDIR*/
			/*get over the opening brace to set the link and call navjump()*/
			(*p)++;
			e->type = NAVDIR;
			return NAVDIR;
		}
	}

	if((*p)[0] == '}'){
		(*p)++;
		return NAVEOD;
	}
	/*'\0'*/
	(*p)--;
	return -1;
}
