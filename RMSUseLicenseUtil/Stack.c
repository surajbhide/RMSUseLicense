#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "Stack.h"

node NewNode(LicenseInfoT data)
{
	node n;
	n = (node)malloc(sizeof(StackNode));
	if (n != NULL)
	{
		n->data = data;
		n->prev = NULL;
	}
	return n;
}

DynamicStack StackNew()
{
	DynamicStack s;

	s = (DynamicStack)calloc(1, sizeof(stack_t));
	if (s != NULL)
		s->first = NULL;
	return s;
}

void StackPush(DynamicStack s, LicenseInfoT data)
{
	if (s == NULL)
		return;

	node n = NewNode(data);
	n->prev = s->first;
	s->first = n;
}

StackNode StackPop(DynamicStack s)
{
	node aux;
	StackNode n = { {0, "", ""}, NULL };

	if (s == NULL)
		return n;

	if (s->first != NULL)
	{
		aux = s->first;
		s->first = s->first->prev;
		n = *aux;
		free(aux);
	}
	return n;
}

BOOL StackIsEmpty (DynamicStack s)
{
	if (s == NULL)
		return TRUE;

	if (s->first != NULL)
		return FALSE;
	else
		return TRUE;
}

void StackDestroy(DynamicStack s)
{
	if (s == NULL)
		return;
	StackNode n;
	while (s->first != NULL)
	{
		n = StackPop(s);
	}
	free(s);
	s = NULL;
}