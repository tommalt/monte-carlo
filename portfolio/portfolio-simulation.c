/*
 * portfolio-simulation.c
 * Tom Maltese
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
/* GNU Scientific Library: for distribution functions, random number generation,
 * etc. */
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_sys.h>

#define INITIAL_CAPITAL 27000
#define ADDITIONAL_CAPITAL 10000

#define STOCK_ALLOC 0.70
#define BOND_ALLOC 0.30

#define STOCK_RETURNS_MEAN 0.08
#define STOCK_RETURNS_STDEV 0.10
#define BOND_RETURNS_MEAN 0.04
#define BOND_RETURNS_STDEV 0.04

/* simulation parameters */
#define NYEARS 14
#define ITERATIONS 1000
#define CONFIDENCE_LEVEL 0.95

/* random number generation */
static const gsl_rng_type *T;
static gsl_rng *r;

void run(double stock_mean, double stock_stdev, double bond_mean,
	 double bond_stdev, int rebalance);

int sort_comp(void const *l, void const *r) {
	double *ld = (double*) l;
	double *rd = (double*) r;
	if (*ld < *rd) {
		return -1;
	} else if (*ld > *rd) {
		return 1;
	}
	return 0;
}
/*
 * here, we run the simulation four times:
 * once under normal conditions
 * once with zero standard deviations (fixed return)
 * once with the funds rebalanced each year
 * and once with funds rebalance and zero standard deviations
 */
int main(int argc, char **argv)
{
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	printf("Scenario: no rebalance, normal parameters\n");
	run(STOCK_RETURNS_MEAN, STOCK_RETURNS_STDEV, BOND_RETURNS_MEAN,
	    BOND_RETURNS_STDEV, 0);

	printf("\nScenario: no rebalance, standard deviation = 0\n");
	run(STOCK_RETURNS_MEAN, 0, BOND_RETURNS_MEAN, 0, 0);

	printf("\nScenario: portfolio rebalanced\n");
	run(STOCK_RETURNS_MEAN, STOCK_RETURNS_STDEV, BOND_RETURNS_MEAN,
	    BOND_RETURNS_STDEV, 1);

	printf("\nScenario: portfolio rebalanced, standard deviation = 0\n");
	run(STOCK_RETURNS_MEAN, 0, BOND_RETURNS_MEAN, 0, 1);

	gsl_rng_free(r);
	return 0;
}

/* confidence interval */
struct confint {
	double l;
	double r;
};

/* note to self: should we use n or n-1 for the mean & variance computation(?) */
double mean(double *data, int data_siz) {
	double t = 0.0;
	int i;
	for (i = 0; i < data_siz; i++) {
		t += data[i];
	}
	return t / ((double)data_siz);
}
double stdev(double *data, int data_siz, double mu) {
	double var = 0.0;
	int i;
	for (i = 0; i < data_siz; i++) {
		var += pow((data[i] - mu), 2.0);
	}
	var = var / ((double)data_siz);
	return sqrt(var);
}
struct confint confidence_interval(double *data, int data_siz, double conf_level);
double uniform_draw();
double normal_draw(double mean, double stdev);

void run(double stock_mean, double stock_stdev, double bond_mean,
	 double bond_stdev, int rebalance)
{
	/* these persist accross iterations */
	double acc[ITERATIONS];
	double acc_lvls[] = {240000, 270000, 300000};
	int gtcnt[] = {0, 0, 0};

	/* temporaries; reset after each iteration */
	double amt = INITIAL_CAPITAL;
	double stock_amt = amt * STOCK_ALLOC;
	double bond_amt = amt * BOND_ALLOC;
	int i, j, k;

	for (i = 0; i < ITERATIONS; i++) {
		for (j = 0; j < NYEARS; j++) {
			stock_amt = stock_amt *
				    (1 + normal_draw(stock_mean, stock_stdev));
			bond_amt =
			    bond_amt * (1 + normal_draw(bond_mean, bond_stdev));
			amt = stock_amt + bond_amt;
			if (!rebalance) {
				stock_amt = stock_amt +
					    (STOCK_ALLOC * ADDITIONAL_CAPITAL);
				bond_amt = bond_amt +
					   (BOND_ALLOC * ADDITIONAL_CAPITAL);
			} else {
				stock_amt =
				    STOCK_ALLOC * (amt + ADDITIONAL_CAPITAL);
				bond_amt =
				    BOND_ALLOC * (amt + ADDITIONAL_CAPITAL);
			}
		}
		/* record results and continue */
		acc[i] = amt;
		for (k = 0; k < 3; k++) {
			if (amt > acc_lvls[k])
				gtcnt[k]++;
		}
		/* reset for next iteration */
		amt = INITIAL_CAPITAL;
		stock_amt = amt * STOCK_ALLOC;
		bond_amt = amt * BOND_ALLOC;
	}
	double mu = mean(acc, ITERATIONS);
	double sd = stdev(acc, ITERATIONS, mu);
	qsort(acc, ITERATIONS, sizeof *acc, &sort_comp);
	struct confint confint = confidence_interval(acc, ITERATIONS, CONFIDENCE_LEVEL);
	printf("\tParameters\n"
	       "Mean stock return   : %0.2f\n"
	       "Stdev stock returns : %0.2f\n"
	       "Mean bond return    : %0.2f\n"
	       "Stdev bond returns  : %0.2f\n"
	       "Rebalance portfolio : %s\n",
	       stock_mean, stock_stdev, bond_mean, bond_stdev,
	       ((rebalance) ? "true" : "false"));
	printf("\tOutputs\n"
	       "Mean accumulation   : $%0.0f\n"
	       "Stdev accumulation  : $%0.0f\n",
	       mu, sd);
	for (k = 0; k < 3; k++) {
		printf("P(acc > %.0f)     : %.2f\n", acc_lvls[k],
		       ((double)gtcnt[k]) / (double)ITERATIONS);
	}
	printf("Confidence interval : (%.0f,%.0f)\n", confint.l, confint.r);
}
double normal_draw(double mean, double stdev)
{
	double uni = uniform_draw();
	return gsl_cdf_ugaussian_Pinv(uni) * stdev + mean;
}
double uniform_draw() { return gsl_ran_flat(r, 0.000001, 0.999999); }
struct confint confidence_interval(double *data, int data_siz, double conf_level)
{
	double alpha = (1.0 - conf_level) / 2.0;
	int cutoff = data_siz * alpha;
	struct confint c;
	c.l = data[cutoff];
	c.r = data[data_siz-cutoff-1];
	return c;
}
