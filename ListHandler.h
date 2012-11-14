// Last modified: 2012-10-20 20:22:44
 
/**
 * @file: ListHandler.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-29 00:42:14
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _LISTHANDLER_H_
#define _LISTHANDLER_H_

#include "hash.h"
#include "MemoryDict.h"
#include "CacheFrame.h"

class ListHandler
{
private:
	unsigned int termid;
	unsigned int listLength;
	unsigned long long offset;
	hashnode_t *hnode;
public:
	ListHandler(unsigned _termid, DictItem _item);
	~ListHandler();
	
	int GetItem(unsigned int itemID);

	unsigned long long GetOffset();
	unsigned long long GetLength();
};
 
#endif
