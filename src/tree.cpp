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
    m_Head = nullptr;
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
        NodeUnlock(found);
        NodeUnlock(leaf);

        if(leaf == nullptr)
            pthread_mutex_unlock(&m_HeadAddMtx);

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
    node_t *parent;
    node_t *to_remove = InnerFind(value, &parent);

    // Not found
    if(to_remove == nullptr){
        // m_Head == nullptr
        if(parent == nullptr){
            pthread_mutex_unlock(&m_HeadAddMtx);
            return false;
        }

        NodeUnlock(parent);
        return false;
    }

    // to_remove == m_Head
    if(parent == nullptr){
        NodeUnlock(to_remove); // no need

        if(to_remove->left == nullptr && to_remove->right == nullptr){
            m_Head = nullptr;

            pthread_mutex_destroy(&to_remove->mtx);
            delete to_remove;

            pthread_mutex_unlock(&m_HeadAddMtx);
            return true;
        }
        else if(to_remove->left == nullptr || to_remove->right == nullptr){
            m_Head = to_remove->left == nullptr ? to_remove->right : to_remove->left;

            pthread_mutex_destroy(&to_remove->mtx);
            delete to_remove;

            pthread_mutex_unlock(&m_HeadAddMtx);
            return true;

        } else {
            NodeLock(to_remove);
            HandleRemovingWithBothChildren(to_remove);
            NodeUnlock(to_remove);

            pthread_mutex_unlock(&m_HeadAddMtx);
            return true;
        }
    }

    if(RemoveNode(parent, to_remove))
        return true;
    else if(to_remove->left != nullptr && to_remove->right != nullptr){
        HandleRemovingWithBothChildren(to_remove);

        NodeUnlock(to_remove);
        NodeUnlock(parent);
    }

    return true;
}

bool CTree::HandleRemovingWithBothChildren(node_t *node){
    node_t *closest_parent;
    node_t *closest_node = MinMaxValueNode(node->right, false, &closest_parent);

    node->value = closest_node->value;
    RemoveNode(closest_parent == nullptr ? node : closest_parent, closest_node);

    if(closest_parent == nullptr)
        NodeLock(node);

    return true;
}

bool CTree::RemoveNode(node_t *parent, node_t *to_remove){
    // parent and to_remove are locked
    if(to_remove->left == nullptr && to_remove->right == nullptr){
        NodeUnlock(to_remove);

        if(parent->left == to_remove)
            parent->left = nullptr;
        else if(parent->right == to_remove)
            parent->right = nullptr;

        pthread_mutex_destroy(&to_remove->mtx);
        delete to_remove;

        NodeUnlock(parent);
        return true;
    }
    else if(to_remove->left == nullptr && to_remove->right != nullptr){
        if(parent->left == to_remove)
            parent->left = to_remove->right;
        else if(parent->right == to_remove)
            parent->right = to_remove->right;

        NodeUnlock(to_remove);
        pthread_mutex_destroy(&to_remove->mtx);

        delete to_remove;

        NodeUnlock(parent);
        return true;
    }
    else if(to_remove->left != nullptr && to_remove->right == nullptr){
        if(parent->left == to_remove)
            parent->left = to_remove->left;
        else if(parent->right == to_remove)
            parent->right = to_remove->left;

        NodeUnlock(to_remove);
        pthread_mutex_destroy(&to_remove->mtx);

        delete to_remove;

        NodeUnlock(parent);
        return true;
    }

    return false;
}

bool CTree::Find(int value){
    node_t *parent;
    node_t *result = InnerFind(value, &parent);

    if(result == nullptr){
        if(parent == nullptr){
            pthread_mutex_unlock(&m_HeadAddMtx);
            return false;
        } else {
            NodeUnlock(parent);
            return false;
        }
    } else {
        NodeUnlock(result);
        NodeUnlock(parent);

        if(parent == nullptr)
            pthread_mutex_unlock(&m_HeadAddMtx);

        return true;
    }

    return result != nullptr;
}

node_t *CTree::InnerFind(int value, node_t **parent){
    *parent = nullptr;

    // cout << "InnerFind: " << value << endl;

    pthread_mutex_lock(&m_HeadAddMtx);

    if(m_Head == nullptr){
        // cout << "Head is empty" << endl;
        return nullptr;
    }

    node_t *current = m_Head;

    NodeLock(current);

    int v = current->value;

    if(v == value){
        // cout << "Head is what we need" << endl;
        // NodeUnlock(current);
        return current;
    }

    pthread_mutex_unlock(&m_HeadAddMtx);    

    node_t *prev = current;
    current = value < v ? current->left : current->right;
    if(current == nullptr){
        // cout << "There is only head in our tree" << endl;
        // cout << "Parent - head" << endl;

        // NodeUnlock(prev);
        *parent = prev;
        return nullptr;
    }

    NodeLock(current);

    node_t *next;
    while(current){
        v = current->value;

        // cout << "Checking value: " << v << endl;

        if(v == value){
            // NodeUnlock(current);
            // NodeUnlock(prev);
            
            // cout << "Found value, parent: " << prev << " result: " << current << endl;

            *parent = prev;
            return current;
        }

        next = value < v ? current->left : current->right;
        if(!next){
            // cout << "Breaking" << endl;
            break;
        }

        NodeLock(next);
        NodeUnlock(prev);

        prev = current;
        current = next;
    }

    // NodeUnlock(current);
    NodeUnlock(prev);

    // cout << "Nothing found, returning nullptr, parent: " << current << endl << endl;

    *parent = current;
    return nullptr;
}

node_t *CTree::MinMaxValueNode(node_t *node, bool max, node_t **parent){
    *parent = nullptr;
    if(node == nullptr)
        return nullptr;

    node_t *current = node;
    NodeLock(current);
    
    node_t *next = max ? current->right : current->left;
    if(next == nullptr){
        return current;
    }

    NodeLock(next);

    *parent = current;
    current = next;

    while(current){
        next = max ? current->right : current->left;

        if(next == nullptr){
            break;
        }

        NodeLock(next);
        NodeUnlock(*parent);

        *parent = current;
        current = next;
    }

    return current;
}

void CTree::PrintTree(){
    pthread_mutex_lock(&m_HeadAddMtx);
    PrintNode(m_Head);
    pthread_mutex_unlock(&m_HeadAddMtx);

    cout << endl;
}

void CTree::PrintNode(node_t *node){
    if(node == nullptr)
        return;

    NodeLock(node);

    PrintNode(node->left);

    cout << node->value << " ";

    PrintNode(node->right);
        
    NodeUnlock(node);
}