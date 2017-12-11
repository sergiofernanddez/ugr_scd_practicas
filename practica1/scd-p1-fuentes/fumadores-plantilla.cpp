#include <iostream>
#include<string>
#include<iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

Semaphore fumadores[3] = {0,0,0};
Semaphore estanquero;
string vector_ingredientes[3] = {"cerillas", "tabaco", "papel"};
mutex mtx;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
    while( true ){
      Producir();
    }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

void producir(){
  sem_wait(estanquero);
  
  unsigned int ingrediente = (rand() % 3U);
  mtx.lock();

  cout << "Producido el ingrediente: " << vector_ingredientes[ingrediente] << endl;
  mtx.unlock();

  sem_signal(fumador[ingrediente]);
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
     sem_wait(fumadores[num_fumador]);
     cout << "Retirando " << vector_ingredientes[num_fumador] << " de la mesa." << endl;
     sem_signal(estanquero);
     mtx.lock();
     fumar(num_fumador);
     sem_signal(fumadores[num_fumador]);
     mtx.unlock();

   }
}

//----------------------------------------------------------------------

int main()
{
  thread estanquero(funcion_hebra_estanquero);
  thread fumadores[3];

  estanquero.join();

   for (int i=0; i<3; i++){
     fumadores[i](funcion_hebra_fumador(i));
     fumadores[i].join();
   }

   return 0;

}
