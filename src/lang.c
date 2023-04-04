#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg.h"
#include "file.h"
#include "navvdf.h"
#include "lang.h"


static Tree *lang;
static Pos p = {0};


int
lang_init(void)
{
	char *txt;
	FILE *f = fopen(arg_getlangpath(), "rb");
	if(!f)
		return -1;

	if(loadf(f, &txt)<0)
		return -1;

	/*For some reason, TF2's language files begin with a header
	 *of 6 bytes, starting with the char 0xEF (U+FFFD) "replacement
	 *character". This crudely checks for that and replaces the
	 *header with spaces if it's the case.*/

	if((unsigned char)txt[0] == 0xEF)
		memset(txt, ' ', 6);

	lang = navgentree(txt, 2048);
	if(!lang){
		free(txt);
		return -1;
	}
	pos_init(&p, lang);
	if(navto2(&p, "/lang/Tokens")<0){
		/*TODO: free the tree*/
		return -1;
	}

	return 0;
}

void
lang_free(void)
{
	/*TODO*/
}
