#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "Stack.h"

node NewNode(LicenseInfoT data)
{
	node n;
	n = (node)malloc(sizeof(StackNode));
	n->data = data;
	n->prev = NULL;
	return n;
}

DynamicStack StackNew()
{
	DynamicStack s;
	s = (DynamicStack)malloc(sizeof(stack_t));
	s->first = NULL;
	return s;
}

void StackPush(DynamicStack s, LicenseInfoT data)
{
	node n = NewNode(data);
	n->prev = s->first;
	s->first = n;
}

StackNode StackPop(DynamicStack s)
{
	node aux;
	StackNode n;

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
	if (s->first != NULL)
		return FALSE;
	else
		return TRUE;
}

void StackDestroy(DynamicStack s)
{
	StackNode n;
	while (s->first != NULL)
	{
		n = StackPop(s);
	}
	free(s);
}