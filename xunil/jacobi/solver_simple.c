/* Code for the Jacobi equation solver. 
 * Author: Naga Kandasamy
 * Date created: April 19, 2019
 * Date modified: February 21, 2020
 *
 * Compile as follows:
 * gcc -o solver_simple solver_simple.c solver_gold.c -O3 -Wall -std=c99 -lm -lpthread 
 *
 * If you wish to see debug info, add the -D DEBUG option when compiling the code.
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "grid.h"
#include <sys/time.h> 

extern int compute_gold (grid_t *);
grid_t * compute_using_pthreads_jacobi (grid_t *, int);
void compute_grid_differences(grid_t *, grid_t *);
grid_t *create_grid (int, float, float);
grid_t *copy_grid (grid_t *);
void print_grid (grid_t *);
void print_stats (grid_t *);
double grid_mse (grid_t *, grid_t *);
void *workerjob(void *);
double update (grid_t *, grid_t *, int, int);

/* Structure used to pass arguments to the worker threads */
typedef struct args_t {
    int tid;                        /* tid of the thread */
    int num_threads;                /* number of threads */
    int num_rows;                   /* number of rows we're solving */
    pthread_mutex_t *mutex;         /* Location of the lock variable protecting the grid */
    int num_iter;
} ARGS; 

struct timeval start, stop;	

//grids and globals
grid_t *gridA;                      /* one grid to either read or write from*/
grid_t *gridB;                      /* second grid */
int num_iter_threaded;              /* the number of iterations for the multi-threaded version */
int num_elements_threaded;
double bigdiff;
pthread_mutex_t mutex;              /* Lock for the shared variable diff */

int main (int argc, char **argv)
{
	if (argc < 5) {
        printf ("Usage: %s grid-dimension num-threads min-temp max-temp\n", argv[0]);
        printf ("grid-dimension: The dimension of the grid\n");
        printf ("num-threads: Number of threads\n"); 
        printf ("min-temp, max-temp: Heat applied to the north side of the plate is uniformly distributed between min-temp and max-temp\n");
        exit (EXIT_FAILURE);
    }
    
    /* Parse command-line arguments. */
    int dim = atoi (argv[1]); //grid dimension
    int num_threads = atoi (argv[2]);
    float min_temp = atof (argv[3]);
    float max_temp = atof (argv[4]);

    
    /* Generate the grids and populate them with initial conditions. */
 	grid_t *grid_1 = create_grid (dim, min_temp, max_temp);
    /* Grid 2 should have the same initial conditions as Grid 1. */
    grid_t *grid_2 = copy_grid (grid_1); 

    gridA = copy_grid (grid_2);
    gridB = copy_grid (grid_2);

	/* Compute the reference solution using the single-threaded version. */
	gettimeofday (&start, NULL);
	int num_iter = compute_gold (grid_1);
    gettimeofday (&stop, NULL);
    float time_1 = (float) (stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float) 1000000);
    int num_iter_1 = num_iter;


// #ifdef DEBUG
//     print_grid (grid_1);
// #endif
	
	/* Use pthreads to solve the equation using the jacobi method. */;

    gettimeofday (&start, NULL);
    grid_2 = compute_using_pthreads_jacobi (grid_2, num_threads);
    gettimeofday (&stop, NULL);

    
	printf ("\nUsing the single threaded version to solve the grid\n");
    printf ("Execution time = %fs\n", time_1);
    printf ("Convergence achieved after %d iterations\n", num_iter_1);
    /* Print key statistics for the converged values. */
	printf ("Printing statistics for the interior grid points\n");
    print_stats (grid_1);

	printf ("\nUsing pthreads to solve the grid using the jacobi method\n");
    printf ("Execution time = %fs\n", (float) (stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float) 1000000));
    printf ("Convergence achieved after %d iterations\n", num_iter_threaded);			
    printf ("Printing statistics for the interior grid points\n");
	print_stats (grid_2);





// #ifdef DEBUG
//     print_grid (grid_2);
// #endif
    
    /* Compute grid differences. */
    double mse = grid_mse (grid_1, grid_2);
    printf ("MSE between the two grids: %f\n", mse);

	/* Free up the grid data structures. */
	free ((void *) grid_1->element);	
	free ((void *) grid_1); 
	free ((void *) grid_2->element);	
	free ((void *) grid_2);

	exit (EXIT_SUCCESS);
}

