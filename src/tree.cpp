#include "tree.h"
#include <iostream>
using namespace std;

CTree::CTree(){
    m_Head = nullptr;
}

CTree::~CTree(){
    Destroy(m_Head);
}

void CTree::Destroy(node_t *node){
    if(node == nullptr)
        return;

    pthread_mutex_t *mtx = &node->mtx;
    pthread_mutex_lock(mtx);

    Destroy(node->left);
    Destroy(node->right);

    delete node;

    pthread_mutex_unlock(mtx);
    pthread_mutex_destroy(mtx);
}

bool CTree::Add(int value){
    node_t *leaf = nullptr;
    node_t *found = InnerFind(value, &leaf);

    if(found != nullptr)
        return false;

    // Head is empty
    if(leaf == nullptr){
        m_Head = new node_t(value);
        pthread_mutex_init(&m_Head->mtx, nullptr);
        cout << endl;
        return true;
    }

    NodeLock(leaf);

    node_t *node = new node_t(value);

    pthread_mutex_init(&node->mtx, nullptr);

    if(value < leaf->value){
        leaf->left = node;
    }
    else {
        leaf->right = node;
    }

    NodeUnlock(leaf);

    return true;
}

bool CTree::Remove(int value){
    return RemoveNode(m_Head, value) != nullptr;
}

node_t *CTree::RemoveNode(node_t *node, int value){
    if(node == nullptr)
        return nullptr;

    NodeLock(node);

    int v = node->value;
    if(value < v)
        node->left = RemoveNode(node->left, value);
    else if(value > v)
        node->right = RemoveNode(node->right, value);
    else {
        if(node->left == nullptr){
            node_t *ret = node->right;

            pthread_mutex_t *mtx = &node->mtx;
            delete node;

            pthread_mutex_unlock(mtx);
            pthread_mutex_destroy(mtx);
            return ret;
        } else if(node->right == nullptr){
            node_t *ret = node->left;

            pthread_mutex_t *mtx = &node->mtx;
            delete node;

            pthread_mutex_unlock(mtx);
            pthread_mutex_destroy(mtx);
            return ret;
        }

        int newval;
        node_t *ret = MinMaxValueNode(node->right, false, &newval);

        node->value = newval;

        node->right = RemoveNode(node->right, ret->value);
    }

    NodeUnlock(node);
    return node;
}

bool CTree::Find(int value){
    node_t *dummy;
    return InnerFind(value, &dummy) != nullptr;
}

node_t *CTree::InnerFind(int value, node_t **parent){
    node_t *current = m_Head;
    *parent = nullptr;

    if(current == nullptr){
        return nullptr;
    }

    NodeLock(current);

    int v = current->value;

    if(v == value){
        NodeUnlock(current);
        return current;
    }

    node_t *prev = current;
    current = value < v ? current->left : current->right;
    if(current == nullptr){
        NodeUnlock(prev);
        *parent = prev;
        return nullptr;
    }

    NodeLock(current);

    node_t *next;
    while(current){
        v = current->value;

        if(v == value){
            NodeUnlock(current);
            NodeUnlock(prev);

            *parent = prev;
            return current;
        }

        next = value < v ? current->left : current->right;
        if(!next){
            break;
        }

        NodeLock(next);
        NodeUnlock(prev);

        prev = current;
        current = next;
    }

    NodeUnlock(current);
    NodeUnlock(prev);

    *parent = current;
    return nullptr;
}

node_t *CTree::MinMaxValueNode(node_t *node, bool max, int *value){
    if(node == nullptr)
        return nullptr;

    node_t *current = node;
    NodeLock(current);
    
    node_t *next;
    while(current){
        *value = current->value;

        next = max ? current->right : current->left;
        if(next == nullptr){
            NodeUnlock(current);
            return current;
        }

        NodeLock(next);
        NodeUnlock(current);

        current = next;
    }

    return current;
}

void CTree::PrintTree(){
    PrintNode(m_Head);
    std::cout << endl;
}

void CTree::PrintNode(node_t *node){
    if(node == nullptr)
        return;

    NodeLock(node);

    PrintNode(node->left);

    std::cout << node->value << " ";

    PrintNode(node->right);
        
    NodeUnlock(node);
}