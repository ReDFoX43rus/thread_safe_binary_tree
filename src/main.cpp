#include <iostream>
#include "tree.h"

using namespace std;

int main(){
    CTree tree;

    int i = 50;
    while(i){
        tree.Add(i);

        if(i < 0)
            i += 10;
        i *= -1;
    }

    tree.PrintTree();

    tree.Remove(-40);
    tree.PrintTree();
    tree.Remove(10);
    tree.PrintTree();

    for(int i = -50; i <= 50; i += 10){
        cout << i << " found: " << tree.Find(i) << endl;
    }

    return 0;
}