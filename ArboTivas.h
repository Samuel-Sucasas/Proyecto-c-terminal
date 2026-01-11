//Librerias utilizadas
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <utility>
using namespace std;
//primitivas de arboles
//Sistema de archivos  
struct FSNode {
	string name;
	bool isDir;
	string content; 
	FSNode* parent;
	FSNode** children;
	size_t children_count;
	size_t children_capacity;
	FSNode(const string &n, bool dir, FSNode* p=nullptr): name(n), isDir(dir), content(), parent(p), children(nullptr), children_count(0), children_capacity(0) {}
};

// Arreglo dinámico de cadenas (puro puntero)
struct ArregloCadenas {
	string* items;
	size_t count;
	size_t capacity;
	ArregloCadenas(): items(nullptr), count(0), capacity(0) {}
};

static void agregarCadena(ArregloCadenas &arr, const string &s){
	if(arr.capacity==0){ arr.capacity = 4; arr.items = new string[arr.capacity]; }
	else if(arr.count==arr.capacity){ size_t nc = arr.capacity * 2; string* nitems = new string[nc]; for(size_t i=0;i<arr.count;++i) nitems[i]=move(arr.items[i]); delete [] arr.items; arr.items = nitems; arr.capacity = nc; }
	arr.items[arr.count++] = s;
}

static void liberarArregloCadenas(ArregloCadenas &arr){ 
	if(arr.items) delete [] arr.items; arr.items = nullptr; arr.count = arr.capacity = 0; 
}

static void invertirArregloCadenas(ArregloCadenas &arr){ 
	for(size_t i=0;i<arr.count/2;++i) swap(arr.items[i], arr.items[arr.count-1-i]); 
}

static ArregloCadenas dividirRuta(const string &path){
	ArregloCadenas partes;
	string tmp;
	for(char c: path){
		if(c=='/'){
			if(!tmp.empty()){ 
                agregarCadena(partes, tmp); tmp.clear();
            }
		} else tmp.push_back(c);
	}
	if(!tmp.empty()) agregarCadena(partes, tmp);
	return partes;
}

//Union de Rutas 
static string unirRuta(const ArregloCadenas &partes){
	if(partes.count==0) return "/";
	string s;
	for(size_t i=0;i<partes.count;++i){
         s += "/" + partes.items[i];
    }
	return s.empty()?"/":s;
}

// Manejo de hijos con punteros 
static void agregarHijo(FSNode* parent, FSNode* hijo){
	if(!parent) return;
	if(parent->children_capacity==0){ parent->children_capacity = 4; parent->children = new FSNode*[parent->children_capacity]; }
	else if(parent->children_count==parent->children_capacity){ size_t nc = parent->children_capacity * 2; FSNode** nitems = new FSNode*[nc]; for(size_t i=0;i<parent->children_count;++i) nitems[i]=parent->children[i]; delete [] parent->children; parent->children = nitems; parent->children_capacity = nc; }
	parent->children[parent->children_count++] = hijo;
}
//Eliminacion de hijo para el arbol
static bool eliminarHijo(FSNode* parent, FSNode* hijo){
	if(!parent || parent->children_count==0) return false;
	size_t idx = parent->children_count;
	for(size_t i=0;i<parent->children_count;++i) if(parent->children[i]==hijo){ idx = i; break; }
	if(idx==parent->children_count) return false;
	for(size_t i=idx;i+1<parent->children_count;++i) parent->children[i]=parent->children[i+1];
	parent->children_count--;
	parent->children[parent->children_count] = nullptr;
	return true;
}

// Sirve para buscar hijo por nombre
static FSNode* buscarHijo(FSNode* dir, const string &name){
	if(!dir || !dir->isDir) return nullptr;
	for(size_t i=0;i<dir->children_count;++i) if(dir->children[i] && dir->children[i]->name==name) return dir->children[i];
	return nullptr;
}