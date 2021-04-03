#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "Stack.h"

#define BUFSIZE 4096

BOOL ValidatePath(char *inpath)
{
	static TCHAR path[MAX_PATH] = { 0 };
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	TCHAR  buffer[BUFSIZE] = TEXT("");

	// first get the full path
	if (GetFullPathName(inpath, BUFSIZE, buffer, NULL) == FALSE)
	{
		printf("Invalid path\n");
		return FALSE;
	}
	printf("Getfull path returned [%s]\n", buffer);

	// get parts that matter
	_splitpath_s(buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	sprintf_s(path, MAX_PATH, "%s%s", drive, dir);
	printf("drive [%s], dir [%s], file [%s], ext [%s]\n", drive, dir, fname, ext);
	// check if path exists. if not return false;
	DWORD dwAttrib = GetFileAttributes(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void splitpath (char *inpath)
{
	static TCHAR path[MAX_PATH] = { 0 };
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	// get parts that matter
	_splitpath_s(inpath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	printf("drive [%s], dir [%s], file [%s], ext [%s]\n", drive, dir, fname, ext);
}

void StackTest()
{
	//DynamicStack s = StackNew();//Reserving memory
	//StackPush(s, 1);  //add element
	//StackPush(s, 2);  //add element
	//StackPush(s, 3);  //add element
	//StackPush(s, 4);  //add element
	//while (StackIsEmpty(s) != TRUE)
	//{
	//	StackNode n = StackPop(s); //Remove element
	//	printf("popped [%d]\n", n.Data);
	//}
	//StackDestroy(s);     //Liberate all allocated memory
}

void sscanftest()
{
	char *test1 = "featureName=test this";
	char *test2 = "version=1.0";
	char *test3 = "server=localhost.test.com";
	char *test4 = "path=c:\\temp\\test.txt";
	char buf[1024];
	char buf1[1024];

	printf("%d\n", sscanf_s(test1, "%[^=]=%[^\n]", buf, (unsigned)_countof(buf), buf1, (unsigned)_countof(buf1)));
	printf("[%s] and [%s]\n", buf, buf1);
	printf("%d\n", sscanf_s(test2, "%[^=]=%[^\n]", buf, (unsigned)_countof(buf), buf1, (unsigned)_countof(buf1)));
	printf("[%s] and [%s]\n", buf, buf1);
	printf("%d\n", sscanf_s(test3, "%[^=]=%[^\n]", buf, (unsigned)_countof(buf), buf1, (unsigned)_countof(buf1)));
	printf("[%s] and [%s]\n", buf, buf1);
	printf("%d\n", sscanf_s(test4, "%[^=]=%[^\n]", buf, (unsigned)_countof(buf), buf1, (unsigned)_countof(buf1)));
	printf("[%s] and [%s]\n", buf, buf1);
}
main(int argc, char **argv)
{
	//if (argc < 2)
	//{
	//	printf("Error: input needed\n");
	//	return;
	//}
	//splitpath(argv[1]);
	//printf("Validate returned [%d]\n", ValidatePath(argv[1]));
	//StackTest();
	sscanftest();
}