/* FIXME: Edit this function to use the jacobi method of solving the equation. The final result should be placed in the grid data structure. */
grid_t * compute_using_pthreads_jacobi (grid_t *grid, int num_threads)
{

    int i,j;
    float eps = 1e-6; /* Convergence criteria. */
    int dim = grid->dim;
    int num_usable = dim*dim - (dim-1)*4;
    int done = 0;
    int iteration = 0;

    pthread_mutex_init (&mutex, NULL);  /* Initialize the mutex */	
    
    // Allocate memory for the IDs of the worker threads and their args
    pthread_t *worker_thread = (pthread_t *) malloc (num_threads * sizeof (pthread_t));
    ARGS **args_for_thread;
    args_for_thread = malloc (sizeof (ARGS*) * num_threads);

    while(!done) { /* While we have not converged yet. */
        bigdiff = 0.0;

        for (i = 0; i < num_threads; i++) {
                args_for_thread[i] = (ARGS *) malloc (sizeof (ARGS)); /* Memory for structure to pack the arguments */
                
                args_for_thread[i]->tid = i;
                args_for_thread[i]->num_threads = num_threads;
                args_for_thread[i]->num_rows = (int) floor ((float) (dim-2)/(float) num_threads);
                args_for_thread[i]->num_iter = iteration;
                args_for_thread[i]->mutex = &mutex;
                if ((pthread_create (&worker_thread[i], NULL, workerjob, (void *)args_for_thread[i])) != 0) {
                    perror ("Toop!");
                    exit (EXIT_FAILURE);
                }
        }
        for(i = 0; i < num_threads; i++)
            pthread_join (worker_thread[i], NULL);

        /* Free data structures */
        for(i = 0; i < num_threads; i++)
            free ((void *) args_for_thread[i]);

        iteration++;
        /* End of an iteration. Check for convergence. */
        bigdiff = bigdiff/num_elements_threaded;
        if (bigdiff < eps) 
            done = 1;
        printf ("Iteration: %d - DIFF: %f\n", iteration, bigdiff);
        bigdiff = 0;
        num_elements_threaded = 0;
    } 

    
    num_iter_threaded = iteration;
    if (num_iter_threaded %2 == 1){
        return gridA;
    } else {
        return gridB;
    }

}

void *workerjob(void *arg)
{
    ARGS *args_for_me = (ARGS *) arg; /* Typecast argument passed to function to appropriate type */
    int dim = gridA->dim;
    int i, j;

    double diff = 0.0;
    int num_ele = 0;
    
    //gotta find the start and end for our threads
    // int startI = 1 + args_for_me->tid * args_for_me->num_rows;
    // int endI = startI + args_for_me->num_rows-1;

    int iteration = args_for_me->num_iter;
    //if it's the last thread
    // if (args_for_me->tid == (args_for_me->num_threads - 1)) {
    //     endI = dim -2;
    // } 

    //For every other iteration, we want to swap the reading and writing grids.
    if (iteration % 2 == 1){  //odd iteration
        for (i = args_for_me->tid + 1; i < (gridA->dim - 1); i += args_for_me->num_threads) {
            for (j = 1; j < (gridA->dim - 1); j++) {
                diff +=update(gridA, gridB, i, j);
                num_ele++;
                //read from A, write to B
            }
        }
    }
    else { //even iteration
        for (i = args_for_me->tid + 1; i < (gridB->dim - 1); i += args_for_me->num_threads) {
            for (j = 1; j < (gridB->dim - 1); j++) {
                diff += update (gridB, gridA, i, j);
                num_ele++;
                //read from B, write to A
            }
        }
    }

    pthread_mutex_lock(&mutex);
    bigdiff = bigdiff + diff; /* Update the diff value. */
    num_elements_threaded = num_elements_threaded + num_ele;
    pthread_mutex_unlock(&mutex);

    pthread_exit (NULL);

}

