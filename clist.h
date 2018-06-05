#ifndef _C_LIST_
#define _C_LIST_
#include <stddef.h>

typedef struct clist {
	struct clist *prox;
	int type;
	char comando[];
} *CommandList;

void addToList (CommandList list, CommandList x){
	while (list->prox != NULL)
		list = list->prox;

	list->prox = x;
}

#endif
