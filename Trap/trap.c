/*  Purpose: Calculate definite integral using trapezoidal rule.
 *
 * Input:   a, b, n, num_threads
 * Output:  Estimate of integral from a to b of f(x)
 *          using n trapezoids, with num_threads.
 *
 * Compile: gcc -o trap trap.c -O3 -std=c99 -Wall -lpthread -lm
 * Usage:   ./trap
 *
 * Note:    The function f(x) is hardwired.
 *
 * Author: Naga Kandasamy
 * Date modified: February 21, 2020
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/time.h>
#include <pthread.h>

/* Shared data structure used by the threads */
typedef struct args_for_thread_t {
    int tid;                          /* The thread ID */
    int num_threads;                  /* Number of worker threads */
    float a;
    float b;
    int n;
    float h;
    double *integral;                      /* Location of the shared variable integral tot */
    pthread_mutex_t *mutex_for_integral;   /* Location of the lock variable protecting integral tot */
} ARGS_FOR_THREAD;

double compute_using_pthreads (float, float, int, float, int);
double compute_gold (float, float, int, float);
void * thread_int (void *);

int 
main (int argc, char **argv) 
{
    if (argc < 5) {
        printf ("Usage: %s lower-limit upper-limit num-trapezoids num-threads\n", argv[0]);
        printf ("lower-limit: The lower limit for the integral\n");
        printf ("upper-limit: The upper limit for the integral\n");
        printf ("num-trapezoids: Number of trapeziods used to approximate the area under the curve\n");
        printf ("num-threads: Number of threads to use in the calculation\n");
        exit (EXIT_FAILURE);
    }

    float a = atoi (argv[1]); /* Lower limit */
	float b = atof (argv[2]); /* Upper limit */
	float n = atof (argv[3]); /* Number of trapezoids */

	float h = (b - a)/(float) n; /* Base of each trapezoid */  
	printf ("The base of the trapezoid is %f\n", h);

	struct timeval start, stop;	
    
    gettimeofday (&start, NULL);
    double reference = compute_gold (a, b, n, h);
    gettimeofday (&stop, NULL);
    printf ("Reference solution computed using single-threaded version = %f\n", reference);
    printf ("Execution time = %fs\n", (float) (stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float) 1000000));

	/* Write this function to complete the trapezoidal rule using pthreads. */
    int num_threads = atoi (argv[4]); /* Number of threads */
    gettimeofday (&start, NULL);
	double pthread_result = compute_using_pthreads (a, b, n, h, num_threads);
    gettimeofday (&stop, NULL);
	printf ("Solution computed using %d threads = %f\n", num_threads, pthread_result);
    printf ("Execution time = %fs\n", (float) (stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float) 1000000));

    exit (EXIT_SUCCESS);
} 

/*------------------------------------------------------------------
 * Function:    f
 * Purpose:     Defines the integrand
 * Input args:  x
 * Output: sqrt((1 + x^2)/(1 + x^4))

 */
float 
f (float x) 
{
    return sqrt ((1 + x*x)/(1 + x*x*x*x));
}

/*------------------------------------------------------------------
 * Function:    compute_gold
 * Purpose:     Estimate integral from a to b of f using trap rule and
 *              n trapezoids using a single-threaded version
 * Input args:  a, b, n, h
 * Return val:  Estimate of the integral 
 */
double 
compute_gold (float a, float b, int n, float h) 
{
   double integral;
   int k;

   integral = (f(a) + f(b))/2.0;

   for (k = 1; k <= n-1; k++) 
     integral += f(a+k*h);
   
   integral = integral*h;

   return integral;
}  

/* FIXME: Complete this function to perform the trapezoidal rule using pthreads. */
double 
compute_using_pthreads (float a, float b, int n, float h, int num_threads)
{
	double integral = 0.0;

    pthread_t *tid = (pthread_t *) malloc (sizeof (pthread_t) * num_threads); /* Data structure to store the thread IDs */
    if (tid == NULL) {
        perror ("malloc");
        exit (EXIT_FAILURE);
    }

    pthread_attr_t attributes;                  /* Thread attributes */
    pthread_mutex_t mutex_for_integral;              /* Lock for the shared variable sum */
    
    pthread_attr_init (&attributes);            /* Initialize the thread attributes to the default values */
    pthread_mutex_init (&mutex_for_integral, NULL);  /* Initialize the mutex */

    /* Allocate memory on the heap for the required data structures and create the worker threads */
    int i;
    ARGS_FOR_THREAD **args_for_thread;
    args_for_thread = malloc (sizeof (ARGS_FOR_THREAD) * num_threads);
    for (i = 0; i < num_threads; i++){
        args_for_thread[i] = (ARGS_FOR_THREAD *) malloc (sizeof (ARGS_FOR_THREAD));
        args_for_thread[i]->tid = i; 
        args_for_thread[i]->num_threads = num_threads;
        args_for_thread[i]->n = n; 
        args_for_thread[i]->a = a; 
        args_for_thread[i]->b = b; 
        args_for_thread[i]->h = h; 
        args_for_thread[i]->integral = &integral;
        args_for_thread[i]->mutex_for_integral = &mutex_for_integral;
    }

    integral = (f(a) + f(b))/2.0;
    for (i = 0; i < num_threads; i++){
        pthread_create (&tid[i], &attributes, thread_int, (void *) args_for_thread[i]);
    }


    /* Wait for the workers to finish */
    for(i = 0; i < num_threads; i++)
        pthread_join (tid[i], NULL);
		
    integral = integral*h;
    /* Free data structures */
    for(i = 0; i < num_threads; i++)
        free ((void *) args_for_thread[i]);

    return integral;
}

void *
thread_int (void *args) 
{

    ARGS_FOR_THREAD *args_for_me = (ARGS_FOR_THREAD *) args;

    double part_integral = 0.0;
    int k;


    for (k = (args_for_me->tid)+1; k <= args_for_me->n-1; k += args_for_me->num_threads) 
        part_integral += f(args_for_me->a + k*args_for_me->h);

    // part_integral = part_integral*args_for_me->h;

    pthread_mutex_lock(args_for_me->mutex_for_integral);
    *(args_for_me->integral) += part_integral;
    pthread_mutex_unlock(args_for_me->mutex_for_integral);

    pthread_exit ((void *)0);
}
