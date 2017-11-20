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

void test();
void run(double stock_mean, double stock_stdev, double bond_mean,
	 double bond_stdev, int rebalance);

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
#ifdef DEBUG
	fprintf(stderr, "Running tests ...\n");
	test(); /* test to validate Knuth's method */
#endif		/* DEBUG */
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

/* `struct run_stat` represents a 'running' or 'rolling' statistic.
 * this enables us to compute the mean, variance, and stdev without allocating
 * any additional memory. This is due to Knuth, from The Art of Computer
 * Programming.
 */
struct run_stat {
	double mk, mk_1, vk, vk_1;
	int k;
};
double run_stat_mean(struct run_stat *s) { return s->mk; }
double run_stat_variance(struct run_stat *s) { return s->vk / (s->k - 1); }
double run_stat_stdev(struct run_stat *s) { return sqrt(run_stat_variance(s)); }

void run_stat_push(struct run_stat *s, double x)
{
	s->k++;
	if (s->k == 1) {
		s->mk = s->mk_1 = x;
		s->vk_1 = 0.0;
	} else {
		s->mk = s->mk_1 + (x - s->mk_1) / s->k;
		s->vk = s->vk_1 + (x - s->mk_1) * (x - s->mk);
		/* update for next iteration */
		s->mk_1 = s->mk;
		s->vk_1 = s->vk;
	}
}

/* confidence interval */
struct confint {
	double l;
	double r;
};
struct confint confidence_interval(double mean, double stdev, double n,
				   double p);
double uniform_draw();
double normal_draw(double mean, double stdev);

void run(double stock_mean, double stock_stdev, double bond_mean,
	 double bond_stdev, int rebalance)
{
	/* these persist accross iterations */
	struct run_stat acc = (struct run_stat){0, 0, 0, 0, 0};
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
		run_stat_push(&acc, amt);
		for (k = 0; k < 3; k++) {
			if (amt > acc_lvls[k])
				gtcnt[k]++;
		}
		amt = INITIAL_CAPITAL;
		stock_amt = amt * STOCK_ALLOC;
		bond_amt = amt * BOND_ALLOC;
	}
	double mean = run_stat_mean(&acc);
	double stdev = run_stat_stdev(&acc);
	struct confint confint =
	    confidence_interval(mean, stdev, ITERATIONS, CONFIDENCE_LEVEL);
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
	       mean, stdev);
	for (k = 0; k < 3; k++) {
		printf("P(acc > %.0f)     : %.2f\n", acc_lvls[k],
		       ((double)gtcnt[k]) / (double)ITERATIONS);
	}
	printf("Confidence interval : (%.0f,%.0f)\n", confint.l, confint.r);
}

/*
 * testing Knuth's method of computing the mean and standard deviation
 */
int approxeq(double approx, double exact)
{
	/* assert that approx is within (+/-)1% of exact */
	double err = 0.01;
	if ((approx < ((1 - err) * exact)) || (approx > ((1 + err) * exact)))
		return 0;
	return 1;
}
void test()
{
	fprintf(stderr, "Testing Knuth's method ... ");
	fflush(stderr);
	struct run_stat stock = (struct run_stat){0, 0, 0, 0, 0};
	struct run_stat bond = (struct run_stat){0, 0, 0, 0, 0};
	int n = 1000000;
	int i;
	for (i = 0; i < n; i++) {
		run_stat_push(&stock, normal_draw(STOCK_RETURNS_MEAN,
						  STOCK_RETURNS_STDEV));
		run_stat_push(
		    &bond, normal_draw(BOND_RETURNS_MEAN, BOND_RETURNS_STDEV));
	}
	assert(approxeq(run_stat_mean(&stock), 0.08) &&
	       approxeq(run_stat_stdev(&stock), 0.10));
	assert(approxeq(run_stat_mean(&bond), 0.04) &&
	       approxeq(run_stat_stdev(&bond), 0.04));
	fprintf(stderr, "OK.\n");
}
double normal_draw(double mean, double stdev)
{
	double uni = uniform_draw();
	return gsl_cdf_ugaussian_Pinv(uni) * stdev + mean;
}
double uniform_draw() { return gsl_ran_flat(r, 0.000001, 0.999999); }
struct confint confidence_interval(double mean, double stdev, double n,
				   double p)
{
	assert(p > 0 && p < 1 && "Confidence level out of range");
	double z = gsl_cdf_ugaussian_Pinv(p);
	double d = z * stdev / sqrt(n);
	struct confint c;
	c.l = mean - d;
	c.r = mean + d;
	return c;
}
