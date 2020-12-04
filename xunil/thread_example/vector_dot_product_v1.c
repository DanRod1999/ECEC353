/* Vector dot product A.B using pthreads. Version 1 
 * 
 * Author: Naga Kandasamy
 * Date created: 4/4/2011
 * Date modified: February 19, 2020
 * 
 * Compile as follows: gcc -o vector_dot_product_v1 vector_dot_product_v1.c -std=c99 -Wall -lpthread -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

/* Data structure passed to each thread */
typedef struct args_for_thread_t {
    int num_threads;        /* Number of worker threads */
    int tid;                /* The thread ID */
    int num_elements;       /* Number of elements in vector */
    float *vector_a;        /* Starting address of vector_a */
    float *vector_b;        /* Starting address of vector_b */
    int offset;             /* Starting offset for thread within the vectors */
    int chunk_size;         /* Chunk size */
    double *partial_sum;    /* Starting address of the partial_sum array */
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
		printf ("Usage: %s num-elements num-threads\n", argv[0]);
		exit (EXIT_FAILURE);
	}
	
    int num_elements = atoi (argv[1]); /* Size of vector */
    int num_threads = atoi (argv[2]); /* Number of worker threads */

	/* Create vectors A and B and fill them with random numbers between [-.5, .5] */
	float *vector_a = (float *) malloc (sizeof (float) * num_elements);
	float *vector_b = (float *) malloc (sizeof (float) * num_elements); 
	srand (time (NULL)); /* Seed random number generator */
	for (int i = 0; i < num_elements; i++) {
		vector_a[i] = ((float)rand ()/(float)RAND_MAX) - 0.5;
		vector_b[i] = ((float)rand ()/(float)RAND_MAX) - 0.5;
	}
	/* Compute dot product using the reference, single-threaded solution */
	struct timeval start, stop;	
	gettimeofday (&start, NULL);
	float reference = compute_gold (vector_a, vector_b, num_elements); 
	gettimeofday (&stop, NULL);

	printf ("Reference solution = %f\n", reference);
	printf ("Execution time = %fs\n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
	printf ("\n");

	gettimeofday (&start, NULL);
	float result = compute_using_pthreads (vector_a, vector_b, num_elements, num_threads);
	gettimeofday (&stop, NULL);

	printf ("Pthread solution = %f\n", result);
	printf ("Execution time = %fs\n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
	printf ("\n");

	/* Free memory */
	free ((void *)vector_a);
	free ((void *)vector_b);
    exit (EXIT_SUCCESS);
}

/* Function computes reference soution using a single thread. */
float 
compute_gold (float *vector_a, float *vector_b, int num_elements)
{
	double sum = 0.0;
	for (int i = 0; i < num_elements; i++)
			  sum += vector_a[i] * vector_b[i];
	
	return (float)sum;
}

/* This function computes dot product using multiple threads. */
float 
compute_using_pthreads (float *vector_a, float *vector_b, int num_elements, int num_threads)
{
    pthread_t *thread_id;                       /* Data structure to store the thread IDs */
    pthread_attr_t attributes;                  /* Thread attributes */
    pthread_attr_init(&attributes);             /* Initialize thread attributes to default values */
    int i;

    /* Allocate memory on heap for data structures and create worker threads */
    thread_id = (pthread_t *) malloc (sizeof (pthread_t) * num_threads);
    double *partial_sum = (double *) malloc (sizeof (double) * num_threads);
    ARGS_FOR_THREAD *args_for_thread = (ARGS_FOR_THREAD *) malloc (sizeof (ARGS_FOR_THREAD) * num_threads);
    
    int chunk_size = (int) floor ((float) num_elements/(float) num_threads); /* Compute chunk size */
		  
    for (i = 0; i < num_threads; i++) {
        args_for_thread[i].num_threads = num_threads;
        args_for_thread[i].tid = i; 
        args_for_thread[i].num_elements = num_elements; 
        args_for_thread[i].vector_a = vector_a; 
        args_for_thread[i].vector_b = vector_b; 
        args_for_thread[i].offset = i * chunk_size; 
        args_for_thread[i].chunk_size = chunk_size; 
        args_for_thread[i].partial_sum = partial_sum; 
    }
		  
    for (i = 0; i < num_threads; i++)
        pthread_create (&thread_id[i], &attributes, dot_product, (void *) &args_for_thread[i]);
					 
    /* Wait for workers to finish */
    for (i = 0; i < num_threads; i++)
        pthread_join (thread_id[i], NULL);
		 
    /* Accumulate partial sums computed by worker threads */ 
    double sum = 0.0; 
    for (i = 0; i < num_threads; i++)
        sum += partial_sum[i];

    /* Free data structures */
    free ((void *) thread_id);
    free ((void *) args_for_thread);
    free ((void *) partial_sum);

    return (float) sum;
}

/* Function executed by each thread to compute the overall dot product */
void *
dot_product (void *args)
{
    int i;
    ARGS_FOR_THREAD *args_for_me = (ARGS_FOR_THREAD *) args; /* Typecast argument to pointer to ARGS_FOR_THREAD structure */

    /* Compute partial sum that this thread is responsible for */
    double partial_sum = 0.0; 
    if (args_for_me->tid < (args_for_me->num_threads - 1)) {
        for (i = args_for_me->offset; i < (args_for_me->offset + args_for_me->chunk_size); i++)
            partial_sum += args_for_me->vector_a[i] * args_for_me->vector_b[i];
    } 
    else { /* Take care of number of elements that final thread must process */
        for (i = args_for_me->offset; i < args_for_me->num_elements; i++)
            partial_sum += args_for_me->vector_a[i] * args_for_me->vector_b[i];
    }

    /* Store partial sum into the partial_sum array */
    args_for_me->partial_sum[args_for_me->tid] = (float) partial_sum;
    pthread_exit ((void *) 0);
}

/* Helper function */
void 
print_args (ARGS_FOR_THREAD *args_for_thread)
{
    printf ("Thread ID: %d\n", args_for_thread->tid);
    printf ("Num elements: %d\n", args_for_thread->num_elements); 
    printf ("Address of vector A on heap: %p\n", args_for_thread->vector_a);
    printf ("Address of vector B on heap: %p\n", args_for_thread->vector_b);
    printf ("Offset within the vectors for thread: %d\n", args_for_thread->offset);
    printf ("Chunk size to operate on: %d\n", args_for_thread->chunk_size);
    printf ("\n");
    return;
}
