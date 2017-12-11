// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// archivo: prodcons3_su.cpp
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con varios productores y  consumidores.
// Opcion LIFO (stack)
//
// Historial:
// Creado en Julio de 2017
// -----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.hpp"

using namespace HM;
using namespace std;

constexpr int
    num_items = 40, // número de items a producir/consumir
    num_prod = 4,
    num_cons = 5,
    prod_por_hebra = num_items / num_prod,
    cons_por_hebra = num_items / num_cons;

mutex
    mtx; // mutex de escritura en pantalla
unsigned
    cont_prod[num_items], // contadores de verificación: producidos
    cont_cons[num_items], // contadores de verificación: consumidos
    cont_hebras[num_prod];

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template <int min, int max>
int aleatorio()
{
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min, max);
  return distribucion_uniforme(generador);
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(int hebra)
{
  static int contador = 0;
  this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));
  mtx.lock();
  cout << "producido: " << contador << endl
       << flush;
  mtx.unlock();
  if (cont_hebras[hebra] < prod_por_hebra)
  {
    contador = hebra * prod_por_hebra + cont_hebras[hebra];
    cont_hebras[hebra]++;
  }
  cont_prod[contador]++;
  return contador;
}
//----------------------------------------------------------------------

void consumir_dato(unsigned dato)
{
  if (num_items <= dato)
  {
    cout << " dato === " << dato << ", num_items == " << num_items << endl;
    assert(dato < num_items);
  }
  cont_cons[dato]++;
  this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));
  mtx.lock();
  cout << "                  consumido: " << dato << endl;
  mtx.unlock();
}
//----------------------------------------------------------------------

void ini_contadores()
{
  for (unsigned i = 0; i < num_items; i++)
  {
    cont_prod[i] = 0;
    cont_cons[i] = 0;
  }
}

//----------------------------------------------------------------------

void test_contadores()
{
  bool ok = true;
  cout << "comprobando contadores ...." << flush;

  for (unsigned i = 0; i < num_items; i++)
  {
    if (cont_prod[i] != 1)
    {
      cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl;
      ok = false;
    }
    if (cont_cons[i] != 1)
    {
      cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl;
      ok = false;
    }
  }
  if (ok)
    cout << endl
         << flush << "solución (aparentemente) correcta." << endl
         << flush;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SU, varios prod. y varios cons.

class ProdCons2SU : public HoareMonitor
{
private:
  static const int              // constantes:
      num_celdas_total = 10;    //  núm. de entradas del buffer
  int                           // variables permanentes
      buffer[num_celdas_total], //  buffer de tamaño fijo, con los datos
      primera_libre;            //  indice de celda de la próxima inserción
  // mutex
  //     cerrojo_monitor; // cerrojo del monitor
  CondVar              // colas condicion:
      ocupadas,        //  cola donde espera el consumidor (n>0)
      libres;          //  cola donde espera el productor  (n<num_celdas_total)

public:                     // constructor y métodos públicos
  ProdCons2SU();            // constructor
  int leer();               // extraer un valor (sentencia L) (consumidor)
  void escribir(int valor); // insertar un valor (sentencia E) (productor)
};
// -----------------------------------------------------------------------------

ProdCons2SU::ProdCons2SU()
{
  primera_libre = 0;
  ocupadas = newCondVar();
  libres = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdCons2SU::leer()
{
  static int contador = 0; 

  if ( ! libres.empty() )
    contador ++;
  else 
    contador = 0;
  
  if (contador == 10)
    ocupadas.wait();
  // esperar bloqueado hasta que 0 < num_celdas_ocupadas
  if (primera_libre == 0)
    ocupadas.wait();

  // hacer la operación de lectura, actualizando estado del monitor
  assert(0 < primera_libre);
  primera_libre--;
  const int valor = buffer[primera_libre];

  // señalar al productor que hay un hueco libre, por si está esperando
  libres.signal();

  // devolver valor
  return valor;
}
// -----------------------------------------------------------------------------

void ProdCons2SU::escribir(int valor)
{
  // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
  if (primera_libre == num_celdas_total)
    libres.wait();

  //cout << "escribir: ocup == " << num_celdas_ocupadas << ", total == " << num_celdas_total << endl ;
  assert(primera_libre < num_celdas_total);

  // hacer la operación de inserción, actualizando estado del monitor
  buffer[primera_libre] = valor;
  primera_libre++;

  // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
  ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora(MRef<ProdCons2SU> monitor, int hebra)
{
  for (unsigned i = 0; i < prod_por_hebra; i++)
  {
    int valor = producir_dato(hebra);
    monitor->escribir(valor);
  }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora(MRef<ProdCons2SU> monitor)
{
  for (unsigned i = 0; i < cons_por_hebra; i++)
  {
    int valor = monitor->leer();
    consumir_dato(valor);
  }
}
// -----------------------------------------------------------------------------

int main()
{
  cout << "-------------------------------------------------------------------------------" << endl
       << "Problema de los productores-consumidores (Varios prod/cons, Monitor SU, buffer LIFO). " << endl
       << "-------------------------------------------------------------------------------" << endl
       << flush;

  for (int i = 0; i < num_prod; i++)  //Inicializamos el contador de cada hebra a 0
    cont_hebras[i] = 0;

  auto monitor = Create<ProdCons2SU>();
  thread hebra_productora[num_prod];
  for (int i = 0; i < num_prod; i++)
  hebra_productora[i] = thread(funcion_hebra_productora, monitor, i);
  
  thread hebra_consumidora[num_cons];
  for (int i = 0; i < num_cons; i++)
    hebra_consumidora[i] = thread(funcion_hebra_consumidora, monitor);

  for (int i = 0; i < num_prod; i++)
    hebra_productora[i].join();
  for (int i = 0; i < num_cons; i++)
    hebra_consumidora[i].join();

  // comprobar que cada item se ha producido y consumido exactamente una vez
  test_contadores();
}
