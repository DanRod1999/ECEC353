/* Vector dot product A.B using pthreads. This version shows how to use lock variables. 
 * 
 * Author: Naga Kandasamy
 * Date: April 4, 2011
 * Date modified: February 19, 2020
 *
 * Compile as follows: gcc -o vector_dot_product_v2 vector_dot_product_v2.c -std=c99 -Wall -lpthread -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

/* Shared data structure used by the threads */
typedef struct args_for_thread_t {
    int tid;                          /* The thread ID */
    int num_threads;                  /* Number of worker threads */
    int num_elements;                 /* Number of elements in the vectors */
    float *vector_a;                  /* Starting address of vector_a */
    float *vector_b;                  /* Starting address of vector_b */
    int offset;                       /* Starting offset for thread within the vectors */
    int chunk_size;                   /* Chunk size */
    double *sum;                      /* Location of the shared variable sum */
    pthread_mutex_t *mutex_for_sum;   /* Location of the lock variable protecting sum */
} ARGS_FOR_THREAD;

/* Function prototypes */
float compute_gold (float *, float *, int);
float compute_using_pthreads (float *, float *, int, int);
void *dot_product (void *);
void print_args (ARGS_FOR_THREAD *);

int 
main (int argc, char **argv)
{
    if (argc != 3) {
		printf ("Usage: %s num-elements num-threads \n", argv[0]);
		exit (EXIT_FAILURE);
	}
	
    int num_elements = atoi (argv[1]);  /* Obtain the size of the vector */
    int num_threads = atoi (argv[2]);   /* Obtain number of worker threads */

	/* Create the vectors A and B and fill them with random numbers between [-.5, .5] */
	float *vector_a = (float *) malloc (sizeof (float) * num_elements);
	float *vector_b = (float *) malloc (sizeof (float) * num_elements); 
	srand (time(NULL)); /* Seed the random number generator */
	for (int i = 0; i < num_elements; i++) {
		vector_a[i] = ((float) rand ()/(float) RAND_MAX) - 0.5;
		vector_b[i] = ((float) rand ()/(float) RAND_MAX) - 0.5;
	}

	/* Compute the dot product using the reference, single-threaded solution */
	struct timeval start, stop;	
	gettimeofday (&start, NULL);
	float reference = compute_gold (vector_a, vector_b, num_elements); 
	gettimeofday (&stop, NULL);

	printf ("Reference solution = %f\n", reference);
	printf ("Execution time = %fs\n", (float) (stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float) 1000000));
	printf ("\n");

	gettimeofday (&start, NULL);
	float result = compute_using_pthreads (vector_a, vector_b, num_threads, num_elements);
	gettimeofday (&stop, NULL);

	printf ("Pthread solution = %f\n", result);
	printf ("Execution time = %fs\n", (float) (stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float) 1000000));
	printf ("\n");

	/* Free memory */
	free ((void *) vector_a);
	free ((void *) vector_b);

	pthread_exit (NULL);
}

/* Compute the reference soution using a single thread. */
float 
compute_gold (float *vector_a, float *vector_b, int num_elements)
{
	double sum = 0.0;
	for (int i = 0; i < num_elements; i++)
			  sum += vector_a[i] * vector_b[i];
	
	return (float) sum;
}

/* Compute the dot product using multiple threads. This version uses mutex locks. */
float 
compute_using_pthreads (float *vector_a, float *vector_b, int num_threads, int num_elements)
{
    pthread_t *tid = (pthread_t *) malloc (sizeof (pthread_t) * num_threads); /* Data structure to store the thread IDs */
    if (tid == NULL) {
        perror ("malloc");
        exit (EXIT_FAILURE);
    }

    pthread_attr_t attributes;                  /* Thread attributes */
    pthread_mutex_t mutex_for_sum;              /* Lock for the shared variable sum */
    
    pthread_attr_init (&attributes);            /* Initialize the thread attributes to the default values */
    pthread_mutex_init (&mutex_for_sum, NULL);  /* Initialize the mutex */

    /* Allocate memory on the heap for the required data structures and create the worker threads */
    int i;
    double sum = 0; 
    ARGS_FOR_THREAD **args_for_thread;
    args_for_thread = malloc (sizeof (ARGS_FOR_THREAD) * num_threads);
    int chunk_size = (int) floor ((float) num_elements/(float) num_threads); // Compute the chunk size
    for (i = 0; i < num_threads; i++){
        args_for_thread[i] = (ARGS_FOR_THREAD *) malloc (sizeof (ARGS_FOR_THREAD));
        args_for_thread[i]->tid = i; 
        args_for_thread[i]->num_threads = num_threads;
        args_for_thread[i]->num_elements = num_elements; 
        args_for_thread[i]->vector_a = vector_a; 
        args_for_thread[i]->vector_b = vector_b; 
        args_for_thread[i]->offset = i * chunk_size; 
        args_for_thread[i]->chunk_size = chunk_size; 
        args_for_thread[i]->sum = &sum;
        args_for_thread[i]->mutex_for_sum = &mutex_for_sum;
    }

    for (i = 0; i < num_threads; i++)
        pthread_create (&tid[i], &attributes, dot_product, (void *) args_for_thread[i]);
					 
    /* Wait for the workers to finish */
    for(i = 0; i < num_threads; i++)
        pthread_join (tid[i], NULL);
		
    /* Free data structures */
    for(i = 0; i < num_threads; i++)
        free ((void *) args_for_thread[i]);
		
    return (float)sum;
}

/* This function is executed by each thread to compute the overall dot product */
void *
dot_product (void *args)
{
    ARGS_FOR_THREAD *args_for_me = (ARGS_FOR_THREAD *) args; /* Typecast the argument to a pointer the the ARGS_FOR_THREAD structure */
		  
    /* Compute the partial sum that this thread is responsible for */
    double partial_sum = 0.0; 
    if (args_for_me->tid < (args_for_me->num_threads - 1)) {
        for (int i = args_for_me->offset; i < (args_for_me->offset + args_for_me->chunk_size); i++)
            partial_sum += args_for_me->vector_a[i] * args_for_me->vector_b[i];
    } 
    else { /* This takes care of the number of elements that the final thread must process */
        for (int i = args_for_me->offset; i < args_for_me->num_elements; i++)
            partial_sum += args_for_me->vector_a[i] * args_for_me->vector_b[i];
		 
    }

    /* Accumulate partial sums into the shared variable */
    pthread_mutex_lock(args_for_me->mutex_for_sum);
    *(args_for_me->sum) += partial_sum;
    pthread_mutex_unlock(args_for_me->mutex_for_sum);

    pthread_exit ((void *)0);
}

/* Helper function for debugging purposes */
void 
print_args (ARGS_FOR_THREAD *args_for_thread)
{
    printf ("Thread ID: %d \n", args_for_thread->tid);
    printf ("Number of threads: %d \n", args_for_thread->num_threads);
    printf ("Num elements: %d \n", args_for_thread->num_elements); 
    printf ("Address of vector A on heap: %p \n", args_for_thread->vector_a);
    printf ("Address of vector B on heap: %p \n", args_for_thread->vector_b);
    printf ("Offset within the vectors for thread: %d \n", args_for_thread->offset);
    printf ("Chunk size to operate on: %d \n", args_for_thread->chunk_size);
    printf ("\n");
}
