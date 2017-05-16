#pragma once

#include<unordered_map>
#include<algorithm>
#include<iostream>
#include<stack>

using std::string;
using std::unordered_map;
using std::swap;
using std::cout;
using std::endl;
using std::stack;

template<class K, class V>
struct pNode{
  pNode(K k, V v):
    val(v),
    key(k),
    left(nullptr),
    right(nullptr),
    parent(nullptr){}

  ~pNode(){
    removePointers();
  }
  void removePointers(){
    if(left)
      left->parent = nullptr;
    if(right)
      right->parent = nullptr;
    if(parent)
      if(parent->left == this)
        parent->left = nullptr;
      else
        parent->right = nullptr;
    left = right = parent = nullptr;
  }
  V val;
  K key;
  pNode<K,V> *left, *right, *parent;

};

template<class K, class V>
struct pQueue{
  pQueue(){ head = nullptr;}
  ~pQueue(){
    for(auto p : key2Node)
      delete p.second;
  }
  pNode<K,V>* merge(pNode<K,V>* a, pNode<K,V>* b){
    if(!a){
      return b;
    }
    if(!b){
      return a;
    }
    if(b->val < a->val){
      swap(a,b);
    }
    swap(a->left, a->right);
    a->right = merge(b, a->right);
    a->right-> parent = a;
    return a;
  }
  void insert(K key, V val){
    pNode<K,V>* node = new pNode<K,V>(key,val);
    key2Node[key] = node;
    head = merge(head,node);
  }

  //Push either adds or decreases key
  void push(K key, V val){
    if(key2Node.find(key) == key2Node.end()){
      insert(key,val);
    }else if(val < key2Node[key]->val){
      change(key,val);
    }
  }

  void del(K key){
    if(key2Node.find(key) != key2Node.end()){
      pNode<K,V>* node = key2Node[key];
      pNode<K,V>* left = node->left;
      pNode<K,V>* right = node->right;
      if(node == head)
        head = nullptr;
      delete node;
      head = merge(head,merge(left,right));
      key2Node.erase(key);
    }
  }
  void change(K key, V val){
    del(key);
    insert(key, val);
  }
  std::pair<K,V> pop(){
    K ret = head->key;
    V v = head->val;
    del(ret);
    return std::make_pair(ret, v);
  }
  V top(){
    return head->val;
  }
  bool empty(){
    return head == nullptr;
  }
  void printStructure(pNode<K,V>* node, int tabLevel){
    for(int i = 0; i < tabLevel; i++)
      cout <<"\t";
    if(node){
      cout << node->key << ":" << node->val << endl;
      printStructure(node->left, tabLevel+1);
      printStructure(node->right, tabLevel+1);
    }else{
      cout << "*" << endl;
    }
  }
  pNode<K,V>* head;
  unordered_map<K, pNode<K,V>*> key2Node;
};
