//
//  LLRBTree.h
//  TestTB
//
//  Created by Terrin Eager on 7/9/12.
//
//  based on rbtree.h but converted to C++
//  ll from http://www.cs.princeton.edu/~rs/talks/LLRB/RedBlack.pdf

#ifndef __TestTB__LLRBTree__
#define __TestTB__LLRBTree__

#include <iostream>
#include "bjtypes.h"
#include <sys/socket.h>
#include "bjstring.h"
#include "bjIPAddr.h"

template <class KeyType>
class CRBNode
{
public:
    CRBNode() {m_bIsRed = true; m_rbLeft = m_rbRight = NULL;};
    virtual ~CRBNode();

    inline virtual BJ_COMPARE Compare(KeyType* pKey) = 0; // Key values are equal

    inline virtual void CopyNode(CRBNode* pSource) = 0;
    inline virtual void Init()=0;
    inline virtual void Clear()=0;

    CRBNode* GetMinNode();
    CRBNode* GetMaxNode();


    inline CRBNode* RotateNodeLeft();
    inline CRBNode* RotateNodeRight();


    CRBNode* AddRecord(CRBNode* pNewRecord);

    void FlipColor();

    BJ_UINT64 GetCount();

    CRBNode* Fixup();

    CRBNode* MoveRedLeft();
    CRBNode* MoveRedRight();

    void  CallBack(int(*callback)(const void*, const void*),void* pParam);


    bool  m_bIsRed;

//protected:
    KeyType  m_Key;
    CRBNode* m_rbLeft;
    CRBNode* m_rbRight;


};

template <class KeyType, class NodeType>
class CLLRBTree
{

public:
    CLLRBTree();
    virtual ~CLLRBTree() { if (m_Root) delete m_Root;};

    NodeType* Find(KeyType* pKey);

    NodeType* FindwithAddRecord(KeyType* pKey);
    NodeType* AddRecord(KeyType* pKey);
    void RemoveRecord(KeyType* pKey);

    NodeType* GetRoot() { return m_Root;};
    void ClearAll() { delete m_Root; m_Root = NULL;};

    BJ_UINT64 GetCount();


    NodeType* GetMinNode();
    NodeType* GetMaxNode();

    NodeType* deleteMin(NodeType* pRecord);


private:
    NodeType* RemoveRecord(NodeType* pRecord,KeyType* pKey);

    virtual NodeType* newNode(KeyType* pKey) { return new NodeType(pKey);}
    virtual void freeNode(NodeType * pNode){ delete pNode;};


    NodeType* m_Root;


};
/////////////////


template<class KeyType>
CRBNode<KeyType>::~CRBNode()
{
    if (m_rbLeft)
        delete m_rbLeft;
    if (m_rbRight)
        delete m_rbRight;

}




template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::RotateNodeLeft()
{
    CRBNode<KeyType>* pTemp = m_rbRight;
    m_rbRight = pTemp->m_rbLeft;
    pTemp->m_rbLeft = this;
    pTemp->m_bIsRed = pTemp->m_rbLeft->m_bIsRed;
    pTemp->m_rbLeft->m_bIsRed = 1;

    return pTemp;
}


template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::RotateNodeRight()
{
    CRBNode<KeyType>* pTemp = m_rbLeft;
    m_rbLeft = pTemp->m_rbRight;
    pTemp->m_rbRight = this;
    pTemp->m_bIsRed = pTemp->m_rbRight->m_bIsRed;
    pTemp->m_rbRight->m_bIsRed = 1;

    return pTemp;
}

template<class KeyType>
BJ_UINT64 CRBNode<KeyType>::GetCount()
{
    BJ_UINT64 Num = 1;
    if (m_rbLeft)
        Num += m_rbLeft->GetCount();
    if (m_rbRight)
        Num += m_rbRight->GetCount();

    return Num;

}

template<class KeyType>
void CRBNode<KeyType>::FlipColor()
{
    m_bIsRed = !m_bIsRed;
    if (m_rbLeft)
        m_rbLeft->m_bIsRed = !m_rbLeft->m_bIsRed;
    if (m_rbRight)
        m_rbRight->m_bIsRed = !m_rbRight->m_bIsRed;

}

