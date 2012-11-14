// Last modified: 2012-10-26 14:48:35
 
/**
 * @file: lru.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-10-25 17:24:14
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _LRU_H_
#define _LRU_H_
 
#include "CacheFrame.h"

class CCacheFrame_LRU : public CCacheFrame
{
public:
	CCacheFrame_LRU(int _size) : CCacheFrame(_size) {}
	//~CCacheFrame_LRU();

	void CacheListInsert(
			unsigned int termid,
			unsigned int listLen,
			unsigned long long file_offset);
	void CacheListUpdate(cachenode_t *pcur);
	void CacheListEvict(int cur_len);
};
 
#endif
