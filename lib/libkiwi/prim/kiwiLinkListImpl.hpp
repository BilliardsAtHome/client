// Implementation header
#ifndef LIBKIWI_PRIM_LINKLIST_IMPL_HPP
#define LIBKIWI_PRIM_LINKLIST_IMPL_HPP

// Declaration header
#ifndef LIBKIWI_PRIM_LINKLIST_H
#include <libkiwi/prim/kiwiLinkList.h>
#endif

namespace kiwi {

/**
 * @brief Inserts node at iterator
 *
 * @param iter Iterator at which to insert node
 * @param pNode Node to insert
 * @return Iterator to new node
 */
template <typename T>
TList<T>::Iterator TList<T>::Insert(Iterator iter, TListNode<T>* pNode) {
    K_ASSERT(pNode != NULL);

    TListNode<T>* next = iter.mpNode;
    TListNode<T>* prev = next->mpPrev;

    // prev <- pNode -> next
    pNode->mpNext = next;
    pNode->mpPrev = prev;
    // prev <-> pNode <-> next
    next->mpPrev = pNode;
    prev->mpNext = pNode;

    mSize++;

    return Iterator(pNode);
}

/**
 * @brief Erases node from list
 *
 * @param pNode Node to erase
 * @return Iterator to next node
 */
template <typename T> TList<T>::Iterator TList<T>::Erase(TListNode<T>* pNode) {
    K_ASSERT(pNode != NULL);

    TListNode<T>* next = pNode->mpNext;
    TListNode<T>* prev = pNode->mpPrev;

    // Remove connections to node
    next->mpPrev = prev;
    prev->mpNext = next;
    // Isolate node
    pNode->mpNext = NULL;
    pNode->mpPrev = NULL;
    // Free memory
    delete pNode;

    mSize--;

    return Iterator(next);
}

/**
 * @brief Erases range of nodes
 *
 * @param begin Beginning of range (inclusive)
 * @param end End of range (exclusive)
 * @return Iterator to end of range
 */
template <typename T>
TList<T>::Iterator TList<T>::Erase(Iterator begin, Iterator end) {
    TListNode<T>* pCurr = begin.mpNode;
    TListNode<T>* pEnd = end.mpNode;

    while (pCurr != pEnd) {
        // Preserve next node before erasing pointers
        TListNode<T>* pNext = pCurr->mpNext;
        // Erase current node
        Erase(pCurr);
        pCurr = pNext;
    }

    return Iterator(pEnd);
}

} // namespace kiwi

#endif