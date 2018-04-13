#pragma once
#include <pthread.h>

typedef struct node{
    int value;
    struct node* left;
    struct node* right;

    pthread_mutex_t mtx;

    node(int val){
        value = val;
        left = nullptr;
        right = nullptr;
    }

} node_t;

class CTree{
public:
    CTree();
    ~CTree();

    bool Add(int value);
    bool Remove(int value);
    bool Find(int value);

    void PrintTree();
private:
    node_t *m_Head;
    pthread_mutex_t m_HeadAddMtx;

    void Destroy(node_t *node);
    node_t *InnerFind(int value, node_t **parent);
    void PrintNode(node_t *node);
    bool RemoveNode(node_t *parent, node_t *to_remove);
    bool HandleRemovingWithBothChildren(node_t *node);

    static node_t *MinMaxValueNode(node_t *node, bool max, node_t **parent);
    static void NodeLock(node_t *node) { if(node != nullptr) pthread_mutex_lock(&node->mtx); }
    static void NodeUnlock(node_t *node) { if(node != nullptr) pthread_mutex_unlock(&node->mtx); }
};