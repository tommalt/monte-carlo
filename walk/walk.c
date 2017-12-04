/*
 * walk.c
 * Tom Maltese
 */
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_randist.h>

#define N_MINUTES 10
#define ITERATIONS 10000
#define P_SLEEP 0.32       /* probability of falling asleep */
static const gsl_rng_type *T;
static gsl_rng *sleep_gen;
static gsl_rng *cardinal_gen;

int **cardinals_init();
void cardinals_destroy();
void run();
void render_stats(int **);
int icount_if(int*,size_t, int(*)(int));
double mean(int *, size_t);
int distp(int);

int main(int argc, char **argv)
{
	gsl_rng_env_setup();
	T = gsl_rng_default;
	sleep_gen    = gsl_rng_alloc(T);
	cardinal_gen = gsl_rng_alloc(T);
	int **cardinals = cardinals_init();
	run(cardinals);
	gsl_rng_free(sleep_gen);
	gsl_rng_free(cardinal_gen);
	cardinals_destroy(cardinals);
}
int **cardinals_init() {
	int **cardinals = malloc(ITERATIONS * sizeof *cardinals);
	int i;
	for (i = 0; i < ITERATIONS; i++) {
		cardinals[i] = calloc(4, sizeof(int));
	}
	return cardinals;
}
void cardinals_destroy(int **cardinals) {
	int i;
	for (i = 0; i < ITERATIONS; i++) {
		free(cardinals[i]);
	}
	free(cardinals);
}
/* let draw = random draw from uniform [0,100]. if:
 * draw <  25   		-> north
 * 25 <= draw < 50		-> east
 * 50 <= draw < 75		-> south
 * 75 <= draw < 100		-> west
 */
void run(int **cardinals) {
	int it, k;
	for (it = 0; it < ITERATIONS; it++) {
		/* he always walks east on the first minute */
		cardinals[it][1]++;
		for (k = 1; k < N_MINUTES; k++) {
			double sleep = gsl_ran_flat(sleep_gen, 0.0, 1.0);		
			if (sleep <= P_SLEEP) /* fell asleep, goto next minute */
				continue;
			/* which direction did he move? */
			int c = (int)gsl_ran_flat(cardinal_gen,0.0,100.0);
			cardinals[it][c/25]++;
		}
	}
	render_stats(cardinals);
}
/* P(within 2 blocks), Avg blocks walked, and Avg distance (manhattan) */
void render_stats(int **cardinals) {
	int it;
	int vhd[2];/* vertical and horizonal distances: vhd[0] = N|S, vhd[1] = W|E
		      (+) for North & East, (-) for South & West (like a cartesian plane) */
	int *man_distances = malloc(ITERATIONS * sizeof *man_distances);
	int *blocs_walked = malloc(ITERATIONS * sizeof *blocs_walked);
	for (it = 0; it < ITERATIONS; it++) {
		int *c = cardinals[it];
		vhd[0] = c[0] - c[2];
		vhd[1] = c[1] - c[4];
		man_distances[it] = vhd[0] + vhd[1];
		blocs_walked[it] = c[0] + c[1] + c[2] + c[3];
	}
	int less_2_blocs = icount_if(man_distances, ITERATIONS, &distp);
	double avg_walked = mean(blocs_walked, ITERATIONS);
	double avg_distance = mean(man_distances, ITERATIONS);
	
	printf("Iterations			: %d\n", ITERATIONS);
	printf("P(within two blocks from store) : %0.4f\n",
			(double)less_2_blocs/(double)ITERATIONS);
	printf("Average blocks walked           : %0.4f\n", avg_walked);
	printf("Average distance from store     : %0.4f\n", avg_distance);
}
int icount_if(int *array, size_t siz, int (*pred)(int)) {
	int i;
	int c = 0;
	for (i = 0; i < siz; i++) {
		if (pred(array[i]))
			c++;
	}
	return c;
}
double mean(int *array, size_t siz) {
	int acc = 0;
	int i;
	for (i = 0; i < siz; i++) {
		acc += array[i];
	}
	return (double)acc / (double)siz;
}
int distp(int x) {
	return x <= 2 ? 1 : 0;
}
