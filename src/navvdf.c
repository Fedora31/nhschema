#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "str.h"
#include "navvdf.h"


/*
 *A key-value pair is considered as a file (NAVFILE). A subkey is
 *considered as a directory (NAVDIR).
 */

/*Go to the different NAVDIRS specified in path.
 */
int
navto(Pos *p, const char *path)
{
	char tmp[NAVBUFSIZE] = {0};
	char *label;

	/*if absolute path, set p to start (root)*/
	if(path[0] == '/')
		p->p = p->start;

	strncpy(tmp, path, NAVBUFSIZE-1);

	for(label = strtok(tmp, "/"); label != NULL; label = strtok(NULL, "/")){
		if(strcmp(label, ".") == 0)
			continue;
		if(strcmp(label, "..") == 0){
			navreturn(p);
			continue;
		}
		if(navopen(p, label) < 0)
			return -1;
	}

	return 0;
}

/*Go to the child NAVDIR with the given name.
 *TODO: make it use navnextentry instead?
 */
int
navopen(Pos *p, const char *label)
{
	Entry e;
	char *lp = p->p;

	while(navnextentry(&lp, &e) == 0)
		if(strcmp(e.name, label) == 0 && e.type == NAVDIR){
			p->p = e.link;
			return 0;
		}
	return -1;
}

int
navgetwd(const Pos *p, Entry *e)
{
	/*TODO: this should not rely on double quotes to be present*/

	int dq = 0, len;
	char *lp = p->p;

	while(dq < 2 && lp > p->start){
		lp--;
		if(lp[0] == '"')
			dq++;
	}
	if(dq != 2){
		fprintf(stderr, "navgetwd: err: double quotes missing\n");
		return -1;
	}

	/*advance 1 due to the first double quote*/
	lp++;

	len = strchr(lp, '"') - lp;
	strncpy(e->name, lp, len);
	e->name[len] = '\0';
	e->type = NAVDIR;
	e->link = p->p;

	return 0;
}

/*Get into the next NAVDIR.
 */
int
navnext(Pos *p)
{
	int level;

	/*get the next '{' on the same level, if any*/
	for(level = 0; (p->p[0] != '{' || level >= 1) && p->p[0] != '\0'; p->p++){
		if(p->p[0] == '{')
			level++;
		if(p->p[0] == '}')
			level--;
		/*printf("  level %d %c\n", level, p->p[0]);*/
	}

	if(level < 0){
		fprintf(stderr, "err: arrived at end of block\n");
		return -1;
	}
	if(p->p[0] == '\0'){
		/*the file likely has a missing closing brace*/
		fprintf(stderr, "navnext(): err: abruptly arrived at EOF\n");
		return -1;
	}

	/*go past the '{'*/
	p->p++;

	return 0;
}

/*Get out of the current NAVDIR.
 */
int
navjump(char **p)
{
	int level;

	/*get to the closing '}' on the same level*/
	for(level = 0; ((*p)[0] != '}' || level >= 1) && (*p)[0] != '\0'; (*p)++){
		if((*p)[0] == '{')
			level++;
		if((*p)[0] == '}')
			level--;
		/*printf("j level %d %c\n", level, (*p)[0]);*/
	}

	if((*p)[0] == '\0'){
		/*the file likely has a missing closing brace*/
		fprintf(stderr, "navjump(): err: abruptly arrived at EOF\n");
		return -1;
	}

	/*get past the closing brace*/
	(*p)++;
	return 0;
}

/*Get out of the current NAVDIR and return to the top of the entries
 *in the parent NAVDIR.
 */
int
navreturn(Pos *p)
{
	int level;

	/*get to the start of the current level*/
	for(level = 0; (p->p[0] != '{' || level >= 0) && p->p > p->start; p->p--){
		if(p->p[0] == '{')
			level--;
		if(p->p[0] == '}')
			level++;
		/*printf("w level %d %c\n", level, p->p[0]);*/
	}

	if(p->p <= p->start){
		fprintf(stderr, "navreturn(): err: arrived at SOF\n");
		p->p = p->start;
	}

	/*get past the opening brace*/
	p->p++;

	return 0;
}

int
naventry(const Entry *p, const char *path, Entry *entry)
{
	char tmp[NAVBUFSIZE];
	char *name;
	char *ptr = tmp;
	Entry e = *p;

	strncpy(tmp, path, NAVBUFSIZE-1);

	/*TODO: implement . .. and /*/
	while((name = bstrtok_r(&ptr, "/")) != NULL){
		if(navopene(&e, name, &e) < 0)
			return -1;
	}
	*entry = e;
	return 0;
}

int
navbreak(Entry *e)
{
	char *lp = e->link;
	Pos p;

	if(navjump(&lp)<0)
		return -1;
	p.p = lp;
	return navgetwd(&p, e);
}

int
navopene(const Entry *p, const char *label, Entry *r)
{
	Entry e;
	char *lp = p->link;

	if(p->type == NAVFILE)
		return -1;
	while(navnextentry(&lp, &e) == 0){
		if(strcmp(e.name, label) == 0){
			*r = e;
			return 0;
		}
	}
	return -1;
}


/*Search for the next entry in the current NAVDIR.
 */
int
navnextentry(char **p, Entry *entry)
{
	/*TODO: this should not rely on double quotes to be present*/

	char *name, *val, *save = *p;
	int dq = 0, new = 1, len;

	memset(entry, 0, sizeof(Entry));

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
			len = strchr(name, '"') - name;
			strncpy(entry->name, name, len < NAVBUFSIZE-1 ? len : NAVBUFSIZE-1);
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
			len = strchr(val, '"') - val;
			strncpy(entry->val, val, len < NAVBUFSIZE-1 ? len : NAVBUFSIZE-1);
			entry->type = NAVFILE;
			entry->link = save;
			return 0;
		}

		if((*p)[0] == '{'){
			/*NAVDIR*/
			/*get over the opening brace to set the link and call navjump()*/
			(*p)++;
			entry->link = *p;
			entry->type = NAVDIR;
			navjump(p);
			return 0;
		}
	}

	/*to go behind the closing brace or '\0' again*/
	(*p)--;
	return -1;
}
