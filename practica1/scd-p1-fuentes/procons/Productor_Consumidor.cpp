#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std;
using namespace SEM;

const int NUMERO_VALORES = 50;            //Numero de valores consumidos y producidos
const int TAMANIO = 50;                   //Tamaño del vector
int array_datos[TAMANIO];                 //Vector que almacena datos producidos
int indice = 0;                           //Indice donde se alamcena el dato producido

//Semaforos
Semaphore consumir = 0;
Semaphore producir = 1;
mutex mtx;


template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}


//Metodo para producir datos
int producir_dato(){
    static int i = 0;
    this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

    cout << "producido: " << i << endl << flush ;
    return i++;
}

//Metodo que consume un dato
void consumir_dato(unsigned dato){
    assert(dato < NUMERO_VALORES);
    this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

    cout << "                  consumido: " << dato << endl ;
}

void hebra_productora(){

    for(int i=0; i<NUMERO_VALORES; i++){
        int dato = producir_dato();

        sem_wait(producir);
        mtx.lock();

        array_datos[i] = dato;

        sem_signal(consumir);
        mtx.unlock();
    }
       
}

void hebra_consumidora(){
    for(int i=0; i<NUMERO_VALORES; i++){
        sem_wait(consumir);
        mtx.lock();
        
        int dato = array_datos[i];
        consumir_dato(dato);
       
        sem_signal(producir);
        mtx.unlock();
        
    }
}

int main(){

   thread hebra_consumir(hebra_consumidora);
   thread hebra_producir(hebra_productora);

   hebra_consumir.join();
   hebra_producir.join();


   return 0;

}

