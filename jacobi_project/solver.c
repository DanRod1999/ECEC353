/* Code for the Jacobi equation solver. 
 * Authors: Caleb Secor and Darius Olega
 * 
 * Date modified: March 26, 2020
 *
 * Compile as follows:
 * gcc -o solver solver.c solver_gold.c -O3 -Wall -std=c99 -lm -lpthread
 * OR
 * make build
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
#include <sys/time.h>
#include "grid.h" 

/* function declaration */
extern int compute_gold (grid_t *);
grid_t * compute_using_pthreads_jacobi (grid_t *, int);
void compute_grid_differences(grid_t *, grid_t *);
grid_t *create_grid (int, float, float);
grid_t *copy_grid (grid_t *);
void print_grid (grid_t *);
void print_stats (grid_t *, int);
double grid_mse (grid_t *, grid_t *);
void pthreads_solver(void*);
void print_single_thread_file(void);

/* Structure that holds the arguments for thread function */
typedef struct args_for_thread_s {
    int tid;        /* Thread ID */
    int num_iter;
    int num_threads;
} ARGS_FOR_THREAD_t;

/* Global variables for all threads */
grid_t *grid_multi_0;
grid_t *grid_multi_1;
double diff_multi;
int num_elements_multi, num_iter_multi;
pthread_mutex_t mutex1;

int 
main (int argc, char **argv)
{	
    /* Check argument input */
	if (argc < 5) {
        printf ("Usage: %s grid-dimension num-threads min-temp max-temp\n", argv[0]);
        printf ("OR: make run grid-dimension=[va1] num-threads=[val2] min-temp=[val3] max-temp=[val4]\n");
        printf ("grid-dimension: The dimension of the grid\n");
        printf ("num-threads: Number of threads\n"); 
        printf ("min-temp, max-temp: Heat applied to the north side of the plate is uniformly distributed between min-temp and max-temp\n");
        exit (EXIT_FAILURE);
    }

    /* Save results of the Single thread ouput to this file */
    FILE * output;
    output = fopen("single_thread_output", "w");

    /* variable used to track function execution time */
    struct timeval start, stop;
    double time_taken;

    /* Parse command-line arguments. */
    int dim = atoi (argv[1]);
    int num_threads = atoi (argv[2]);
    float min_temp = atof (argv[3]);
    float max_temp = atof (argv[4]);
    
    /* Generate the grids and populate them with initial conditions. */
 	grid_t *grid_1 = create_grid (dim, min_temp, max_temp);
    /* Grid 2 should have the same initial conditions as Grid 1. */
    grid_t *grid_2 = copy_grid (grid_1);
    /* copy grid 2 to global grids for multithread computation */
    grid_multi_0 = copy_grid (grid_2); 
    grid_multi_1 = copy_grid (grid_2); 

	/* Compute the reference solution using the single-threaded version. */
	printf ("\nUsing the single threaded version to solve the grid\n");

    gettimeofday (&start, NULL); /* Start timer */
	int num_iter = compute_gold (grid_1);
    gettimeofday (&stop, NULL); /* End timer */
    time_taken = (double)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(double)1000000); /* Compute time taken */

    /* Print key statistics for single thread computation to ouput file. */
	fprintf (output, "Solution computed using single thread in: %fs\n", time_taken);
	fprintf (output,"Convergence achieved after %d iterations\n", num_iter);
    fprintf (output, "Statistics for the interior grid points:\n");
	printf ("Statistics for the interior grid points:\n");
    fclose(output);

    print_stats (grid_1, 1);
#ifdef DEBUG
    print_grid (grid_1);
