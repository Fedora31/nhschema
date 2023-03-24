#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pool.h>
#include "str.h"
#include "navvdf.h"
#include "navvdf2.h"

#define TREEDEPTH 128

struct Tree{
	char *buf;
	Pool pool;
	Entry2 *path[TREEDEPTH];
	unsigned int i;
};

enum NAVRES{
	NAVDEFERR = -1,
	NAVNOMATCH = -2,
	NAVISFILE = -3,
	NAVEOD = 3
};

static int fnextentry(char **, Entry2 *);
static int genentries(Tree *);
static int entryinit(Entry2 *, Entry2 *);
static int entryaddto(Entry2 *, Entry2 *);
static int mergeprefabs(Tree *);
static int navopen2(Entry2 *, const char *, Entry2 **);
static int navmerge(Entry2 *, Entry2 *);


Tree *
navgentree(char *buf, unsigned int alloc)
{
	Tree *t = calloc(1, sizeof(Tree));
	t->buf = buf;
	t->i = 0;

	pool_init(&t->pool, alloc, sizeof(Entry2));
	genentries(t);
	mergeprefabs(t);
	return t;
}

int
navto2(Tree *t, const char *path)
{
	char tmp[NAVBUFSIZE] = {0};
	char *name, *ptr = tmp;
	int res = 0, i = t->i;

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
		if((res = navopen2(t->path[i], name, &t->path[i+1]))<0)
			break;
		i++;
	}
	if(res >= 0)
		t->i = i;
	return res;
}

static int
navopen2(Entry2 *e, const char *name, Entry2 **to)
{
	unsigned int i;

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

	t->path[0] = parent = pool_getslot(pool, &handle);
	entryinit(NULL, t->path[0]);
	t->path[0]->type = NAVDIR; /*we won't be able to cd otherwise*/

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

		if(res == NAVFILE){
			printf("%d: file: %s\n", handle, child->name);
		}else if(res == NAVDIR){
			printf("%d: dir: %s\n", handle, child->name);
			parent = child;
		}

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

	for(i = 0, handle = 0;; i++, handle++){
		if((curr = pool_getnextused(&t->pool, &handle)) == NULL)
			break;

		/*from this point, should be done in a loop? or since it's going from start to finish, there's no need to?*/

		if(navopen2(curr, "prefab", &preffile) < 0)
			continue;

		printf("%d:%d: got: %s (%s)\n", i, handle, curr->name, preffile->val);
		ptr = tmp;

		strncpy(tmp, preffile->val, NAVBUFSIZE-1);
		while((prefab = bstrtok_r(&ptr, " ")) != NULL){
			/*but what if the prefab has a looong name? strncpy doesn't check the destination buffer...*/
			strcpy(strrchr(path, '/')+1, prefab);

			if(navto2(t, path)<0){
				fprintf(stderr, "warn: prefab \"%s\" not found\n", path);
				continue;
			}
			printf(">%s\n", path);

			navmerge(t->path[t->i], curr);
		}
	}

	navto2(t, "/");

	return 0;
}

static int
navmerge(Entry2 *from, Entry2 *to)
{
	printf("MERGE %s TO %s\n", from->name, to->name);

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
		if((parent->childs = realloc(parent->childs, parent->childm * sizeof(Entry**))) == NULL)
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
