#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <navvdf.h>
#include "arg.h"
#include "file.h"
#include "lang.h"


static Vdftree l;
static Vdfpos o;


int
lang_init(void)
{
	FILE *f = fopen(arg_getlangpath(), "rb");
	if(!f)
		return -1;

	if(vdf_loadf(&l, f, '\r', VDF_ESCSEQ) < 0){
		fclose(f);
		return -1;
	}
	fclose(f);

	vdf_posinit(&o, &l);

	if(vdf_nav(&o, "\rlang\rTokens", &o) < 0){
		vdf_free(&l);
		return -1;
	}
	return 0;
}

const char *lang_get(const char *key)
{
	const char *val;
	if(!(val = vdf_valptr(&o, key)))
		return key;
	return val;
}

void
lang_free(void)
{
	vdf_free(&l);
}
