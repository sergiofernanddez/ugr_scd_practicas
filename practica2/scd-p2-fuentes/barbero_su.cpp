#include <iostream>
#include<string>
#include<iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

int numero_clientes = 10;

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void EsperarFueraBarberia(int n_cliente)
{
    chrono::milliseconds duracion_espera( aleatorio<20,200>() );

     cout << "Cliente " << n_cliente << " espera fuera de la barbería" << endl;

    this_thread::sleep_for( duracion_espera );

    cout <<"Cliente " << n_cliente << " termina de estar fuera y vuelve a entrar" << endl;

}

void CortarPeloACliente()
{
    chrono::milliseconds duracion_cortar( aleatorio<20,200>() );

     cout << "Barbero empieza a cortar" << endl;

    this_thread::sleep_for( duracion_cortar );
}

class Barberia : public HoareMonitor
{
    CondVar
        silla_cortar,
        sala_espera,
        barbero;

    public:
        Barberia();
        void CortarPelo(int n_cliente);
        void SiguienteCliente();
        void FinCliente();
};

Barberia::Barberia(){
    silla_cortar = newCondVar();
    sala_espera = newCondVar();
    barbero = newCondVar();
}

void Barberia::CortarPelo(int n_cliente)
{
    cout <<"\nCliente " << n_cliente << " llega a la barbería" << endl;
    if(!silla_cortar.empty())
    {
        cout << "\nBarbero pelando, cliente " << n_cliente << " se sienta en la sala de espera" << endl;
        sala_espera.wait();
    }

    cout << "\nBarbero llama a cliente " << n_cliente << " y se sienta en la silla" << endl;
    if(!barbero.empty())
        barbero.signal();

    silla_cortar.wait();

}

void Barberia::SiguienteCliente()
{
    cout << "Barbero esperando siguiente cliente" << endl;

    if (!sala_espera.empty() && silla_cortar.empty())
        sala_espera.signal();

    if (sala_espera.empty() && silla_cortar.empty())
        barbero.wait();
}
    

void Barberia::FinCliente()
{
    cout << "\nEl cliente termina de pelarse y sale de la barberia" << endl;
    silla_cortar.signal();
}

void funcion_hebra_cliente(MRef<Barberia> barberia, int n_cliente)
{
    while(true)
    {
        barberia->CortarPelo(n_cliente);
        EsperarFueraBarberia(n_cliente);
    }
}

void funcion_hebra_barbero(MRef<Barberia> barberia)
{
    while(true)
    {
        barberia->SiguienteCliente();
        CortarPeloACliente();
        barberia->FinCliente();
    }
}

int main(){

 cout << "--------------------------------------------------------" << endl
         << "Problema de la barbería." << endl
         << "--------------------------------------------------------" << endl
         << flush;

    auto monitor = Create<Barberia>();
    thread hebra_clientes[numero_clientes];

    thread barbero(funcion_hebra_barbero, monitor);

    for (int i = 0; i < numero_clientes; i++)
        hebra_clientes[i] = thread(funcion_hebra_cliente, monitor, i);

    
    for (int i = 0; i < numero_clientes; i++)
        hebra_clientes[i].join();
    barbero.join();

   return 0;


}