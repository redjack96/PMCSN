//
// Created by giaco on 13/04/2021.
// Servente singolo Bounded Pareto FIFO
//

#define PARETO

double Pareto(double k, double p, double alpha) {
    return k / (1 + Random() * (pow(k / p, alpha) - 1));
}

#ifdef PARETO
time += Pareto(2.0, 100.0, 3.0);
#else
#endif