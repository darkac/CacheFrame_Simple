// Last modified: 2012-12-03 03:11:55
 
/**
 * @file: qtfdf.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-11-30 10:56:47
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _QTFDF_H_
#define _QTFDF_H_

#include <map>
#include <unordered_map>
#include <queue>
#include <cstring>
#include "CacheFrame.h"

struct QtfDf
{
public:
	unsigned int termid;
	double value; // qtfdf value
	
	QtfDf(const QtfDf& rq)
	{
		termid = rq.termid;
		value = rq.value;
	}

	QtfDf(int ntid, double nvalue)
	{
		termid = ntid;
		value = nvalue;
	}

	QtfDf& operator=(const QtfDf& rq)
	{
		termid = rq.termid;
		value = rq.value;
		return *this;
	}
};

/*
bool operator < (const QtfDf &qt1, const QtfDf &qt2)
{
	return qt1.value > qt2.value;
}
*/
struct QtfDf_Smaller {
	bool operator()(const QtfDf *qt1, const QtfDf *qt2)
	{
		return qt1->value > qt2->value;
	}
};

class CCacheFrame_QTFDF : public CCacheFrame
{
private:
	FILE *fSTAT;
	string stat_file;
	
	unordered_map<unsigned int, int> mapFre;
	unordered_map<unsigned int, int> mapLen;
	//priority_queue<QtfDf, vector<QtfDf>, less<QtfDf> > qtQue;
	priority_queue<QtfDf*, vector<QtfDf*>, QtfDf_Smaller > qtQue;

public:
	CCacheFrame_QTFDF(int _size, const char *stat_file);
	~CCacheFrame_QTFDF();

	void CacheListInsert(
			unsigned int termid,
			unsigned int listLen,
			unsigned long long file_offset,
			ItemType type);
	void CacheListUpdate(cachenode_t *pcur);
	void CacheListEvict(int cur_len);
	
	const QtfDf* qtQue_top()			{ return qtQue.top(); }
	void qtQue_push(QtfDf* newq)	{ qtQue.push(newq); }
	void qtQue_pop()					{ qtQue.pop(); }
	bool qtQue_empty()					{ return qtQue.empty(); }
};

#endif
