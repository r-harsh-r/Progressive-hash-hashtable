#pragma once
#include<string>
#include<iostream>
using namespace std;
struct Node{
    std::string key;
    std::string value;
    Node*nex;
};

class HashTable{
    Node**bin;
    int N;
    int count_keys;

    Node**newBin;
    int trans_idx;
    int batchSize;
    
private:
    void resize();
    size_t getIdx(size_t);
    size_t Hash(const string&);
    void progressiveRehashing();
    
    public:
    HashTable();
    ~HashTable();

    void insert(string&,string&);
    string find(const string&);
    void remove(string&);
    
    void print();
};