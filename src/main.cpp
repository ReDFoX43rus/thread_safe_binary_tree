#include <iostream>
#include "tree.h"

#define THREADS_N 4

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

int main(){
    // int i = 50;
    // while(i){
    //     tree.Add(i);

    //     if(i < 0)
    //         i += 10;
    //     i *= -1;
    // }

    // tree.PrintTree();

    // tree.Remove(-40);
    // tree.PrintTree();
    // tree.Remove(10);
    // tree.PrintTree();

    // for(int i = -50; i <= 50; i += 10){
    //     cout << i << " found: " << tree.Find(i) << endl;
    // }

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

    delete tree;

    return 0;
}