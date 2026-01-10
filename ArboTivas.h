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

Nodo *crearNodo(int valor){
    Nodo* nuevoNodo = new Nodo();
    nuevoNodo->dato = valor;
    nuevoNodo->altura = 1; 
    nuevoNodo->izq = nullptr;
    nuevoNodo->derch = nullptr;
    return nuevoNodo;
}


Nodo* rotacionDerecha(Nodo* p) {
    Nodo* q = p->izq;
    Nodo* T2 = q->derch;

    q->derch = p;
    p->izq = T2;

    p->altura = maximo(obtenerAltura(p->izq), obtenerAltura(p->derch)) + 1;
    q->altura = maximo(obtenerAltura(q->izq), obtenerAltura(q->derch)) + 1;

    return q;
}

Nodo* rotacionIzquierda(Nodo* q) {
    Nodo* p = q->derch;
    Nodo* T2 = p->izq;

    p->izq = q;
    q->derch = T2;

    q->altura = maximo(obtenerAltura(q->izq), obtenerAltura(q->derch)) + 1;
    p->altura = maximo(obtenerAltura(p->izq), obtenerAltura(p->derch)) + 1;

    return p;
}

int obtenerBalance(Nodo* n) {
    if (n == nullptr) return 0;
    return obtenerAltura(n->izq) - obtenerAltura(n->derch);
}

bool buscarNodo(Nodo* arbol, int valor) {
    if (arbol == nullptr) {
        return false;
    }
    if (arbol->dato == valor) {
        return true; 
    }

    if (valor < arbol->dato) {
        return buscarNodo(arbol->izq, valor);
    } else {
        return buscarNodo(arbol->derch, valor);
    }
}



Nodo* insertarAVL(Nodo* nodo, int valor) {
    if (nodo == nullptr)
        return crearNodo(valor);

    if (valor < nodo->dato)
        nodo->izq = insertarAVL(nodo->izq, valor);
    else if (valor > nodo->dato)
        nodo->derch = insertarAVL(nodo->derch, valor);
    else
        return nodo;

    nodo->altura = 1 + maximo(obtenerAltura(nodo->izq), obtenerAltura(nodo->derch));

    int balance = obtenerBalance(nodo);

    if (balance > 1 && valor < nodo->izq->dato)
        return rotacionDerecha(nodo);

    if (balance < -1 && valor > nodo->derch->dato)
        return rotacionIzquierda(nodo);

    if (balance > 1 && valor > nodo->izq->dato) {
        nodo->izq = rotacionIzquierda(nodo->izq);
        return rotacionDerecha(nodo);
    }

    if (balance < -1 && valor < nodo->derch->dato) {
        nodo->derch = rotacionDerecha(nodo->derch);
        return rotacionIzquierda(nodo);
    }

    return nodo;
}

void insertar(Nodo* &arbol, int valor){
    arbol = insertarAVL(arbol, valor);
}


Nodo* eliminarAVL(Nodo* raiz, int valor) {
    if (raiz == nullptr)
        return raiz;

    if (valor < raiz->dato)
        raiz->izq = eliminarAVL(raiz->izq, valor);
    else if (valor > raiz->dato)
        raiz->derch = eliminarAVL(raiz->derch, valor);
    else {
        if ((raiz->izq == nullptr) || (raiz->derch == nullptr)) {
            Nodo *temp = raiz->izq ? raiz->izq : raiz->derch;

            if (temp == nullptr) { // Caso: Sin hijos
                temp = raiz;
                raiz = nullptr;
            } else { // Caso: Un hijo
                *raiz = *temp; // Copiar contenido del hijo no vacío
            }
            delete temp;
        } else {
            // Caso: Dos hijos. Obtener el sucesor en inorden (menor del subárbol derecho)
            Nodo* temp = nodoMinimo(raiz->derch);

            // Copiar el dato del sucesor a este nodo
            raiz->dato = temp->dato;

            // Eliminar el sucesor
            raiz->derch = eliminarAVL(raiz->derch, temp->dato);
        }
    }

    if (raiz == nullptr)
        return raiz;

    
    raiz->altura = 1 + maximo(obtenerAltura(raiz->izq), obtenerAltura(raiz->derch));

    int balance = obtenerBalance(raiz);

    // Caso Izquierda Izquierda
    if (balance > 1 && obtenerBalance(raiz->izq) >= 0)
        return rotacionDerecha(raiz);

    // Caso Izquierda Derecha
    if (balance > 1 && obtenerBalance(raiz->izq) < 0) {
        raiz->izq = rotacionIzquierda(raiz->izq);
        return rotacionDerecha(raiz);
    }

    // Caso Derecha Derecha
    if (balance < -1 && obtenerBalance(raiz->derch) <= 0)
        return rotacionIzquierda(raiz);

    // Caso Derecha Izquierda
    if (balance < -1 && obtenerBalance(raiz->derch) > 0) {
        raiz->derch = rotacionDerecha(raiz->derch);
        return rotacionIzquierda(raiz);
    }

    return raiz;
}

void eliminar(Nodo* &arbol, int valor) {
    arbol = eliminarAVL(arbol, valor);
}


void mostrarArbol(Nodo* arbol ,int cont){
    if (arbol == nullptr){
        return;
    }else{
        mostrarArbol(arbol->derch,cont+1);
        for (int i = 0; i < cont; i++)
        {
            cout<<"   "; 
        }
        cout<<arbol->dato<<endl;
        mostrarArbol(arbol->izq,cont+1);
    }
}