template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::Fixup()
{
    // fix the tree balance
    CRBNode<KeyType>* pNode = this;

    if (m_rbRight && m_rbRight->m_bIsRed) // fix right leaning reds on the way up
        pNode = RotateNodeLeft();

    if (pNode && pNode->m_rbLeft && pNode->m_rbLeft->m_bIsRed && pNode->m_rbLeft->m_rbLeft && pNode->m_rbLeft->m_rbLeft->m_bIsRed) // fix two reds in a row on the way up
        pNode = RotateNodeRight();

    if (pNode && pNode->m_rbRight && pNode->m_rbRight->m_bIsRed && pNode->m_rbLeft && pNode->m_rbLeft->m_bIsRed) //split 4-nodes on the way up
        pNode->FlipColor();

    return pNode;
}

template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::MoveRedLeft()
{
    CRBNode* pNode = this;
    FlipColor();

    if (m_rbRight && m_rbRight->m_rbLeft && m_rbRight->m_rbLeft->m_bIsRed)
    {
        m_rbRight = m_rbRight->RotateNodeRight();
        pNode = RotateNodeLeft();
        if (pNode)
            pNode->FlipColor();
    }
    return pNode;
}

template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::MoveRedRight()
{
    CRBNode* pNode = this;
    FlipColor();

    if (m_rbLeft && m_rbLeft->m_rbLeft && m_rbLeft->m_rbLeft->m_bIsRed)
    {
        pNode = RotateNodeRight();
        if (pNode)
            pNode->FlipColor();
    }

    return pNode;
}
template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::AddRecord(CRBNode* pNewRecord)
{

    switch (Compare(&pNewRecord->m_Key))
    {
        case BJ_GT:
            if (m_rbRight)
                m_rbRight = m_rbRight->AddRecord(pNewRecord);
            else
                m_rbRight = pNewRecord;

            break;
        case BJ_LT:
            if (m_rbLeft)
                m_rbLeft = m_rbLeft->AddRecord(pNewRecord);
            else
                m_rbLeft = pNewRecord;

            break;
        default: // equal
            pNewRecord->m_bIsRed = false;
            pNewRecord->m_rbLeft = m_rbLeft;
            m_rbLeft = pNewRecord;
            return this;
    };

    // fix the tree balance
    CRBNode* pRecord = this;

    // fix the tree balance

    if (pRecord && pRecord->m_rbRight && pRecord->m_rbRight->m_bIsRed) // fix right leaning reds on the way up
        pRecord = pRecord->RotateNodeLeft();

    if (pRecord && pRecord->m_rbLeft && pRecord->m_rbLeft->m_bIsRed && pRecord->m_rbLeft->m_rbLeft && pRecord->m_rbLeft->m_rbLeft->m_bIsRed) // fix two reds in a row on the way up
        pRecord = pRecord->RotateNodeRight();

    if (pRecord && pRecord->m_rbRight && pRecord->m_rbRight->m_bIsRed && pRecord->m_rbLeft && pRecord->m_rbLeft->m_bIsRed) //split 4-nodes on the way up
        pRecord->FlipColor();


    return pRecord;
}



template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::GetMinNode()
{
    CRBNode* pRecord = this;
    while (pRecord && pRecord->m_rbLeft)
        pRecord = pRecord->m_rbLeft;

    return pRecord;
}
template<class KeyType>
CRBNode<KeyType>* CRBNode<KeyType>::GetMaxNode()
{
    CRBNode* pRecord = this;
    while (pRecord && pRecord->m_rbRight)
        pRecord = pRecord->m_rbRight;

    return pRecord;
}

template<class KeyType>
void  CRBNode<KeyType>::CallBack(int(*callback)(const void*, const void*),void* pParam)
{

    if (m_rbLeft)
        m_rbLeft->CallBack(callback,pParam);

    callback(this,pParam);

    if (m_rbRight)
        m_rbRight->CallBack(callback,pParam);


}



///////////
template<class KeyType,class NodeType>
CLLRBTree<KeyType,NodeType>::CLLRBTree()
{
    m_Root = NULL;
}

template<class KeyType,class NodeType>
NodeType* CLLRBTree<KeyType,NodeType>::Find(KeyType* pKey)
{

    CRBNode<KeyType>* pNode = m_Root;

    while (pNode)
    {
        switch (pNode->Compare(pKey))
        {
            case BJ_GT:
                pNode = pNode->m_rbRight;
                break;
            case BJ_LT:
                pNode = pNode->m_rbLeft;
                break;
            default:
                return (NodeType*)pNode;
                break;
        }
    }

    return NULL;

}

