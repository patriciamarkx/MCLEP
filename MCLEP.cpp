/**
  * Minimum Cost Linear Extension
  * Patricia Marques.
  * Universidade Federal de Minas Gerais, Janeiro de 2022.
  */
#include <iostream>
#include <stdio.h>
#include <list>
#include <queue>
#include <vector>
#include <fstream>

using namespace std;

class VetorBits{
	int tamanho;
	unsigned int *v;

public:

	VetorBits(int tam){
		tamanho = ceil(tam / (float)sizeof(int));
		v = new unsigned int[tamanho];
		for (int i = 0; i < tamanho; i++)
			v[i] = 0;
	}

	~VetorBits(){
		delete v;
		tamanho = 0;
	}

	inline void inserirPredecessores(int e, VetorBits *y){
		int i = floor(e / 32);
		int j = (32 - (e % 32) - 1);
		v[i] = v[i] | (1 << j);
		for (int k = 0; k < tamanho; k++)
			v[k] = v[k] | y->v[k];
	};

	inline bool estaNoConjunto(int e){
		int i = floor(e / 32);
		int j = (32 - (e % 32) - 1);

		if (v[i] & (1 << j))
			return true;
		return false;
	}

	inline const int get_tamanho(){ return tamanho; }

};

class Vertice{
private:
	int _id;
	int _altura;
	int _grauDeEntrada;
	Vertice *_predecessor;
	VetorBits *_conjRest;
public:
	Vertice(int id, int n){
		_id = id;
		_altura = -1;
		_grauDeEntrada = 0;
		_predecessor = NULL;
		_conjRest = new VetorBits(n);
	}

	inline int getIndice() { return _id; }
	inline int getAltura() { return _altura; }
	inline int getGrauDeEntrada() { return _grauDeEntrada; }
	inline Vertice* getPredecessor() { return _predecessor; }

	inline void setAltura(int altura) { _altura = altura; }
	inline void setGrauDeEntrada(int grau) { _grauDeEntrada = grau; }
	void setPredecessor( Vertice * predecessor) {
		_predecessor = predecessor; 
		(predecessor == NULL)? setAltura(0): setAltura(_predecessor->getAltura() + 1 );
	}

	void setConjRest( Vertice *&v){
		_conjRest->inserirPredecessores(v->getIndice(),v->_conjRest);
	}

	inline bool restringe( Vertice *v ){	return _conjRest->estaNoConjunto(v->getIndice());
	}


};

class Arco {
private:
	int _origem;
	int _destino;
public:
	// Construtor padrao
	Arco() {
		_origem = _destino = -1;
	}
	// Construtor principal
	Arco(int origem, int destino) {
		_origem = origem;
		_destino = destino;
	}

	// Construtor de copia
	Arco(const Arco &outro) {
		_origem = outro._origem;
		_destino = outro._destino;
	}

	// Operador de atribuicao
	Arco & operator=(const Arco &outro) {
		_origem = outro._origem;
		_destino = outro._destino;
		return *this;
	}

	// Getters
	int origem() const { return _origem; }
	int destino() const { return _destino; }

	// Setters
	void setOrigem(int o) { _origem = o; }
	void setDestino(int d) { _destino = d; }
};

class Digrafo {
private:
	int _n;  // Numero de vertices
	Vertice* *_Ver;  // Vetor de ponteiros p vertices 
	list<Arco> *_Adj;        // Listas de adjacencia
	list<Arco> *_AdjReversa; // Listas de adjacencia reversa
public:
	// Construtor padrao
	Digrafo() {
		_n = 0;
		_Ver = NULL;
		_Adj = _AdjReversa = NULL;		
	}

	void definirTamanho(int n) {
		// Se o grafo ja existia, destruimo-lo
		if (_n > 0) {
			delete [] _Ver;
			delete [] _Adj;
			delete [] _AdjReversa;
		}

		// Dimensionar listas de adjacencia apropriadamente
		_n = n;
		_Ver = new Vertice*[_n];
		for(int i=0; i<_n; i++) _Ver[i]= new Vertice(i, _n);
		_Adj = new list<Arco>[_n];
		_AdjReversa = new list<Arco>[_n];
	}

	// Destrutor
	~Digrafo() {
		delete [] _Adj;
		delete [] _AdjReversa;
		delete []_Ver;
		delete _Ver;
	}

	void inserirArco(int origem, int destino) {
		Arco *novo = new Arco(origem, destino);
		_Adj[origem].push_back( *novo );
		_AdjReversa[destino].push_back( *novo );
		_Ver[destino]->setGrauDeEntrada( _Ver[destino]->getGrauDeEntrada() + 1);
	}

	// Retorna a lista de adjacencia do vertice origem.
	list<Arco> adjacencia(int origem) const {
		return _Adj[origem]; // Uma copia da lista e' retornada
	}

	// Retorna lista de adjacencia reversa do vertice destino.
	list<Arco> adjacenciaReversa(int destino) const {
		return _AdjReversa[destino];
	}

	Vertice* predecessorMax(Vertice* &v){
		list<Arco> l = adjacenciaReversa(v->getIndice());
		if( l.empty() )
			return NULL;

		Vertice* retorno = _Ver[l.begin()->origem()];

		list<Arco>::iterator pos = l.begin();
		while( pos != l.end()){
			v->setConjRest(_Ver[pos->origem()]);
			if(_Ver[pos->origem()]->getAltura() > retorno->getAltura())
				retorno = _Ver[pos->origem()];
			pos++;
		}

		return retorno;
	}

