#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

const unsigned
    num_clientes = 3; //Número de clientes del barbero

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

//-------------------------------------------------------------------------
// Función que simula la acción de esperar fuera de la barberia, como un retardo aleatoria de la hebra
void EsperarFueraBarberia(int cliente)
{
    chrono::milliseconds duracion_espera(aleatorio<20, 200>());

    cout << "Cliente " << cliente << "  :"
         << " espera fuera (" << duracion_espera.count() << " milisegundos)" << endl;

    this_thread::sleep_for(duracion_espera);

    cout << "Cliente " << cliente << "  : termina de estar fuera, entra en la barbería." << endl;
}

// Función que simula el tiempo que tarda el barbero en cortar el pelo, como un retardo aleatorio en cada llamada
void cortarPeloACliente()
{
    chrono::milliseconds duracion_corte(aleatorio<20, 200>());

    cout << "Barbero  : empieza a cortar (" << duracion_corte.count() << " milisegundos)" << endl;

    this_thread::sleep_for(duracion_corte);
}

class Barberia : public HoareMonitor
{
  private:
    CondVar
        sala_espera,   //Cola condición para los clientes que esperan
        silla_barbero, //Cola condición para el cliente que se está pelando
        barbero;       //Cola condición para el barbero si no hay ningún cliente

  public:
    Barberia();
    void cortarPelo(unsigned i);
    void siguienteCliente();
    void finCliente();
};

Barberia::Barberia()
{
    sala_espera = newCondVar();
    silla_barbero = newCondVar();
    barbero = newCondVar();
}

void Barberia::cortarPelo(unsigned i)
{
    if (!silla_barbero.empty())
    {
        cout << "Cliente : " << i << " se sienta en la sala de espera" << endl;
        sala_espera.wait();
    }

    cout << "Cliente : " << i << "  empieza a cortarse el pelo" << endl;
    if (!barbero.empty())
        barbero.signal();

    silla_barbero.wait();
}

void Barberia::siguienteCliente()
{
    cout << "Barbero preparado para siguiente cliente" << endl;

    if (!sala_espera.empty() && silla_barbero.empty())
        sala_espera.signal();

    if (sala_espera.empty() && silla_barbero.empty())
        barbero.wait();
}

void Barberia::finCliente()
{
    cout << "Corte de pelo terminado, el cliente sale de la barberia" << endl;
    silla_barbero.signal();
}
//----------------------------------------------------------------------
// función que ejecuta la hebra del barbero
void funcion_hebra_barbero(MRef<Barberia> barberia)
{
    while (true)
    {
        barberia->siguienteCliente();
        cortarPeloACliente();
        barberia->finCliente();
    }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra de los clientes

void funcion_hebra_cliente(int num_cliente, MRef<Barberia> barberia)
{
    while (true)
    {
        barberia->cortarPelo(num_cliente);
        EsperarFueraBarberia(num_cliente);
    }
}
//----------------------------------------------------------------------

int main()
{
    cout << "--------------------------------------------------------" << endl
         << "Problema del barbero." << endl
         << "--------------------------------------------------------" << endl
         << flush;

    auto monitor = Create<Barberia>();
    
    thread hebra_barbero(funcion_hebra_barbero, monitor);

    thread hebra_clientes[num_clientes];
    for (int i = 0; i < num_clientes; i++)
        hebra_clientes[i] = thread(funcion_hebra_cliente, i, monitor);


    for (int i = 0; i < num_clientes; i++)
        hebra_clientes[i].join();
    hebra_barbero.join();
}