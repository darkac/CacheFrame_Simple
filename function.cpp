// Last modified: 2012-11-30 15:23:10
 
/**
 * @file: function.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-27 11:27:15
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */

#include <cstdio>
#include <cstdlib>

#include "function.h"

void checkPointer(void *p, int line)
{
	if (p == NULL)
	{
		printf("null pointer in line %d\n", line);
		exit(-1);
	}
}

void checkResource(void *p, const char *source_name)
{
	if (p == NULL)
	{
		printf("Failed to open `%s`\n", source_name);
		exit(-1);
	}
}

void freeResource(void *pointer)
{
	if (pointer)
	{
		free(pointer);
		pointer = 0;
	}
}

void closeResource(FILE *fHandler)
{
	if (fHandler)
		fclose(fHandler);
}

void testPoint(int line)
{
	printf("Arrive here, line %d\n", line);
}