	list<Vertice*> ordenarTop(){
		list<Vertice*> ot;
		queue<Vertice*> aux;
		int *graus = new int [_n];
		
		for(int i=0; i<_n; i++){
			graus[i]=_Ver[i]->getGrauDeEntrada();
			if( graus[i] == 0 ){
				_Ver[i]->setAltura(0);
				aux.push(_Ver[i]);
			}
		}

		while( !aux.empty() ){
			Vertice *v = aux.front();
			aux.pop();
			v->setPredecessor(predecessorMax(v));
			ot.push_back(v);

			list<Arco> l = adjacencia(v->getIndice());
			list<Arco>::iterator pos = l.begin();
			while (pos != l.end()){
				graus[pos->destino()]--;
				if(graus[pos->destino()] == 0 )
					aux.push(_Ver[pos->destino()]);
				pos++;
			}
		}

		delete graus;

		return ot;
	}

	list<Vertice*> maiorCadeia(list<Vertice*> &r){
		list<Vertice*> c;
		r = ordenarTop();
		
		c.push_front(r.back());
		r.pop_back();

		while( c.front()->getPredecessor() != NULL ){
			r.remove(c.front()->getPredecessor());
			c.push_front(c.front()->getPredecessor());			
		}

		return c;
	}


};

class ExtensaoLinear{
	list<Vertice*> E;
	int valor;

public:
	ExtensaoLinear(list<Vertice*> _E){
		E = _E;
		valor = 0;
	}

	inline list<Vertice*>::iterator inicio(){ return E.begin();	}

	inline list<Vertice*>::iterator fim(){ return E.end();	}

	void setElemento(list<Vertice*>::iterator p , Vertice* &e){
		if( *p == NULL)
			E.push_back(e);
		else
			E.insert(p,e);	
	}

	void atualizarValor(int vs, int ve){
		valor = valor - vs + ve ;		
	}
	
	inline int obterValor(){ return valor; }

	inline list<Vertice*>::iterator limiteSuperior(Vertice* &v){
		list<Vertice*>::iterator pos = E.begin();

		while( pos!= E.end() ){
			if((*pos)->restringe(v))
				return pos;
			pos++;
		}
		return --pos;
	}

	inline list<Vertice*>::iterator limiteInferior(Vertice* &v){
		list<Vertice*>::iterator pos = E.end();
		--pos;
		while( pos!= E.begin() ){
			if( v->restringe(*pos) )
				return pos;
			pos--;
		}
		return pos;
	}
	
	void imprimir(){
		cout<<endl;
		cout<< " * EXTENSAO LINEAR DE CUSTO NAO MINIMO * " <<endl;
		while( ! E.empty() ){
			cout<< E.front()->getIndice() << " --> ";
			E.pop_front();
		}
		cout<< endl << " * CUSTO DA EXTENSAO = " << valor <<endl; 
	}

};

class  Poset {
private:
	int _n;     // Numero de elementos
	Digrafo _g; // Grafo dirigido associado aas relacoes
	vector<vector<int>> c;

public:
	Poset(int n) {
		_n = n;
		_g.definirTamanho(_n);	
		c.resize(_n);
		for (int i = 0; i<_n; i++)
			 c[i].resize(_n);
	}

	void inserirRelacao(int origem, int destino) {
		_g.inserirArco(origem, destino);
		c[destino][origem] = -1;
	}

	void inserirCusto(int origem, int destino, int custo) {
		 c[origem][destino] = custo;
	}

	inline int obterCusto(int origem, int destino){	return c[origem][destino];	}

	inline int obterCusto(Vertice* origem, Vertice* destino){
		if ((origem == NULL) || (destino == NULL))
			return 0;
		return c[origem->getIndice()][destino->getIndice()];
	}

	
	list<Vertice*>::iterator  melhorposicao(list<Vertice*>::iterator i, list<Vertice*>::iterator f, Vertice* v){
		list<Vertice*>::iterator retorno, pos;
		retorno = pos = i;
		retorno++;
		int aux = obterCusto(*i,v) + obterCusto(v, *++i);

		while( ++pos != f ){
			if( aux > (obterCusto(*--pos, v) + obterCusto(v, *++pos)) ){
						retorno = pos;	
						aux = (obterCusto(*--pos, v) + obterCusto(v, *++pos));
			}
		}
		
		return retorno;		
	}

	ExtensaoLinear obterExtensaoLinear(){
		list<Vertice*> R;
		ExtensaoLinear retorno(_g.maiorCadeia(R));
		list<Vertice*>::iterator p;

		while( !R.empty()){
			Vertice *v = R.front();
			R.pop_front();
			p = melhorposicao(retorno.limiteInferior(v),retorno.limiteSuperior(v), v);
			retorno.setElemento( p , v);
			//retorno.atualizarValor();
		}

		return retorno;
	}
	


	void imprimir(){
		
		obterExtensaoLinear().imprimir();

	}
	
};



int main(int argc, char **args) {
	
	ifstream arquivo;
	arquivo.open("arq3.txt");

	int tam, nrelacoes, ncustos;
	arquivo >> tam;
	arquivo >> nrelacoes;
	arquivo >> ncustos;
	Poset P(tam);

	int vertice1, vertice2, custo;
	for(int i=0; i<nrelacoes; i++){
		arquivo >> vertice1 >> vertice2;
		P.inserirRelacao(vertice1, vertice2);
	}

	while (!arquivo.eof()){
		arquivo >> vertice1 >> vertice2 >> custo;
		P.inserirCusto(vertice1, vertice2, custo);
	}
	arquivo.close();

	P.imprimir();


	system("pause");
	return 0;
}