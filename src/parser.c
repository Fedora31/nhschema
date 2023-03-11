#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stack.h>
#include "parser.h"


#define BUFSIZE 512

enum NAVTYPE{
	NAVDIR,
	NAVFILE
};

typedef struct Entry{
	enum NAVTYPE type;
	char name[BUFSIZE];
	char val[BUFSIZE];
}Entry;

static int navto(char *, char **, const char *);
static int navnext(char **);
static int navjump(char **);
static char * getname(char *, char *);
static int navreturn(char *, char **);
static int navscan(char *, Stack *);
static int navnextentry(char **, Entry *);


int
parse(char *str)
{
	Stack res;
	char *p = str;
	Entry *e;
	int i = 0;

	stack_init(&res, 1024, 1024, sizeof(Entry));


	if(navto(str, &p, "items_game") < 0){
		fprintf(stderr, "err: couldn't find section \"items_game\"\n");
		return -1;
	}
	navto(str, &p, "items");
	navto(str, &p, "30091");
	navscan(p, &res);

	for(; (e = stack_getnextused(&res, &i)) != NULL; ){
		printf("TYPE=%d ", e->type);
		printf("NAME=%s ", e->name);
		if(e->type == NAVFILE)
			printf("VAL=%s", e->val);
		printf("\n");
	}

	stack_free(&res);
	return 0;
}

static int
navto(char *start, char **p, const char *label)
{
	int res = -1;
	char *match;

	while(1){

		if(navnext(p) < 0)
			return -1;

		/*get the current label*/
		if((match = getname(start, *p)) == NULL)
			return -1;

		if(strcmp(match, label) == 0){
			printf("OK!\n");
			res = 0;
			free(match);
			break;
		}

		/*not a match*/

		free(match);
		if(navjump(p) < 0)
			return -1;
	}
	return res;
}

/*Get the name of the current NAVDIR. The returned
 *char * must be freed after use.
 */
static char *
getname(char *start, char *p)
{
	int dq = 0, len;
	char *name;

	while(dq < 2 && p > start){
		p--;
		if(p[0] == '"')
			dq++;
	}
	if(dq != 2){
		fprintf(stderr, "getname: err: double quotes missing\n");
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
static int
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
static int
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
static int
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
static int
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

/*Search for the next entry in the current NAVDIR.
 */
static int
navnextentry(char **p, Entry *entry)
{
	char *name, *val;
	int dq = 0, new = 1, len;

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
