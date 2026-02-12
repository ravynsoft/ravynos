//
//  LLRBTree.cpp
//  TestTB
//
//  Created by Terrin Eager on 7/9/12.
//
//

#include "LLRBTree.h"



#include <stdio.h>
#import <stdlib.h>
#include <string.h>
#include <curses.h>

#include "bjtypes.h"

#include <time.h>

void test3();






////////////////////

////////////////
// test case
// Integrity checks

/*********************
BJ_BOOL isBST(CRBNode* pRecord, BJ_UINT64 min, BJ_UINT64 max);
BJ_BOOL is234(CRBNode* pRecord);
BJ_BOOL isBalanced(CLLRBTree* pCache);
BJ_BOOL isBalancedNode(CRBNode* pRecord, int black);

BJ_BOOL check(CLLRBTree* pCache)
{  // Is this tree a red-black tree?
    BJ_BOOL bBST = isBST(pCache->GetRoot(),pCache->minRecord(pCache->GetRoot())->nKey,pCache->maxRecord(pCache->GetRoot())->nKey);
    BJ_BOOL b234 = is234(pCache->GetRoot());
    BJ_BOOL bisBalanced = isBalanced(pCache);

    printf("Bst=%d,234=%d, Balanced=%d",bBST,b234,bisBalanced);

    return bBST && b234 && bisBalanced;
}


BJ_BOOL isBST(CRBNode* pRecord, BJ_UINT64 min, BJ_UINT64 max)
{  // Are all the values in the BST rooted at x between min and max,
    // and does the same property hold for both subtrees?
    if (pRecord == NULL) return 1;
    if ((pRecord->nKey > min) || (max > pRecord->nKey)) return 0;
    return isBST(pRecord->m_rbLeft, min, pRecord->nKey) && isBST(pRecord->m_rbRight, pRecord->nKey, max);
}
BJ_BOOL is234(CRBNode* pRecord)
{  // Does the tree have no red right links, and at most two (left)
    // red links in a row on any path?
    if (pRecord == NULL) return 1;
    if (IsRed(pRecord->m_rbRight)) return 0;
    if (IsRed(pRecord))
        if (IsRed(pRecord->m_rbLeft))
            if (IsRed(pRecord->m_rbLeft->m_rbLeft)) return 0;
    return is234(pRecord->m_rbLeft) && is234(pRecord->m_rbRight);
}

BJ_BOOL isBalanced(CLLRBTree* pCache)
{ // Do all paths from root to leaf have same number of black edges?
    int black = 0;     // number of black links on path from root to min
    CRBNode* pRecord = pCache->m_Root;
    while (pRecord != NULL)
    {
        if (!IsRed(pRecord)) black++;
        pRecord = pRecord->m_rbLeft;
    }
    return isBalancedNode(pCache->root, black);
}

BJ_BOOL isBalancedNode(CRBNode* pRecord, int black)
{ // Does every path from the root to a leaf have the given number
    // of black links?
    if      (pRecord == NULL && black == 0) return 1;
    else if (pRecord == NULL && black != 0) return 0;
    if (!IsRed(pRecord)) black--;
    return isBalancedNode(pRecord->m_rbLeft, black) && isBalancedNode(pRecord->m_rbRight, black);
}
****************/

/**
// sample code for testing
void CStringNode_test()
{
    CStringTree Cache;


    char DummyData[] = {'a','b','d','x'};
    BJ_UINT64 i = 0;
    CStringNode* pRecord;

    while (i++ < sizeof(DummyData))
    {

        pRecord = (CStringNode*)Cache.FindwithAddRecord(&i);
        if (pRecord)
            pRecord->m_Value[0] = DummyData[i];
    }

    i = 2;
    pRecord = (CStringNode*)Cache.Find(&i);


    test3();

}

void test3()
{
    //  float nSaveCPU =0;

    CStringTree Cache;

    CStringNode test;


    BJ_UINT64 i = 0;
    long starttime = clock();
    float elapsedtime = 0;
    CStringNode* pRecord;

    // nSaveCPU = getCPUtime();
    while (i++ < 1000000)
    {
        pRecord = (CStringNode*) Cache.FindwithAddRecord(&i);
        if (pRecord)
            memccpy(pRecord->m_Value, "test",4, 1);

        // snprintf(pRecord->m_Value,sizeof(pRecord->m_Value),"%llx",key.m_nKey);
    }
    elapsedtime = clock() - starttime;
    elapsedtime /= CLOCKS_PER_SEC;

    // elapsedtime = getCPUtime() - nSaveCPU;

    printf("Test elapsed time %f, check = %d\n",elapsedtime,0);




}

*****/
///////////////

