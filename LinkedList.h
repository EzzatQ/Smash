//
//  LL.hpp
//  Homework 2
//
//  Created by Ezzat Qupty on 25/11/2019.
//  Copyright © 2019 Ezzat Qupty. All rights reserved.
//

#ifndef LL_hpp
#define LL_hpp

#include <stdio.h>


template <class K,class D>
class LLnode{
	K* key;
	D* data;
	LLnode* next;
	LLnode* prev;
public:
	LLnode():key(nullptr), data(nullptr), next(nullptr), prev(nullptr){}
	
	LLnode(K& k, D& d = D(), LLnode* n = nullptr, LLnode* p = nullptr): next(n), prev(p){
		key = new K(k);
		data = new D(d);
		
	}
	LLnode(const LLnode& n){
		next = n.next;
		prev = n.prev;
		data = new D(*n.data);
		key = new K(*n.key);
	}
	LLnode& operator=(const LLnode& n){
		*this = LLnode(n);
		return &this;
	}
	~LLnode(){
		delete key;
		delete data;
	}
	void setKey(K& k){
		if(key) delete key;
		key = new D(k);
	}
	K* getKeyP(){
		return key;
	}
	K getKey(){
		return *key;
	}
	void setData(D& d){
		if(data) delete data;
		data = new D(d);
	}
	D* getDataP(){
		return data;
	}
	D getData(){
		return *data;
	}
	LLnode* getNext(){
		return next;
	}
	void setNext(LLnode* n){
		next = n;
	}
	LLnode* getPrev(){
		return prev;
	}
	void setPrev(LLnode* p){
		prev = p;
	}
};

template <class K, class D>
class List{
	LLnode<K, D>* head;
	LLnode<K, D>* tail;
	int nodeCount;
	
	LLnode<K, D>* findNode(K& k){
		if(nodeCount > 0){
			if(head->getKey() == k) return head;
			LLnode<K, D>* tmp = head->getNext();
			while(tmp){
				if(tmp->getKey() == k) return tmp;
				tmp = tmp->getNext();
			}
		}
		return nullptr;
	}
public:
	List(): head(nullptr), tail(nullptr), nodeCount(0){
		LLnode<K, D>* n = new LLnode<K, D>();
		head = n;
		tail = n;
	}
	~List(){
		if(nodeCount == 0){
			delete(head);
			return;
		} else {
			LLnode<K, D>* tmp1 = head;
			LLnode<K, D>* tmp2 = head->getNext();
			for(int i = 0; i < nodeCount; i++){
				delete(tmp1);
				tmp1 = tmp2;
				if(tmp2) tmp2 = tmp2->getNext();
			}
		}
	}
	LLnode<K, D>* getHead(){
		return head;
	}
	LLnode<K, D>* getTail(){
		return tail;
	}
	
	void insertHead(K& k, D& d){
		if(nodeCount == 0){
			delete(head);
			LLnode<K, D>* n = new LLnode<K, D>(k, d, nullptr, nullptr);
			head = n;
			tail = n;
			nodeCount++;
			return;
		}
		LLnode<K, D>* n = new LLnode<K, D>(k, d, head, nullptr);
		LLnode<K, D>* tmp = head;
		head = n;
		tmp->setPrev(n);
		nodeCount++;
	}
	
	void insertTail(K& k, D& d){
		if(nodeCount == 0){
			delete(head);
			LLnode<K, D>* n = new LLnode<K, D>(k, d, nullptr, nullptr);
			head = n;
			tail = n;
			nodeCount++;
			return;
		}
		LLnode<K, D>* n = new LLnode<K, D>(k, d, nullptr, tail);
		LLnode<K, D>* tmp = tail;
		tail = n;
		tmp->setNext(n);
		nodeCount++;
	}
	
	D* find(K& k){
		if(nodeCount > 0){
			if(head->getkey() == k) return head->getDataP();
			LLnode<K, D>* tmp = head->getNext();
			while(tmp){
				if(tmp->getkey() == k) return tmp->getDataP();
				tmp = tmp->getNext();
			}
			return nullptr;
		}
		return nullptr;
	}
	
	void remove(K& k){
		if(nodeCount > 0){
			LLnode<K, D>* tmp = findNode(k);
			if(tmp){
				LLnode<K, D>* n = tmp->getNext();
				LLnode<K, D>* p = tmp->getPrev();
				p->setNext(n);
				n->setPrev(p);
				delete tmp;
				nodeCount--;
				if(nodeCount == 0){
					LLnode<K, D>* n = new LLnode<K, D>();
					head = n;
					tail = n;
				}
			}// else throw something
		}
	}
	
	
	int getSize(){
		return nodeCount;
	}
	
	bool empty(){
		return nodeCount == 0;
	}
};


#endif /* LL_hpp */
