#include "tree.h"
#include <iostream>
using namespace std;

CTree::CTree(){
    m_Head = nullptr;
    pthread_mutex_init(&m_HeadAddMtx, nullptr);
}

CTree::~CTree(){
    Destroy(m_Head);
    pthread_mutex_destroy(&m_HeadAddMtx);
}

void CTree::Destroy(node_t *node){
    if(node == nullptr)
        return;

    node_t *left;
    node_t *right;
    if(node == m_Head){
        pthread_mutex_lock(&m_HeadAddMtx);

        left = node->left;
        right = node->right;

        pthread_mutex_destroy(&node->mtx);
        delete node;

        Destroy(left);
        Destroy(right);

        pthread_mutex_unlock(&m_HeadAddMtx);
        return;
    }

    left = node->left;
    right = node->right;

    pthread_mutex_destroy(&node->mtx);
    delete node;

    Destroy(left);
    Destroy(right);
}

bool CTree::Add(int value){
    node_t *leaf = nullptr;
    node_t *found = InnerFind(value, &leaf);

    // cout << "Add: " << value << " found: " << found << " leaf: " << leaf << endl;

    if(found != nullptr){
        NodeUnlock(leaf);
        NodeUnlock(found);
        return false;
    }

    // Head is empty
    if(leaf == nullptr){
        m_Head = new node_t(value);
        pthread_mutex_init(&m_Head->mtx, nullptr);

        pthread_mutex_unlock(&m_HeadAddMtx);
        return true;
    }

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
    pthread_mutex_lock(&m_HeadAddMtx);
    node_t *result = RemoveNode(m_Head, value);
    pthread_mutex_unlock(&m_HeadAddMtx);

    return result != nullptr;
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
    node_t *result = InnerFind(value, &dummy);

    NodeUnlock(dummy);
    NodeUnlock(result);

    if(dummy == nullptr && result == nullptr)
        pthread_mutex_unlock(&m_HeadAddMtx);

    return result != nullptr;
}

node_t *CTree::InnerFind(int value, node_t **parent){
    *parent = nullptr;

    cout << "InnerFind: " << value << endl;

    pthread_mutex_lock(&m_HeadAddMtx);

    if(m_Head == nullptr){
        cout << "Head is empty" << endl;
        return nullptr;
    }

    node_t *current = m_Head;

    NodeLock(current);

    pthread_mutex_unlock(&m_HeadAddMtx);


    int v = current->value;

    if(v == value){
        cout << "Head is what we need" << endl;
        // NodeUnlock(current);
        return current;
    }

    node_t *prev = current;
    current = value < v ? current->left : current->right;
    if(current == nullptr){
        cout << "There is only head in our tree" << endl;
        cout << "Parent - head" << endl;

        // NodeUnlock(prev);
        *parent = prev;
        return nullptr;
    }

    NodeLock(current);

    node_t *next;
    while(current){
        v = current->value;

        cout << "Checking value: " << v << endl;

        if(v == value){
            // NodeUnlock(current);
            // NodeUnlock(prev);
            
            cout << "Found value, parent: " << prev << " result: " << current << endl;

            *parent = prev;
            return current;
        }

        next = value < v ? current->left : current->right;
        if(!next){
            cout << "Breaking" << endl;
            break;
        }

        NodeLock(next);
        NodeUnlock(prev);

        prev = current;
        current = next;
    }

    // NodeUnlock(current);
    NodeUnlock(prev);

    cout << "Nothing found, returning nullptr, parent: " << current << endl << endl;

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