//
//  bjStringtoStringMap.h
//  TestTB
//
//  Created by Terrin Eager on 12/21/12.
//
//

#ifndef __TestTB__bjStringtoStringMap__
#define __TestTB__bjStringtoStringMap__

#include <iostream>
#include "bjstring.h"
#include "LLRBTree.h"

class StringMapNode : public CRBNode<BJString>
{
public:
    StringMapNode();
    StringMapNode(BJString* pKey);
    ~StringMapNode();
    inline virtual BJ_COMPARE Compare(BJString* pKey);
    inline virtual void CopyNode(CRBNode* pSource);
    inline virtual void Init(){};
    inline virtual void Clear() {};


    BJString value;

};

class BJStringtoStringMap : public CLLRBTree<BJString, StringMapNode>
{
public:


};




#endif /* defined(__TestTB__bjStringtoStringMap__) */
