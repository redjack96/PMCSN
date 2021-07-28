//
// Created by giaco on 13/04/2021.
// Servente singolo Bounded Pareto FIFO
//

/* -------------------------------------------------------------------------
 * This program is a next-event simulation of a single-server FIFO service
 * node using Exponentially distributed interarrival times and Uniformly
 * distributed service times (i.e., a M/U/1 queue).  The service node is
 * assumed to be initially idle, no arrivals are permitted after the
 * terminal time STOP, and the service node is then purged by processing any
 * remaining jobs in the service node.
 *
 * Name            : ssq3.c  (Single Server Queue, version 3)
 * Author          : Steve Park & Dave Geyer
 * Language        : ANSI C
 * Latest Revision : 10-19-98
 * -------------------------------------------------------------------------
 */

#include <stdio.h>
#include <math.h>
#include "rngs.h"

/* the multi-stream generator */

#define START         0.0              /* initial time                   */
// TODO: Aumentare stop per far andare avanti l'index dei job in arrivo fino a 50.000!!!
// #define STOP      200000.0              /* terminal (close the door) time */
// #define INFINITY   (100.0 * STOP)      /* must be much larger than STOP  */


#define BOUNDED_PARETO_P pow(10,10)

#define AVERAGE_SERVICE 3000.0

#define RHO 0.8


#define LAMBDA 3300 // (double) RHO/(double) AVERAGE_SERVICE
#define ALPHA_LIGHTTAIL 2.9
#define ALPHA_HEAVYTAIL 1.5

// #define VARIABILITY

#ifdef VARIABILITY
#define ALPHA ALPHA_HEAVYTAIL
#else
#define ALPHA ALPHA_LIGHTTAIL
#endif

#define BOUNDED_PARETO_K (double) (ALPHA - 1.0) / (ALPHA) * AVERAGE_SERVICE

#define MAX_JOBS 50000

double BoundedPareto(double alpha, double k, double p) {
    double u = Random();
    return k / pow(( 1 + u * (pow(k / p, alpha) - 1)), 1 / alpha);
}


/* ------------------------------
 * return the smaller of a, b
 * ------------------------------
 */
double Min(double a, double c)
{
    if (a < c)
        return (a);
    else
        return (c);
}


/* ---------------------------------------------------
 * generate an Exponential random variate, use m > 0.0
 * ---------------------------------------------------
 */
double Exponential(double m)
{
    return (-m * log(1.0 - Random()));
}


/** --------------------------------------------
 * generate a Uniform random variate, use a < b
 * --------------------------------------------
 */
double Uniform(double a, double b)
{
    return (a + (b - a) * Random());
}

/**
 * Gli arrivi sono esponenziali.
 * ---------------------------------------------
 * generate the next arrival time, with rate 1/LAMBDA
 * ---------------------------------------------
 */
double GetArrival() {
    static double arrival = START;

    SelectStream(0);
    arrival += Exponential(LAMBDA);
    return (arrival);
}


/** --------------------------------------------
 * generate the next service time with rate 2/3
 * --------------------------------------------
 */
double GetService()
{
    SelectStream(1);

    return BoundedPareto(ALPHA, BOUNDED_PARETO_K, BOUNDED_PARETO_P);
}


int main(void) {
    printf("lambda %6.2f\n", LAMBDA);
    printf("rho %6.2f\n", RHO);
    printf("service %6.2f\n", AVERAGE_SERVICE);
    printf("bounded pareto (alpha, k, p) = (%6.2f, %6.2f, 10^10)\n", ALPHA_LIGHTTAIL, BOUNDED_PARETO_K);


    struct {
        double arrival;                 /* next arrival time                   */
        double completion;              /* next completion time                */
        double current;                 /* current time                        */
        double next;                    /* next (most imminent) event time     */
        double last;                    /* last arrival time                   */
    } t;
    struct {
        double node;                    /* time integrated number in the node  */
        double queue;                   /* time integrated number in the queue */
        double service;                 /* time integrated number in service   */
    } area = {0.0, 0.0, 0.0};
    long index = 0;                  /* used to count departed jobs         */
    long number = 0;                  /* number in the node                  */

    PlantSeeds(123456789);
    t.current = START;           /* set the clock                         */
    t.arrival = GetArrival();    /* schedule the first arrival            */
    t.completion = INFINITY;        /* the first event can't be a completion */

    while (index < MAX_JOBS || (number > 0)) {
        t.next = Min(t.arrival, t.completion);  /* next event time   */
        if (number > 0) {                               /* update integrals  */
            area.node += (t.next - t.current) * number;
            area.queue += (t.next - t.current) * (number - 1);
            area.service += (t.next - t.current);
        }
        t.current = t.next;                    /* advance the clock */

        if (t.current == t.arrival) {               /* process an arrival */
            number++;
            t.arrival = GetArrival();
            // if(t.arrival > STOP)
            if (index + number > MAX_JOBS) {
                t.last = t.current;
                t.arrival = INFINITY;
            }
            if (number == 1)
                t.completion = t.current + GetService();
        } else {                                        /* process a completion */
            index++;
            number--;
            if (number > 0)
                t.completion = t.current + GetService();
            else
                t.completion = INFINITY;

            // if (index % 1000 == 0) printf("index %ld\n", index);
        }
        if(index == 50000) {
            // printf("Jobs rimasti: %ld\n", number);
            if(number == 0) break;
        }
        // if(number % 1000 == 0 || index % 1000 == 0)
            // printf("number %ld\n", number);
    }

    printf("\nfor %ld jobs\n", index);
    printf("\t1/lambda:\taverage interarrival time = %6.2f\n", t.last / index);
    printf("\tE[T]:\t\taverage wait ............ = %6.2f\n", area.node / index);
    printf("\tE[Tq]:\t\taverage delay ........... = %6.2f\n", area.queue / index);
    printf("\tE[S]:\t\taverage service time .... = %6.2f\n", area.service / index);
    printf("\tE[N]:\t\taverage # in the node ... = %6.2f\n", area.node / t.current);
    printf("\tE[Nq]:\t\taverage # in the queue .. = %6.2f\n", area.queue / t.current);
    printf("\trho:\t\tutilization ............. = %6.2f\n", area.service / t.current);

    return (0);
}