#endif
	
	/* Use pthreads to solve the equation using the jacobi method. */
	printf ("\nUsing pthreads to solve the grid using the jacobi method\n");
    gettimeofday (&start, NULL); /* Start timer */
	grid_2 = compute_using_pthreads_jacobi (grid_2, num_threads);
    gettimeofday (&stop, NULL); /* End timer */
    time_taken = (double)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(double)1000000); /* Compute time taken */

    /* print single thread ouputfile */
    printf("\nResults from single thread computation:\n");
    print_single_thread_file();
    /* Print key statistics for multi-thread computation to stdout */
    printf("Results from multi-thread computation:\n");
	printf ("Solution computed using %d thread in: %fs\n", num_threads, time_taken);
    printf ("Convergence achieved after %d iterations\n", num_iter_multi);			
    printf ("Statistics for the interior grid points:\n");
	print_stats (grid_2, 0);

#ifdef DEBUG
    print_grid (grid_2. 0);
#endif
    
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

/*------------------------------------------------------------------
 * Function:    compute_using_pthreads_jacobi
 * Purpose:     Create threads and split the work evenly among them,
 *              set input in args_for_thread stuct and call pthread solver
 *              repead until convergence is at required amount
 *              
 * Input args:  *grid, num_threads
 * Return val:  grid_t* 
 */  /*  */
grid_t * 
compute_using_pthreads_jacobi (grid_t *grid, int num_threads)
{
    /* verify input arguments */
    if (num_threads < 2){
        printf("You only chose one thread, Multi-Thread can't be done!\n");
        return 0;
    }
    int num_iter = 0;
	int done = 0;
    float eps = 1e-6;
    pthread_t *worker_thread = (pthread_t *) malloc (num_threads * sizeof (pthread_t));
    diff_multi = 0;
    num_elements_multi =0;

    /* repeat until convergence is achieved */
    while(!done){

        /* load arguments into thread */
        for (int i=0; i< num_threads; i++){
            ARGS_FOR_THREAD_t *args_for_thread = malloc(sizeof(ARGS_FOR_THREAD_t));
            args_for_thread->tid = i;
            args_for_thread->num_iter = num_iter;
            args_for_thread->num_threads = num_threads;

            /* create thread */
            if ((pthread_create (&worker_thread[i], NULL, (void*)pthreads_solver, (void *)args_for_thread)) != 0) {
                perror ("pthread_create");
                exit (EXIT_FAILURE);
            }
        }
        /* wait for all threads to finish */
        for (int i = 0; i < num_threads; i++){
            pthread_join (worker_thread[i], NULL);
        }
        
        /* reset global variables, test for convergence */
        double test = diff_multi/num_elements_multi;
        num_iter++;
        diff_multi = 0.0;
        num_elements_multi = 0;
        if (test < eps) 
            done = 1;

        printf ("Iteration: %d - DIFF: %f\n", num_iter, test);
    }
    /* return newest grid value when convergence is achieved */
    num_iter_multi = num_iter;
    if (num_iter %2 == 1){
        return grid_multi_0;
    } else {
        return grid_multi_1;
    }
}

/*------------------------------------------------------------------
 * Function:    pthreads_solver
 * Purpose:     Calculate the value for every element in the grid in a multi-thread fashion 
 *              each thread processes a different row, they read from one grid and write to another
 *              switch between grids so the newest one is always being read from
 *              
 * Input args:  this_arg (thread argument structure)
 * Return val:  none 
 */  /*  */
