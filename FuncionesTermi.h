#include#include "Arbotivas.h"
#include <limits>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

inline void imprimirPrompt(FSNode* cwd){
	ArregloCadenas partes;
	FSNode* cur = cwd;
	while(cur && cur->parent){ agregarCadena(partes, cur->name); cur = cur->parent; }
	invertirArregloCadenas(partes);
	string ruta = unirRuta(partes);
	liberarArregloCadenas(partes);
	cout << ruta << " $ ";
}

static void normalizar(string &s){
	s.erase(remove(s.begin(), s.end(), '\0'), s.end());
	if(!s.empty() && s.back()=='\r') s.pop_back();
}

inline void menu(){
	FSNode* root = nullptr;
	FSNode* cwd = nullptr;
	string dataFile;
	cout << "Archivo de sistema (ruta) a cargar/guardar: ";
	getline(cin, dataFile);
	normalizar(dataFile);
	if(dataFile.empty()){
		cout << "Usando archivo por defecto: fs_data.txt" << endl;
		dataFile = "fs_data.txt";
	}
	if(!cargarFSDesdeArchivo(root, dataFile)){
		// iniciar vacio
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
		// parse
		normalizar(line);
		stringstream ss(line);
		string cmd; ss >> cmd;
		if(cmd=="exit"){
			cout << "Guardando y saliendo..." << endl;
			if(!guardarFSaArchivo(root, dataFile)) cout << "Error al guardar en " << dataFile << endl;
			liberarArbol(root);
			break;
		} else if(cmd=="pwd"){
			ArregloCadenas partes; FSNode* cur=cwd; while(cur && cur->parent){ agregarCadena(partes, cur->name); cur=cur->parent; } invertirArregloCadenas(partes); cout<<unirRuta(partes)<<endl; liberarArregloCadenas(partes);
		} else if(cmd=="ls"){
			ArregloCadenas nombres;
			for(size_t i=0;i<cwd->children_count;++i){ FSNode* c = cwd->children[i]; agregarCadena(nombres, (c->isDir?"[D] ":"[F] ")+c->name); }
			sort(nombres.items, nombres.items + nombres.count);
			for(size_t i=0;i<nombres.count;++i) cout<<nombres.items[i]<<"  "; cout<<endl;
			liberarArregloCadenas(nombres);
		} else if(cmd=="cd"){
			string arg; ss >> ws; getline(ss,arg); if(arg.empty()){ cwd = root; continue; }
			FSNode* target = resolverRuta(cwd, arg, root);
			if(!target) cout<<"Ruta no encontrada: "<<arg<<endl;
			else if(!target->isDir) cout<<"No es un directorio: "<<arg<<endl;
			else cwd = target;
		} else if(cmd=="mkdir"){
			string arg; ss >> ws; getline(ss,arg); if(arg.empty()){ cout<<"uso: mkdir <nombre_o_ruta>"<<endl; continue; }
			if(arg[0]=='/'){
				// absolute path: create directories up to final
				crearDirectoriosParaRuta(root, arg);
			} else {
				// relative create under cwd (support nested)
				ArregloCadenas partes = dividirRuta(arg);
				FSNode* cur = cwd;
				for(size_t i=0;i<partes.count;++i){ const string &p = partes.items[i]; FSNode* child = buscarHijo(cur, p); if(child){ if(!child->isDir){ cout<<"Existe un archivo con ese nombre: "<<p<<"\n"; break; } cur = child; } else { FSNode* nd = new FSNode(p, true, cur); agregarHijo(cur, nd); nd->parent = cur; cur = nd; } }
				liberarArregloCadenas(partes);
			}
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
		} else if(cmd=="cat"){
			string arg; ss >> ws; getline(ss,arg); if(arg.empty()){ cout<<"uso: cat <ruta>"<<endl; continue; }
			FSNode* t = resolverRuta(cwd, arg, root);
			if(!t) cout<<"Ruta no encontrada"<<endl; else if(t->isDir) cout<<"Es un directorio"<<endl; else cout<<t->content<<endl;
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
		} else if(cmd=="mv"){
			string src, dst; ss >> src >> dst; if(src.empty()||dst.empty()){ cout<<"uso: mv <src> <dst>"<<endl; continue; }
			FSNode* s = resolverRuta(cwd, src, root);
			if(!s){ cout<<"src no existe"<<endl; continue; }
			FSNode* d = resolverRuta(cwd, dst, root);
			if(d && !d->isDir){ cout<<"dst existe y no es directorio"<<endl; continue; }
			if(d){ // move into directory
				// remove from s->parent children
				eliminarHijo(s->parent, s);
				s->parent = d; agregarHijo(d, s);
			} else {
				// dst does not exist: interpret as new path (rename/move)
				// build parent
				string dstPath = dst;
				bool abs = dstPath.size() && dstPath[0]=='/';
				ArregloCadenas partes = dividirRuta(dstPath);
				if(partes.count==0){ cout<<"dst invalido"<<endl; continue; }
				string newname = partes.items[partes.count-1]; partes.count--;
				string parentPath = unirRuta(partes);
				FSNode* parent = abs ? crearDirectoriosParaRuta(root, parentPath) : resolverRuta(cwd, parentPath.empty()?string("."):parentPath, root);
				if(!parent){ cout<<"No se pudo obtener carpeta padre"<<endl; liberarArregloCadenas(partes); continue; }
				// detach
				eliminarHijo(s->parent, s);
				s->parent = parent; s->name = newname; agregarHijo(parent, s);
				liberarArregloCadenas(partes);
			}
		} else if(cmd=="ren" || cmd=="rename"){
			string target, newname; ss >> target >> newname; if(target.empty()||newname.empty()){ cout<<"uso: ren <ruta> <nuevo_nombre>"<<endl; continue; }
			FSNode* t = resolverRuta(cwd, target, root);
			if(!t) { cout<<"No existe"<<endl; continue; }
			t->name = newname;
		} else if(cmd=="help"){
			cout<<"Comandos: cd ls mkdir touch mv ren edit cat pwd exit help"<<endl;
		} else {
			cout<<"Comando no reconocido (help): "<<cmd<<endl;
		}
	}
}
