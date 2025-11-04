#include "./hashtable.h"
#include <bits/stdc++.h>
using namespace std;

using T = int;
using H = int;

int main(){
    HashTable<T,H> ht;

    while(true){
        int x;
        cin>>x;


        if(x == 1){
            T key;
            H val;
            cin>>key>>val;

            ht.insert(key,val);
        }

        if(x == 2){
            T key;
            cin>>key;

            cout<<ht.find(key)<<endl;
        }

        if(x == 3){
            T key;
            cin>>key;

            ht.remove(key);
        }

        if(x == 4){
            ht.print();
        }
    }

    return 1;

}