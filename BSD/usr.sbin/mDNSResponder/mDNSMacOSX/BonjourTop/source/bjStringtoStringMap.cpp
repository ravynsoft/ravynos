//
//  bjStringtoStringMap.cpp
//  TestTB
//
//  Created by Terrin Eager on 12/21/12.
//
//

#include "bjStringtoStringMap.h"

/////////////////////

StringMapNode::StringMapNode()
{


}

StringMapNode::StringMapNode(BJString* pKey)
{
    m_Key = *pKey;
}

StringMapNode::~StringMapNode()
{

}

void StringMapNode::CopyNode(CRBNode* pSource)
{
    m_Key = ((StringMapNode*)pSource)->m_Key;
}

BJ_COMPARE StringMapNode::Compare(BJString* pKey)
{

    return m_Key.Compare(*pKey);

}

