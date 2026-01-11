#include "Arbotivas.h"

// Estructura de Pila (Stack) para el historial para hacer uso del comando Back
struct HistorialNodos {
    FSNode** nodos;      
    size_t count;        
    size_t capacity;     
    
    HistorialNodos() : nodos(nullptr), count(0), capacity(0) {}
};

// Función para empujar (guardar) un nodo al historial
static void pushHistorial(HistorialNodos &h, FSNode* nodo) {
    if (h.capacity == 0) {
        h.capacity = 4;
        h.nodos = new FSNode*[h.capacity];
    } else if (h.count == h.capacity) {
        size_t nc = h.capacity * 2;
        FSNode** nuevos = new FSNode*[nc];
        for (size_t i = 0; i < h.count; ++i) nuevos[i] = h.nodos[i];
        delete[] h.nodos;
        h.nodos = nuevos;
        h.capacity = nc;
    }
    h.nodos[h.count++] = nodo;
}

// Función para sacar (recuperar) el último nodo del historial
static FSNode* popHistorial(HistorialNodos &h) {
    if (h.count == 0) return nullptr;
    h.count--;
    return h.nodos[h.count];
}

// Limpieza de memoria del historial al salir
static void liberarHistorial(HistorialNodos &h) {
    if (h.nodos) delete[] h.nodos;
    h.nodos = nullptr;
    h.count = h.capacity = 0;
}
// Sirve para calcular la ruta desde la raíz hasta el directorio actual
inline void imprimirPrompt(FSNode* cwd){
	ArregloCadenas partes;
	FSNode* cur = cwd;
	while(cur && cur->parent){ agregarCadena(partes, cur->name); cur = cur->parent; }
	invertirArregloCadenas(partes);
	string ruta = unirRuta(partes);
	liberarArregloCadenas(partes);
	cout << ruta << " $ ";
}
// Elimina caracteres nulos 
static void normalizar(string &s){
	s.erase(remove(s.begin(), s.end(), '\0'), s.end());
	if(!s.empty() && s.back()=='\r') s.pop_back();
}

