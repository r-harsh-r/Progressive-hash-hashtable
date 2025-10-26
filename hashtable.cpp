#include "hashtable.h"
#include <functional>

HashTable::HashTable(){
    N = 1024;
    bin = new Node*[N];
    for(int i = 0; i < N; ++i) bin[i] = nullptr;
    count_keys = 0;

    trans_idx = -1;
    batchSize = 128;
    newBin = nullptr;
}

HashTable::~HashTable(){
    // delete all nodes in each bucket
    for(int i = 0; i < N; ++i){
        Node* curr = bin[i];
        while(curr){
            Node* next = curr->nex;
            delete curr;
            curr = next;
        }
    }
    delete [] bin;
    if (trans_idx != -1) { // Check if we were resizing
        int newN = N * 2;
        for (int i = 0; i < newN; ++i) {
            Node* curr = newBin[i];
            while (curr) {
                Node* next = curr->nex;
                delete curr;
                curr = next;
            }
        }
        // Delete the new bin array
        delete[] newBin;
    }
}

size_t HashTable::Hash(const string&key){
    hash<string> h;
    return h(key);
}

size_t HashTable::getIdx(size_t hashed){
    return hashed%N;
}

void HashTable::insert(string&key,string&val){
    if(trans_idx == -1){
        int idx = getIdx(Hash(key));
    
        Node*node = new Node;
        node->key = key;
        node->value = val;
        node->nex = nullptr;
    
        // insert at head
        if(bin[idx] == nullptr){
            bin[idx] = node;
        }else{
            node->nex = bin[idx];
            bin[idx] = node;
        }
    
        count_keys++;
    
        // if(2*count_keys > N){
        //     resize();
        // }
    }else{
        // place in newBin only
        int idx = Hash(key)%(2*N);
    
        Node*node = new Node;
        node->key = key;
        node->value = val;
        node->nex = nullptr;
    
        // insert at head
        if(newBin[idx] == nullptr){
            newBin[idx] = node;
        }else{
            node->nex = newBin[idx];
            newBin[idx] = node;
        }
    
        count_keys++;

        progressiveRehashing();
    }
    // resize etc

    // cout<<"Inserted : { "<<key<<" "<<val<<" }"<<endl;
    
}

string HashTable::find(const string&key){
    if(trans_idx == -1){
        int idx = getIdx(Hash(key));
        // cout<<"\tfinding "<<key<<" in bin"<<" "<<idx<<endl;
        Node*curr = bin[idx];
    
        while(curr != nullptr){
            if(curr->key == key){
                return curr->value;
            }
            curr = curr->nex;
        }
    
        return "";
    }
    
    // transition mode
    int idx = getIdx(Hash(key));
    Node*curr = bin[idx];

    while(curr != nullptr){
        if(curr->key == key){
            return curr->value;
        }
        curr = curr->nex;
    }
    
    int newIdx = Hash(key)%(2*N);
    curr = newBin[newIdx];
    
    while(curr != nullptr){
        if(curr->key == key){
            return curr->value;
        }
        curr = curr->nex;
    }

    // do the moveBatch operation
    progressiveRehashing();

    return "";
    

}

// void HashTable::remove(string&key){
//     if(trans_idx == -1){
//         int idx = getIdx(Hash(key));
    
//         Node*curr = bin[idx];
// void func(){
    
//     //         // remove from head
//     //         while(curr != nullptr){
//     //             if(curr->key == key){
//     //                 bin[idx] = curr->nex;
//     //                 delete curr;
//     //                 curr = bin[idx];
//     //                 count_keys--;
//     //             }else{
//     //                 break;
//     //             }
//     //         }
        
//     //         while(curr != nullptr && curr->nex != nullptr){
//     //             Node*temp;
//     //             if(curr->nex->key == key){
//     //                 temp = curr->nex;
//     //                 curr->nex = curr->nex->nex;
//     //                 delete temp;
//     //                 count_keys--;
//     //             }
//     //             curr = curr->nex;
//     //         }
    
//     //         return;
//     //     }
//     //     // dop it in bin first then in newbin
//     //     int idx = getIdx(Hash(key));
    
//     //     Node*curr = bin[idx];
    
//     //     // remove from head
//     //     while(curr != nullptr){
//     //         if(curr->key == key){
//     //             bin[idx] = curr->nex;
//     //             delete curr;
//     //             curr = bin[idx];
//     //             count_keys--;
//     //         }else{
//     //             break;
//     //         }
//     //     }
    
//     //     while(curr != nullptr && curr->nex != nullptr){
//     //         Node*temp;
//     //         if(curr->nex->key == key){
//     //             temp = curr->nex;
//     //             curr->nex = curr->nex->nex;
//     //             delete temp;
//     //             count_keys--;
//     //         }
//     //         curr = curr->nex;
//     //     }
    
