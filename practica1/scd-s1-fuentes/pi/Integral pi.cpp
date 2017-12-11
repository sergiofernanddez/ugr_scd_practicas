/*
    @autor: Sergio Fernández
    @Universidad de Granada
    @asignatura: Sistemas concurrentes y distribuidos
    @descripción: programa para el trabajo con hebras sobre el cálculo del área de una integral
*/

#include <iostream>
#include <chrono>
#include<future>
using namespace std ;
using namespace std::chrono;

unsigned long m = 999999;           //m representa el numero de muestras que se van a evaluar 
unsigned long n = 4;                //n representa el numero de hebras

//Implementación de la función f para realizar su integral
double f(double x)
{
    return 4.0/(1+x*x);
}

double integral_secuencial()
{
    double suma = 0.0;
    for(unsigned long i=0; i<m; i++)
        suma+=f((i+0.5)/m);
        return suma/m;
}

//Funcion concurrente 
double funcion_hebra(long ih)
{
    double suma_parcial = 0.0;
    for(long i=ih*(m/n); i < ih*(m/n) + (m/n); i++)
        suma_parcial+=f((i+0.5)/m);

    return suma_parcial/m;
    
}

double integral_concurrente()
{
    double suma = 0.0;
    future<double> hebras[n];

    for(int i=0; i<n; i++)
        hebras[i] = async(launch::async, funcion_hebra, i);

    for(int i=0; i<n; i++)
        suma+=hebras[i].get();

        return suma;
}

int main(){
    const double PI = 3.14159265358979312;

    //Calculo secuencial
    time_point<steady_clock> tinicial_secuencial = steady_clock::now();
    const double pi_secuencial = integral_secuencial();
    time_point<steady_clock> tfinal_secuencial = steady_clock::now();
    duration<float,micro> duracion_secuencial = (tfinal_secuencial - tinicial_secuencial);
    
    //Calculo concurrente
    time_point<steady_clock> tinicial_concurrente = steady_clock::now();
    const double pi_concurrente = integral_concurrente();
    time_point<steady_clock> tfinal_concurrente = steady_clock::now();
    duration<float,micro> duracion_concurrente = (tfinal_concurrente - tinicial_concurrente);

    //Imprimir cálculos
    cout.precision(20);
    cout <<"\nNumero de muestras          :" << m
         <<"\nNumero de hebras            :" << n 
         <<"\nValor de pi                 :" << PI
         <<"\nIntegral secuencial         :" << pi_secuencial
         <<"\nIntegral concurrente        :" << pi_concurrente
         <<"\nDuracion secuencial         :" << duracion_secuencial.count() << " milisegundos"
         <<"\nDuracion concurrente        :" << duracion_concurrente.count() << " milisegundos"
         <<"\nPorcentaje tcon/tsec        :" << duracion_concurrente.count()/(duracion_secuencial.count())*100;

    return 0;
}






