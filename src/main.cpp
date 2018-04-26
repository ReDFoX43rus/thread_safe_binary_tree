#include <iostream>
#include "tree.h"

using namespace std;

typedef struct {
    int from;
    int to;
    int step;
} add_info_t;

CTree *tree = new CTree();

void* adder(void* arg){
    add_info_t *info = (add_info_t*)arg;

    for(int i = info->from; i <= info->to; i += info->step)
        tree->Add(i);

    return nullptr;
}

void *remover(void *arg){
    add_info_t *info = (add_info_t*)arg;

    for(int i = info->from; i <= info->to; i += info->step)
        tree->Remove(i);

    return nullptr;
}

void* addRandoms(void* arg){
    add_info_t *info = (add_info_t*)arg;

    int action = rand()%2;

    for(int i = 0; i < info->to; i++){
        if(action)
            tree->Add(rand());
        else tree->Remove(rand());
    }

    return nullptr;
}

void handleAll(int from, int to, bool add){
    int n = to - from + 1;

    pthread_t *threads = new pthread_t[n];
    add_info_t *infos = new add_info_t[n];

    for(int i = 0; i < n; i++){
        infos[i].from = i + from;
        infos[i].to = i + from;
        infos[i].step = 1;

        pthread_create(&threads[i], NULL, add ? adder : remover, (void*)&infos[i]);
    }

    for(int i = 0; i < n; i++){
        pthread_join(threads[i], NULL);
    }


    bool all = true, any = false;
    for(int i = from; i <= to; i++){
        if(!tree->Find(i)){
            all = false;
        } else any = true;
    }

    cout << "Handle: all " << all << " any: " << any << endl;

    delete[] infos;
    delete[] threads;
}

int main(){
    handleAll(-50, 50, true);
    tree->PrintTree();
    handleAll(-50, 50, false);
    tree->PrintTree();

    int n = 100;

    pthread_t *threads = new pthread_t[n];
    add_info_t *infos = new add_info_t[n];

    for(int i = 0; i < n; i++){
        infos[i].to = 20;

        pthread_create(&threads[i], NULL, addRandoms, (void*)&infos[i]);
    }

    for(int i = 0; i < n; i++){
        pthread_join(threads[i], NULL);
    }

    delete[] infos;
    delete[] threads;

    delete tree;

    return 0;
}
