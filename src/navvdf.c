#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stack.h>
#include "navvdf.h"


/*
 *A key-value pair is considered as a file (NAVFILE). A subkey is
 *considered as a directory (NAVDIR).
 */

/*Go to the different NAVDIRS specified in path.
 */
int
navto(char *start, char **p, const char *path)
{
	char tmp[BUFSIZE] = {0};
	char *label;

	/*if absolute path, set p to start (root)*/
	if(path[0] == '/')
		(*p) = start;

	strncpy(tmp, path, BUFSIZE-1);
	label = strtok(tmp, "/");

	do
		if(navopen(start, p, label) < 0)
			return -1;
	while((label = strtok(NULL, "/")) != NULL);

	return 0;
}

/*Go to the child NAVDIR with the given name.
 */
int
navopen(char *start, char **p, const char *label)
{
	char *match;

	while(1){
		if(navnext(p) < 0)
			return -1;

		/*get the current label*/
		if((match = navgetname(start, *p)) == NULL)
			return -1;

		if(strcmp(match, label) == 0){
			/*printf("OK!\n");*/
			free(match);
			return 0;
		}

		/*not a match*/

		free(match);
		if(navjump(p) < 0)
			return -1;
	}
	return -1;
}

/*Get the name of the current NAVDIR. The returned
 *char * must be freed after use.
 */
char *
navgetname(char *start, char *p)
{
	/*TODO: this should not rely on double quotes to be present*/

	int dq = 0, len;
	char *name;

	while(dq < 2 && p > start){
		p--;
		if(p[0] == '"')
			dq++;
	}
	if(dq != 2){
		fprintf(stderr, "navgetname: err: double quotes missing\n");
		return NULL;
	}

	/*advance 1 due to the first double quote*/
	p++;

	len = strchr(p, '"') - p;
	name = malloc(sizeof(char) * (len+1));
	strncpy(name, p, len);
	name[len] = '\0';

	return name;
}

/*Get into the next NAVDIR.
 */
int
navnext(char **p)
{
	int level;

	/*get the next '{' on the same level, if any*/
	for(level = 0; ((*p)[0] != '{' || level >= 1) && (*p)[0] != '\0'; (*p)++){
		if((*p)[0] == '{')
			level++;
		if((*p)[0] == '}')
			level--;
		/*printf("  level %d %c\n", level, (*p)[0]);*/
	}

	if(level < 0){
		fprintf(stderr, "err: arrived at end of block\n");
		return -1;
	}
	if((*p)[0] == '\0'){
		/*the file likely has a missing closing brace*/
		fprintf(stderr, "navnext(): err: abruptly arrived at EOF\n");
		return -1;
	}

	/*go past the '{'*/
	(*p)++;

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
navreturn(char *start, char **p)
{
	int level;

	/*get to the start of the current level*/
	for(level = 0; ((*p)[0] != '{' || level >= 0) && *p > start; (*p)--){
		if((*p)[0] == '{')
			level--;
		if((*p)[0] == '}')
			level++;
		/*printf("w level %d %c\n", level, (*p)[0]);*/
	}

	if(*p <= start){
		fprintf(stderr, "navreturn(): err: arrived at SOF\n");
		*p = start;
	}

	/*get past the opening brace*/
	(*p)++;

	return 0;
}

/*Fill the given stack with the entries from the current NAVDIR.
 */
int
navscan(char *p, Stack *res)
{
	int i;
	Entry *entry;

	entry = stack_getslot(res, &i);
	while(navnextentry(&p, entry) == 0){
		stack_setused(res, i);
		entry = stack_getslot(res, &i);
	}

	return 0;
}

int
naventry(Entry *entry, const char *label, char *p)
{
	while(navnextentry(&p, entry) == 0){
		if(strcmp(entry->name, label) == 0)
			return 0;
	}
	return -1;
}

/*Search for the next entry in the current NAVDIR.
 */
int
navnextentry(char **p, Entry *entry)
{
	/*TODO: this should not rely on double quotes to be present*/

	char *name, *val;
	int dq = 0, new = 1, len;

	memset(entry, 0, sizeof(Entry));

	/*4 consecutive double quotes means the entry is a NAVFILE.
	 *encountering an opening brace means the entry is a NAVDIR.
	 */

	for(; (*p)[0] != '}'; (*p)++){

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
			strncpy(entry->name, name, len < BUFSIZE-1 ? len : BUFSIZE-1);
			new = 0;
		}

		if(dq == 3 && new){
			val = (*p)+1;
			new = 0;
		}

		if(dq == 4){
			/*get over the last quote for any future calls*/
			(*p)++;
			len = strchr(val, '"') - val;
			strncpy(entry->val, val, len < BUFSIZE-1 ? len : BUFSIZE-1);
			entry->type = NAVFILE;
			return 0;
		}

		if((*p)[0] == '{'){
			/*get over the opening brace to call navjump()*/
			(*p)++;
			navjump(p);
			entry->type = NAVDIR;
			return 0;
		}
	}
	return -1;
}