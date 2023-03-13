#include <string.h>

#include "str.h"

/*A mix between the behaviour of strsep and strtok_r.
 *This acts like strtok as it handles multiple separators
 *but it requires the same arguments as strsep.
 */
char *
bstrtok_r(char **strp, const char *delim)
{
	char *start = *strp, *p = *strp;
	if(start == NULL)
		return start;
	while(1){
		start = p;
		if(*start == '\0')
			break;
		p = strpbrk(p, delim);
		if(p == NULL)
			*strp = NULL;
		else{
			if(p == start){
				p+=strspn(p, delim);
				continue;
			}
			*strp = p+strspn(p, delim);
			*p = '\0';
		}
		return start;
	}
	return NULL;
}
