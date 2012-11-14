// Last modified: 2012-10-17 20:27:07
 
/**
 * @file: testMM.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-28 18:23:34
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sys/time.h>

#include "hash.h"
#include "function.h"
#include "ListHandler.h"
#include "MemoryDict.h"
#include "MemoryManager.h"
#include "CacheFrame.h"

using namespace std;

FILE *pIndex;
MemoryDict dict;
hashnode_t *hashTable[MAX_HASH];
//hashhead_t *hashTable[MAX_HASH];
CCacheFrame *CF;
//MemoryManager *MM;
//pthread_mutex_t hashlock = PTHREAD_MUTEX_INITIALIZER;

void LoadDict()
{
	string strDictDir = "/home/fan/Index/ori/";
	string strDictName = "GOV2.Rand";
	bool bRet = dict.Load(strDictDir, strDictName);

	if(!bRet) {
		cout << "Error in reading the dict" << endl;
		return ;
	}
	string file =strDictDir + "/" + strDictName + ".index";
	pIndex = fopen(file.c_str(), "rb");
	assert(pIndex != NULL);
}

void traditional_read_access(unsigned int termid)
{
	struct timeval start_time, end_time;
	int listLength = dict.m_vecDict[termid].m_nFreq;
	int offset = dict.m_vecDict[termid].m_nOffset;
	int *space = (int *)malloc(listLength * sizeof(int));
	gettimeofday(&start_time, NULL);
	fseek(pIndex, offset, SEEK_SET);
	fread(space, sizeof(int), listLength, pIndex);
	gettimeofday(&end_time, NULL);
	cout << "read time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;

	int docid = 701;
	gettimeofday(&start_time, NULL);
	cout << "the " << docid << "-th element of list(" << termid << ") is "
		<< space[docid - 1] << endl;
	gettimeofday(&end_time, NULL);
	cout << "access time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;
	
	docid = 13501;
	gettimeofday(&start_time, NULL);
	cout << "the " << docid << "-th element of list(" << termid << ") is "
		<< space[docid - 1] << endl;
	gettimeofday(&end_time, NULL);
	cout << "access time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;

	docid = 135168;
	gettimeofday(&start_time, NULL);
	cout << "the " << docid << "-th element of list(" << termid << ") is "
		<< space[docid - 1] << endl;
	gettimeofday(&end_time, NULL);
	cout << "access time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;

	freeResource(space);
}

void cacheframe_read_access(unsigned int termid)
{
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	ListHandler *LH = new ListHandler(termid, dict.m_vecDict[termid]);
	gettimeofday(&end_time, NULL);
	cout << "ListHandler Initiating time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;
	
	int docid = 701;
	gettimeofday(&start_time, NULL);
	cout << "the " << docid << "-th element of list(" << termid << ") is " 
		 << LH->GetItem(docid) << endl;
	gettimeofday(&end_time, NULL);
	cout << "access time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;
	
	docid = 13501;
	gettimeofday(&start_time, NULL);
	cout << "the " << docid << "-th element of list(" << termid << ") is " 
		 << LH->GetItem(docid) << endl;
	gettimeofday(&end_time, NULL);
	cout << "access time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;

	docid = 135168;
	gettimeofday(&start_time, NULL);
	cout << "the " << docid << "-th element of list(" << termid << ") is " 
		 << LH->GetItem(docid) << endl;
	gettimeofday(&end_time, NULL);
	cout << "access time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;

	delete LH;
}

int main()
{
	init_hash();

	LoadDict();

#ifdef STAT
	for (int i = 0; i < 14000000; ++i)
	{
		cout << i << " " << dict.m_vecDict[i].m_nFreq << endl;
	}
	exit(0);
#endif
	
	//unsigned int termid = 331788;
	//unsigned int termid = 1710479;
	unsigned int termid = 0;
	cout << "length(" << termid << ") = " << dict.m_vecDict[termid].m_nFreq << endl;
	
	struct timeval start_time, end_time;
	cout << "------------------------------------------------" << endl;
	//traditional_read_access(termid);
	cout << "------------------------------------------------" << endl;

	cout << "CacheFrame Initiating..." << endl;
	gettimeofday(&start_time, NULL);
	CF = new CCacheFrame(MEMORYSIZE);
	gettimeofday(&end_time, NULL);
	cout << "CacheFrame Initialization Success..." << endl;
	cout << "Initiating time = " << (end_time.tv_sec - start_time.tv_sec) * 1000000
		+ end_time.tv_usec - start_time.tv_usec << "us" << endl;

	//cacheframe_read_access(termid);
	
	cout << "------------------------------------------------" << endl;

	//termid = 169643;
	termid = 1;
	int docid = 512;
	ListHandler *LH = new ListHandler(termid, dict.m_vecDict[termid]);
	cout << "length(" << termid << ") = " << LH->GetLength() << endl;
	cout << "the " << docid << "-th element of list(" << termid << ") is " 
		<< LH->GetItem(docid) << endl;
	docid = 1024;
	cout << "the " << docid << "-th element of list(" << termid << ") is " 
		<< LH->GetItem(docid) << endl;
	delete LH;
/*
	cout << "------------------------------------------------" << endl;

	termid = 3;
	ListHandler *LH3 = new ListHandler(termid, dict.m_vecDict[termid]);
	cout << "length(" << termid << ") = " << LH3->GetLength() << endl;
	delete LH3;
*/
	/*
	unsigned int num = 500000, sum = 0;
	ListHandler **LH = new ListHandler* [num];
	for (termid = 0; termid < num; ++termid)
	{
		LH[termid] = new ListHandler(termid, dict.m_vecDict[termid]);
		sum += CF->NumOfBlock(LH[termid]->GetLength());
		if (termid % 10000 == 0)
		{
			cout << "length(" << termid << ") = " << LH[termid]->GetLength() << endl;
			cout << sum << endl;
		}
		if (termid == 169643)
		{
			int docid = 1;
			cout << "the " << docid << "-th element of list(" << termid << ") is " 
				 << LH[termid]->GetItem(docid) << endl;

		}
	}
	*/
	//termid = 169643;
	termid = 1;
	docid = 512;
	//cout << "the " << docid << "-th element of list(" << termid << ") is " 
	//	 << LH[termid]->GetItem(docid) << endl;

	int listLength = dict.m_vecDict[termid].m_nFreq;
	int offset = dict.m_vecDict[termid].m_nOffset;
	int *space = (int *)malloc(listLength * sizeof(int));
	fseek(pIndex, offset, SEEK_SET);
	fread(space, sizeof(int), listLength, pIndex);
	cout << "the " << docid << "-th element of list(" << termid << ") is "
		<< space[docid - 1] << endl;
	docid = 1024;
	cout << "the " << docid << "-th element of list(" << termid << ") is "
		<< space[docid - 1] << endl;
	free(space);
	/*
	for (termid = 0; termid < num; ++termid)
	{
		delete LH[termid];
	}
	*/
	delete CF;
	free_hash();

	return 0;
}
