#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stack.h>
#include "navvdf.h"
#include "parser.h"

static int ishat(char *, char *);
static int getentry(char *, char *, Entry *, char *);

int
parse(char *start)
{
	Stack item;
	char *p = start;

	if(navto(start, &p, "/items_game/items") < 0){
		fprintf(stderr, "err: couldn't open \"items_game/items\"\n");
		return -1;
	}

	while(navnext(&p) >= 0){
		Entry e;
		static int i = 0;
		printf("%d ", i++);
		stack_init(&item, 1024, 1024, sizeof(Entry));

		/*naventry(&e, "name", p);
		printf("%s\n", e.val);*/

		if(ishat(start, p))
			printf("hat.\n");

		stack_free(&item);
		navjump(&p);
	}
	return 0;
}

static int
ishat(char *start, char *p)
{
	Entry e, prefabs;
	char *prefab;
	char *p2 = start;

	/*this might need to be refined to exclude false positives*/
	if(naventry(&e, "item_slot", p) >= 0)
		if(strcmp(e.val, "head") == 0 || strcmp(e.val, "misc") == 0)
			return 1;

	/*if we get here, we need to look at prefabs for the item_slot entry*/
	if(naventry(&prefabs, "prefab", p) < 0)
		return 0;

	printf("hello\n");

	if(navto(start, &p2, "/items_game/prefabs") < 0){
		fprintf(stderr, "err: prefabs not found\n");
		return -1;
	}

	prefab = strtok(prefabs.val, " ");
	do{
		if(navto(start, &p2, prefab) < 0){
			fprintf(stderr, "err: unkown prefab \"%s\"", prefab);
			return -1;
		}

		/*this might need to be refined to exclude false positives*/
		if(naventry(&e, "item_slot", p2) >= 0)
			if(strcmp(e.val, "head") == 0 || strcmp(e.val, "misc") == 0){
				printf("YES\n");
				return 1;
			}

		navreturn(start, &p2);
	}while((prefab = strtok(NULL, " ")) != NULL);

	return 0;
}

/*Return the entry with the given name. The entry can be
 *from the current block or from a prefab or sub-prefab.
 */
static int
getentry(char *start, char *p, Entry *e, char *name)
{

}