void
pthreads_solver(void* this_arg)
{
    ARGS_FOR_THREAD_t *args_for_me = (ARGS_FOR_THREAD_t *) this_arg;

    int i, j;
	double diff = 0.0;
	float old, new; 
    int num_elements = 0; 
    int num_iter = args_for_me->num_iter;

    if (num_iter %2 == 1){
        for (i = 1 + args_for_me->tid; i < (grid_multi_0->dim - 1); i += args_for_me->num_threads) { /* each thread processes a different row */
            for (j = 1; j < (grid_multi_0->dim - 1); j++) {
                old = grid_multi_0->element[i * grid_multi_0->dim + j]; /* Store old value of grid point from read grid. */
                /* Apply the update rule. */	
                new = 0.25 * (grid_multi_0->element[(i - 1) * grid_multi_0->dim + j] +\
                                grid_multi_0->element[(i + 1) * grid_multi_0->dim + j] +\
                                grid_multi_0->element[i * grid_multi_0->dim + (j + 1)] +\
                                grid_multi_0->element[i * grid_multi_0->dim + (j - 1)]);

                grid_multi_1->element[i * grid_multi_1->dim + j] = new; /* Update the grid-point value on write grid. */
                diff = diff + fabs(new - old); /* Calculate the difference in values. */
                num_elements++;
            }
        }
    } else { /* switch read and write grids */
        for (i = 1 + args_for_me->tid; i < (grid_multi_1->dim - 1); i += args_for_me->num_threads) {
            for (j = 1; j < (grid_multi_1->dim - 1); j++) {
                old = grid_multi_1->element[i * grid_multi_1->dim + j]; /* Store old value of grid point from read grid. */
                /* Apply the update rule. */	
                new = 0.25 * (grid_multi_1->element[(i - 1) * grid_multi_1->dim + j] +\
                                grid_multi_1->element[(i + 1) * grid_multi_1->dim + j] +\
                                grid_multi_1->element[i * grid_multi_1->dim + (j + 1)] +\
                                grid_multi_1->element[i * grid_multi_1->dim + (j - 1)]);

                grid_multi_0->element[i * grid_multi_0->dim + j] = new; /* Update the grid-point value from write grid. */
                diff = diff + fabs(new - old); /* Calculate the difference in values. */
                num_elements++;
            }
        }  
    }
    /* add the values of a threads together */
    pthread_mutex_lock(&mutex1);
    diff_multi += diff;
    num_elements_multi += num_elements;
    pthread_mutex_unlock(&mutex1);
    return;

}


/* Create a grid with the specified initial conditions. */
grid_t * 
create_grid (int dim, float min, float max)
{
    grid_t *grid = (grid_t *) malloc (sizeof (grid_t));
    if (grid == NULL)
        return NULL;

    grid->dim = dim;
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
grid_t *
copy_grid (grid_t *grid) 
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
void 
print_grid (grid_t *grid)
{
    int i, j;
    for (i = 0; i < grid->dim; i++) {
        for (j = 0; j < grid->dim; j++) {
            printf ("%f\t", grid->element[i * grid->dim + j]);
        }
        printf ("\n");
    }
    printf ("\n");
}


/* Print out statistics for the converged values of the interior grid points, including min, max, and average. */
void 
print_stats (grid_t *grid, int output_file)
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

    if (output_file == 1){
        /* print to single thread ouput file */
        FILE * output;
        output = fopen("single_thread_output", "a");
        fprintf(output, "AVG: %f\n", sum/num_elem);
        fprintf(output, "MIN: %f\n", min);
        fprintf(output, "MAX: %f\n", max);
        fprintf(output,"\n");
        fclose(output);
    }
}


/* Calculate the mean squared error between elements of two grids. */
double
grid_mse (grid_t *grid_1, grid_t *grid_2)
{
    double mse = 0.0;
    int num_elem = grid_1->dim * grid_1->dim;
    int i;

    for (i = 0; i < num_elem; i++) 
        mse += (grid_1->element[i] - grid_2->element[i]) * (grid_1->element[i] - grid_2->element[i]);
                   
    return mse/num_elem; 
}

/* print the contents of single_thread ouput file */
void
print_single_thread_file(void)
{
    void* status;
    char input[255];
    FILE * fp;
    fp = fopen("single_thread_output", "r");

    if(fp == NULL){
        printf("Can not open file sing;e_thread_ouput file\n");
    }else{
        do
        {
            status = fgets(input, sizeof(input), fp);

            printf("%s", input);
        }while((int*)status);
    }
    fclose(fp);
    return;
}