template<class KeyType,class NodeType>
NodeType* CLLRBTree<KeyType,NodeType>::AddRecord(KeyType* pKey)
{
    NodeType* pRecord = newNode(pKey);
    if (m_Root)
        m_Root = (NodeType*) m_Root->AddRecord(pRecord);
    else
        m_Root = pRecord;

    if (m_Root)
        m_Root->m_bIsRed = false;

    return pRecord;
}

template<class KeyType,class NodeType>
NodeType* CLLRBTree<KeyType,NodeType>::FindwithAddRecord(KeyType* pKey)
{
    NodeType* pRecord = NULL;

    pRecord = Find(pKey);

    if (pRecord == NULL)
        pRecord = AddRecord(pKey);

    return pRecord;
}

template<class KeyType,class NodeType>
void CLLRBTree<KeyType,NodeType>::RemoveRecord(KeyType* pKey)
{
    m_Root = RemoveRecord(m_Root,pKey);
}

template<class KeyType,class NodeType>
NodeType* CLLRBTree<KeyType,NodeType>::deleteMin(NodeType* pRecord)
{
    if (pRecord->m_rbLeft == NULL)
    {
        freeNode(pRecord);
        return NULL;
    }

    if (!(pRecord->m_rbLeft && pRecord->m_rbLeft->m_bIsRed)  && !(pRecord->m_rbLeft &&  pRecord->m_rbLeft->m_rbLeft &&pRecord->m_rbLeft->m_rbLeft->m_bIsRed))
        pRecord = (NodeType*)pRecord->MoveRedLeft();

    pRecord->m_rbLeft = deleteMin((NodeType*)pRecord->m_rbLeft);

    return (NodeType*)pRecord->Fixup();
}

template<class KeyType,class NodeType>
NodeType* CLLRBTree<KeyType,NodeType>::RemoveRecord(NodeType* pRecord,KeyType* pKey)
{
    NodeType* pTempRecord = NULL;

    if (pRecord == NULL)
        return NULL;

    if (pRecord->Compare(pKey) == BJ_LT)
    {
        if (!(pRecord->m_rbLeft &&pRecord->m_rbLeft->m_bIsRed)  && !(pRecord->m_rbLeft &&  pRecord->m_rbLeft->m_rbLeft &&pRecord->m_rbLeft->m_rbLeft->m_bIsRed))
            pRecord->MoveRedLeft();
        pRecord = RemoveRecord((NodeType*)pRecord->m_rbLeft, pKey);
    }
    else
    {
        if (pRecord->m_rbLeft &&pRecord->m_rbLeft->m_bIsRed)
            pRecord->RotateNodeRight();

        if(pRecord->Compare(pKey) == BJ_EQUAL && pRecord->m_rbRight == NULL)
        {
            freeNode(pRecord);
            return NULL;
        }

        if (!(pRecord->m_rbRight && pRecord->m_rbRight->m_bIsRed) && !(pRecord->m_rbRight && pRecord->m_rbRight->m_rbLeft && pRecord->m_rbRight->m_rbLeft->m_bIsRed))
            pRecord = (NodeType*)pRecord->MoveRedRight();

        if (pRecord->Compare(pKey) == BJ_EQUAL)
        {
            pTempRecord = (NodeType*)pRecord->GetMinNode();
            pRecord->CopyNode(pTempRecord);
            pRecord->m_rbRight = deleteMin((NodeType*)pRecord->m_rbRight);
        }
        else
        {
            pRecord->m_rbRight = RemoveRecord((NodeType*)pRecord->m_rbRight, pKey);
        }
    }
    return pRecord?(NodeType*)pRecord->Fixup():NULL;
}




template<class KeyType,class NodeType>
BJ_UINT64 CLLRBTree<KeyType,NodeType>::GetCount()
{
    if (m_Root)
        return m_Root->GetCount();
    else
        return 0;
}

template<class KeyType,class NodeType>
NodeType* CLLRBTree<KeyType,NodeType>::GetMinNode()
{
    if (m_Root)
        return m_Root->GetMinNode();
    else
        return NULL;

}

template<class KeyType,class NodeType>
NodeType* CLLRBTree<KeyType,NodeType>::GetMaxNode()
{
    if (m_Root)
        return m_Root->GetMaxNode();
    else
        return NULL;

}





#endif /* defined(__TestTB__LLRBTree__) */