//     //     // do it in newBin
//     //     idx = Hash(key)%(2*N);
    
//     //     curr = newBin[idx];
    
//     //     // remove from head
//     //     while(curr != nullptr){
//     //         if(curr->key == key){
//     //             newBin[idx] = curr->nex;
//     //             delete curr;
//     //             curr = newBin[idx];
//     //             count_keys--;
//     //         }else{
//     //             break;
//     //         }
//     //     }
    
//     //     while(curr != nullptr && curr->nex != nullptr){
//     //         Node*temp;
//     //         if(curr->nex->key == key){
//     //             temp = curr->nex;
//     //             curr->nex = curr->nex->nex;
//     //             delete temp;
//     //             count_keys--;
//     //         }
//     //         curr = curr->nex;
//     //     }
    
//     //     progressiveRehashing();
        
//     // }
// }


// NOTE : TOOK HELP FROM GEMINI TO DEBUG MINOR LOGICAL BUG
void HashTable::remove(string& key) {
    // cout<<"gpt request to delete : "<<key<<endl;
    if (trans_idx == -1) {
        // cout<<"Finding in bin"<<endl;
        // --- NOT RESIZING ---
        // Only check bin
        int idx = getIdx(Hash(key));
        Node* curr = bin[idx];
        Node* prev = nullptr;
        while (curr != nullptr) {
            // cout<<"Curr is not a nullptr"<<endl;
            if (curr->key == key) {
                // Found the node to delete
                Node* toDelete = curr;
                if (prev == nullptr) { // It's the head node
                    bin[idx] = curr->nex;
                } else { // It's a node in the middle or end
                    prev->nex = curr->nex;
                }
                
                curr = curr->nex; // Move to the next node
                delete toDelete;  // Delete the found node
                count_keys--;
            } else {
                // Not a match, just advance pointers
                prev = curr;
                curr = curr->nex;
            }
        }
        return; // We are done

    } else {
        // cout<<"Finding in newbin mode"<<endl;
        // --- CURRENTLY RESIZING ---
        // Must check both bins

        // 1. Check old bin
        int idx = getIdx(Hash(key));
        Node* curr = bin[idx];
        Node* prev = nullptr;
        while (curr != nullptr) {
            if (curr->key == key) {
                Node* toDelete = curr;
                if (prev == nullptr) {
                    bin[idx] = curr->nex;
                } else {
                    prev->nex = curr->nex;
                }
                curr = curr->nex;
                delete toDelete;
                count_keys--;
            } else {
                prev = curr;
                curr = curr->nex;
            }
        }

        // 2. Check new bin
        size_t newIdx = Hash(key) % (2 * N); // Use the new size
        curr = newBin[newIdx];
        prev = nullptr;
        while (curr != nullptr) {
            if (curr->key == key) {
                Node* toDelete = curr;
                if (prev == nullptr) {
                    newBin[newIdx] = curr->nex;
                } else {
                    prev->nex = curr->nex;
                }
                curr = curr->nex;
                delete toDelete;
                count_keys--;
            } else {
                prev = curr;
                curr = curr->nex;
            }
        }

        // Only call this when resizing!
        progressiveRehashing();
    }
}

void HashTable::print(){
    for(int i = 0;i<N;i++){
        Node*curr = bin[i];
        cout<<i<<" : ";
        while(curr != nullptr){
            cout<<"{"<<curr->key<<" : "<<curr->value<<"}, ";
            curr = curr->nex;
        }
        cout<<endl;
    }

}

void HashTable::resize(){
    int newN = 2*N;

    newBin = new Node*[newN];
    for(int i = 0;i<newN;i++) newBin[i] = nullptr;
    trans_idx = 0;

    progressiveRehashing();

}

void HashTable::progressiveRehashing(){
    int newN = N*2;
    int remainingInBatch = batchSize;
    
    for(trans_idx;trans_idx < N;trans_idx++){
        while(bin[trans_idx] != nullptr){
            Node*curr = bin[trans_idx];

            // removed this node
            bin[trans_idx]= curr->nex;
            curr->nex = nullptr;

            // moving this node to newBin
            int newIdx = Hash(curr->key)%(2*N);            
            curr->nex = newBin[newIdx];
            newBin[newIdx] = curr;
            
            remainingInBatch--;
            
            if(remainingInBatch <= 0) break;
        }
        if(remainingInBatch <= 0) break;
    }


    // progressive rehashing completed
    if(trans_idx >= N){
        delete[] bin;
        bin = newBin;
        newBin = nullptr;
        trans_idx = -1;
        N = newN;
    }

}