//Funcion principal para el main
inline void menu(){
    FSNode* root = nullptr;
    FSNode* cwd = nullptr;
    HistorialNodos historial; 
    string dataFile;
    cout << "Archivo de sistema (ruta) a cargar/guardar: ";
    getline(cin, dataFile);
    normalizar(dataFile);

    if(dataFile.empty()){
        cout << "Usando archivo por defecto: fs_data.txt" << endl;
        dataFile = "fs_data.txt";
    }
    if(!cargarFSDesdeArchivo(root, dataFile)){
        root = new FSNode("", true, nullptr);
        cout << "Archivo no encontrado o vacio. Iniciando sistema de archivos vacio." << endl;
    } else {
        cout << "Sistema cargado desde: " << dataFile << endl;
    }
    cwd = root;

    string line;
    while(true){
        imprimirPrompt(cwd);
        if(!getline(cin, line)) break;
        if(line.empty()) continue;

        normalizar(line);
        stringstream ss(line);
        string cmd; ss >> cmd;
        //Comando para salir
        if(cmd=="exit"){
            cout << "Guardando y saliendo..." << endl;
            if(!guardarFSaArchivo(root, dataFile)) cout << "Error al guardar en " << dataFile << endl;
            liberarArbol(root);
            liberarHistorial(historial);
            break;
        } 	//Comando para regresar un directorio atras
        else if(cmd == "b" || cmd == "back") {
            FSNode* anterior = popHistorial(historial);
            if(anterior == nullptr) {
                cout << "No hay historial para retroceder." << endl;
            } else {
                cwd = anterior;
            }
        } //Comando para ir hacia un directorio
        else if(cmd=="cd"){
            string arg; ss >> ws; getline(ss,arg); 
            if(arg.empty()){ 
                if(cwd != root) pushHistorial(historial, cwd);
                cwd = root; 
                continue; 
            }
            FSNode* target = resolverRuta(cwd, arg, root);
            if(!target) cout<<"Ruta no encontrada: "<<arg<<endl;
            else if(!target->isDir) cout<<"No es un directorio: "<<arg<<endl;
            else {
                if(target != cwd) {
                    pushHistorial(historial, cwd);
                }
                cwd = target;
            }
        } 	//Comando para decir en que carpeta o ruta te encuentras
			else if(cmd=="pwd"){
				ArregloCadenas partes; FSNode* cur=cwd; while(cur && cur->parent){ agregarCadena(partes, cur->name); cur=cur->parent; } invertirArregloCadenas(partes); cout<<unirRuta(partes)<<endl; liberarArregloCadenas(partes);
			//Comando para Ver la lista archivos y carpetas en la ubicación actual.
			} else if(cmd=="ls"){
				ArregloCadenas nombres;
				for(size_t i=0;i<cwd->children_count;++i){ FSNode* c = cwd->children[i]; agregarCadena(nombres, (c->isDir?"[D] ":"[F] ")+c->name); }
				sort(nombres.items, nombres.items + nombres.count);
				for(size_t i=0;i<nombres.count;++i) cout<<nombres.items[i]<<"  "; cout<<endl;
				liberarArregloCadenas(nombres);
				//Comando para Crear una carpeta o directorio
			} else if(cmd=="mkdir"){
				string arg; ss >> ws; getline(ss,arg); if(arg.empty()){ cout<<"uso: mkdir <nombre_o_ruta>"<<endl; continue; }
				if(arg[0]=='/'){ crearDirectoriosParaRuta(root, arg); } 
				else { ArregloCadenas partes = dividirRuta(arg); FSNode* cur = cwd; for(size_t i=0;i<partes.count;++i){ const string &p = partes.items[i]; FSNode* child = buscarHijo(cur, p); if(child){ if(!child->isDir){ cout<<"Existe un archivo con ese nombre: "<<p<<"\n"; break; } cur = child; } else { FSNode* nd = new FSNode(p, true, cur); agregarHijo(cur, nd); nd->parent = cur; cur = nd; } } liberarArregloCadenas(partes); }
				//Comando para Crear un archivo vacio
			} else if(cmd=="touch"){
				string arg; ss >> ws; getline(ss,arg); if(arg.empty()){ cout<<"uso: touch <nombre_o_ruta>"<<endl; continue; }
			string path = arg;
			if(path[0]=='/'){
				ArregloCadenas partes = dividirRuta(path);
				if(partes.count==0){ cout<<"ruta invalida"<<endl; continue; }
				string filename = partes.items[partes.count-1]; partes.count--; string parentPath = unirRuta(partes);
				FSNode* parent = (parentPath=="/")? root : crearDirectoriosParaRuta(root, parentPath);
				if(!parent){ cout<<"Error creando/obteniendo carpeta padre"<<endl; liberarArregloCadenas(partes); continue; }
				FSNode* existing = buscarHijo(parent, filename);
				if(existing){ if(existing->isDir) cout<<"Ya existe un directorio con ese nombre"<<endl; else cout<<"Archivo ya existe"<<endl; }
				else{ FSNode* f = new FSNode(filename, false, parent); agregarHijo(parent, f); f->parent = parent; }
				liberarArregloCadenas(partes);
			} else {
				ArregloCadenas partes = dividirRuta(path);
				if(partes.count==0){ cout<<"ruta invalida"<<endl; continue; }
				string filename = partes.items[partes.count-1]; partes.count--;
				FSNode* parent = cwd;
				for(size_t i=0;i<partes.count;++i){ const string &p = partes.items[i]; FSNode* ch = buscarHijo(parent, p); if(!ch){ ch = new FSNode(p, true, parent); agregarHijo(parent, ch); ch->parent = parent; } parent = ch; }
				FSNode* existing = buscarHijo(parent, filename);
				if(existing){ if(existing->isDir) cout<<"Ya existe un directorio con ese nombre"<<endl; else cout<<"Archivo ya existe"<<endl; }
				else{ FSNode* f = new FSNode(filename, false, parent); agregarHijo(parent, f); f->parent = parent; }
				liberarArregloCadenas(partes);
			}
			//Comando para Mostrar el contenido de texto de un archivo.
        } else if(cmd=="cat"){
             string arg; ss >> ws; getline(ss,arg); if(arg.empty()){ cout<<"uso: cat <ruta>"<<endl; continue; }
			FSNode* t = resolverRuta(cwd, arg, root);
			if(!t) cout<<"Ruta no encontrada"<<endl; else if(t->isDir) cout<<"Es un directorio"<<endl; else cout<<t->content<<endl;
			//Comando para escribir texto dentro de un archivo.
		} else if(cmd=="edit"){
             string arg; ss >> ws; getline(ss,arg); if(arg.empty()){ cout<<"uso: edit <ruta>"<<endl; continue; }
			FSNode* t = resolverRuta(cwd, arg, root);
			if(!t) { cout<<"No existe: "<<arg<<" (crea con touch)"<<endl; continue; }
			if(t->isDir){ cout<<"No es un archivo"<<endl; continue; }
			cout<<"Contenido actual:\n"<<t->content<<"\n--- Fin contenido ---\n";
			cout<<"Ingrese nuevo contenido (una linea con solo . para terminar):\n";
			string ln; string newc;
			while(true){ if(!getline(cin, ln)) break; normalizar(ln); if(ln==".") break; newc += ln; newc += '\n'; }
			t->content = newc;
			//Comando para Mover Directorios
        } else if(cmd=="mv"){
             string src, dst; ss >> src >> dst; if(src.empty()||dst.empty()){ cout<<"uso: mv <src> <dst>"<<endl; continue; }
			FSNode* s = resolverRuta(cwd, src, root);
			if(!s){ cout<<"src no existe"<<endl; continue; }
			FSNode* d = resolverRuta(cwd, dst, root);
			if(d && !d->isDir){ cout<<"dst existe y no es directorio"<<endl; continue; }
			if(d){
				eliminarHijo(s->parent, s);
				s->parent = d; agregarHijo(d, s);
			} else {
				string dstPath = dst;
				bool abs = dstPath.size() && dstPath[0]=='/';
				ArregloCadenas partes = dividirRuta(dstPath);
				if(partes.count==0){ cout<<"dst invalido"<<endl; continue; }
				string newname = partes.items[partes.count-1]; partes.count--;
				string parentPath = unirRuta(partes);
				FSNode* parent = abs ? crearDirectoriosParaRuta(root, parentPath) : resolverRuta(cwd, parentPath.empty()?string("."):parentPath, root);
				if(!parent){ cout<<"No se pudo obtener carpeta padre"<<endl; liberarArregloCadenas(partes); continue; }
				eliminarHijo(s->parent, s);
				s->parent = parent; s->name = newname; agregarHijo(parent, s);
				liberarArregloCadenas(partes);
			}
			//Comando para Renombrar un Directorio o Carpeta
        } else if(cmd=="ren" || cmd=="rename"){
             string target, newname; ss >> target >> newname; if(target.empty()||newname.empty()){ cout<<"uso: ren <ruta> <nuevo_nombre>"<<endl; continue; }
			FSNode* t = resolverRuta(cwd, target, root);
			if(!t) { cout<<"No existe"<<endl; continue; }
			t->name = newname;
			//Comando para Mostrar Menu de ayuda donde salen todos los comandos de la terminal
        } else if(cmd=="help"){
            cout<<"Comandos: cd ls mkdir touch mv ren edit cat pwd exit help back(b)"<<endl;
        } else {
            cout<<"Comando no reconocido (help): "<<cmd<<endl;
        }
    }
}