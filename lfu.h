// Last modified: 2012-11-22 17:25:32
 
/**
 * @file: lfu.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-11-20 13:57:18
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _LFU_H_
#define _LFU_H_
 
#include "CacheFrame.h"

class CCacheFrame_LFU : public CCacheFrame
{
private:
	cachenode_t *_1Ref_InsertPoint;
	// the leftmost node whose reference is 1
public:
	CCacheFrame_LFU(int _size);
	
	void CacheListInsert(
			unsigned int termid,
			unsigned int listLen,
			unsigned long long file_offset,
			ItemType type);
	void CacheListUpdate(cachenode_t *pcur);
	void CacheListEvict(int cur_len);
};
 
#endif