double update (grid_t *Rgrid, grid_t *Wgrid, int i, int j){
    float old, new;
    double diff;

    old = Rgrid->element[i * Rgrid->dim + j]; /* Store old value of grid point. */
    /* Apply the update rule. */	
    new = 0.25 * (Rgrid->element[(i - 1) * Rgrid->dim + j] +\
                  Rgrid->element[(i + 1) * Rgrid->dim + j] +\
                  Rgrid->element[i * Rgrid->dim + (j + 1)] +\
                  Rgrid->element[i * Rgrid->dim + (j - 1)]);

    Wgrid->element[i * Wgrid->dim + j] = new; /* Update the grid-point value. */
    return diff = fabs(new - old); /* Calculate the difference in values. */

    // pthread_mutex_lock(&mutex);
    // bigdiff = bigdiff + diff; /* Update the diff value. */
    // pthread_mutex_unlock(&mutex);

}

/* Create a grid with the specified initial conditions. */
grid_t *create_grid (int dim, float min, float max)
{
    grid_t *grid = (grid_t *) malloc (sizeof (grid_t));
    if (grid == NULL)
        return NULL;

    grid->dim = dim;
	printf("Creating a grid of dimension %d x %d\n", grid->dim, grid->dim);
	grid->element = (float *) malloc (sizeof (float) * grid->dim * grid->dim);
    if (grid->element == NULL)
        return NULL;

    int i, j;
	for (i = 0; i < grid->dim; i++) {
		for (j = 0; j < grid->dim; j++) {
            grid->element[i * grid->dim + j] = 0.0; 			
		}
    }

    /* Initialize the north side, that is row 0, with temperature values. */ 
    srand ((unsigned) time (NULL));
	float val;		
    for (j = 1; j < (grid->dim - 1); j++) {
        val =  min + (max - min) * rand ()/(float)RAND_MAX;
        grid->element[j] = val; 	
    }

    return grid;
}

/* Creates a new grid and copies over the contents of an existing grid into it. */
grid_t *copy_grid (grid_t *grid) 
{
    grid_t *new_grid = (grid_t *) malloc (sizeof (grid_t));
    if (new_grid == NULL)
        return NULL;

    new_grid->dim = grid->dim;
	new_grid->element = (float *) malloc (sizeof (float) * new_grid->dim * new_grid->dim);
    if (new_grid->element == NULL)
        return NULL;

    int i, j;
	for (i = 0; i < new_grid->dim; i++) {
		for (j = 0; j < new_grid->dim; j++) {
            new_grid->element[i * new_grid->dim + j] = grid->element[i * new_grid->dim + j] ; 			
		}
    }

    return new_grid;
}

/* This function prints the grid on the screen. */
// void print_grid (grid_t *grid)
// {
//     int i, j;
//     for (i = 0; i < grid->dim; i++) {
//         for (j = 0; j < grid->dim; j++) {
//             printf ("%f\t", grid->element[i * grid->dim + j]);
//         }
//         printf ("\n");
//     }
//     printf ("\n");
// }


/* Print out statistics for the converged values of the interior grid points, including min, max, and average. */
void print_stats (grid_t *grid)
{
    float min = INFINITY;
    float max = 0.0;
    double sum = 0.0;
    int num_elem = 0;
    int i, j;

    for (i = 1; i < (grid->dim - 1); i++) {
        for (j = 1; j < (grid->dim - 1); j++) {
            sum += grid->element[i * grid->dim + j];

            if (grid->element[i * grid->dim + j] > max) 
                max = grid->element[i * grid->dim + j];

             if(grid->element[i * grid->dim + j] < min) 
                min = grid->element[i * grid->dim + j];
             
             num_elem++;
        }
    }
                    
    printf("AVG: %f\n", sum/num_elem);
	printf("MIN: %f\n", min);
	printf("MAX: %f\n", max);
	printf("\n");
}

/* Calculate the mean squared error between elements of two grids. */
double grid_mse (grid_t *grid_1, grid_t *grid_2)
{
    double mse = 0.0;
    int num_elem = grid_1->dim * grid_1->dim;
    int i;

    for (i = 0; i < num_elem; i++) 
        mse += (grid_1->element[i] - grid_2->element[i]) * (grid_1->element[i] - grid_2->element[i]);
                   
    return mse/num_elem; 
}
