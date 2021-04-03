#pragma once

#include <stdlib.h>
#include <Windows.h>
#include "CommonDefs.h"

typedef struct NodeStruct
{
	LicenseInfoT data;
	struct NodeStruct* prev;
} StackNode;

typedef StackNode* node;

typedef struct stack_str
{
	node first;             //top node
}stack_t;

typedef stack_t* DynamicStack;

DynamicStack StackNew();
void StackPush(DynamicStack s, LicenseInfoT data);
StackNode StackPop(DynamicStack s);
void StackDestroy(DynamicStack s);
BOOL StackIsEmpty(DynamicStack s);