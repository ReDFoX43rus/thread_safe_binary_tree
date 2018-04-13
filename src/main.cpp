#include <iostream>
#include "tree.h"

#define THREADS_N 4
#define REMOVE_VALUES_N 15

using namespace std;

typedef struct {
    int from;
    int to;
    int step;
} add_info_t;

typedef struct {
    int values[REMOVE_VALUES_N];
} remove_info_t;

CTree *tree = new CTree();

void* adder(void* arg){
    add_info_t *info = (add_info_t*)arg;

    for(int i = info->from; i <= info->to; i += info->step)
        tree->Add(i);

    return nullptr;
}

void *remover(void *arg){
    remove_info_t *info = (remove_info_t*)arg;

    for(int i = 0; i < REMOVE_VALUES_N; i++){
        tree->Remove(info->values[i]);
    }

    return nullptr;
}

int main(){
    int i = 50;
    while(i){
        tree->Add(i);

        if(i < 0)
            i += 10;
        i *= -1;
    }

    tree->PrintTree();

    tree->Remove(-40);
    tree->PrintTree();
    tree->Remove(10);
    tree->PrintTree();

    for(int i = -50; i <= 50; i += 10){
        cout << i << " found: " << tree->Find(i) << endl;
    }

    pthread_t threads[THREADS_N];
    add_info_t infos[THREADS_N];
    for(int i = 0; i < THREADS_N; i++){
        infos[i].from = i*10;
        infos[i].to = 2*i*10;
        infos[i].step = 1;

        pthread_create(&threads[i], NULL, adder, (void*)&infos[i]);
    }

    for(int i = 0; i < THREADS_N; i++)
        pthread_join(threads[i], NULL);

    for(int i = 0; i < THREADS_N; i++){
        infos[i].from = -100 + i*10;
        infos[i].to = -100 + (i+1)*10;
        infos[i].step = 2;

        pthread_create(&threads[i], NULL, adder, (void*)&infos[i]);
    }

    for(int i = 0; i < THREADS_N; i++)
        pthread_join(threads[i], NULL);

    tree->PrintTree();

    remove_info_t rm_infos[THREADS_N];
    for(int i = 0; i < THREADS_N; i++){
        for(int j = 0; j < REMOVE_VALUES_N; j++)
            rm_infos[i].values[j] = (i+1)*10 + j;

        pthread_create(&threads[i], NULL, remover, (void*)&rm_infos[i]);
    }

    for(int i = 0; i < THREADS_N; i++)
        pthread_join(threads[i], NULL);

    tree->PrintTree();

    delete tree;

    return 0;
}