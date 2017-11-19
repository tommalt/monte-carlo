#include <assert.h>
#include <math.h>
#include <random>
#include <stdio.h>

#define NORMDIST_SEED 0x1000
#define ARRIVAL_SEED 0x0
#define ARRIVAL_MIN 0
#define ARRIVAL_MAX 5
#define SERVICE_MEAN 2
#define SERVICE_STDEV 0.5

double norm_cdf(double z);
double rational_approx(double t);
double norm_inv(double p);
void run(int n, int discard);
double normal_draw(double mu, double sd);
int main(int argc, char **argv)
{
	run(1116, 100);
	return 0;
}

/*
 * STATE VARIABLES
 * inter_arriv  -> inter arrival time for this iteration (relative time)
 * ariv		-> arrival time 			 (absolute time)
 * serv_start 	-> the start time of service 		 (absolute time)
 * tq		-> time waiting in queue 		 (relative time)
 * proc		-> processing time 			 (relative time)
 * fin		-> completion time 			 (absolute time)
 * tsys		-> time in system			 (relative time)
 * SUMMARY STATISTICS
 * nwait	-> the total # of customers that had to wait
 * nwait_1m	-> .. .. ..			.. ..	 more than one minute
 * pwait	-> the Probability a customer had to wait
 * pwait_1m	-> the Probability of waiting more than one minute
 * avg_tq	-> average time in queue
 * max_tq	-> maximum waiting time
 * util		-> utilization of resource (total service time / total time elapsed)
 */
void run(int n, int discard)
{
	double inter_arriv = 0, arriv = 0, serv_start = 0,
		tq = 0, proc = 0, fin = 0, tsys = 0;
	double nwait = 0, nwait_1m = 0, pwait = 0, pwait_1m = 0,
		avg_tq = 0, max_tq = 0, util = 0;

	/* random number generator (uniform dist) */
	std::mt19937 engine(ARRIVAL_SEED);
	std::uniform_real_distribution<double> draw_interarrival(ARRIVAL_MIN, ARRIVAL_MAX);
	
	serv_start = arriv = inter_arriv = draw_interarrival(engine);
	tq = serv_start - arriv;
	proc = normal_draw(SERVICE_MEAN, SERVICE_STDEV);
	fin = serv_start + proc;
	tsys = fin - arriv;
	int i;
	#ifdef DEBUG
	printf("inter_arriv, ariv, serv_start, tq, proc, fin, tsys\n");
	#endif
	for (i = 1; i < n; i++) {
		inter_arriv = draw_interarrival(engine);
		arriv += inter_arriv;
		serv_start = fmax(arriv, fin);
		tq = serv_start - arriv;
		proc = normal_draw(SERVICE_MEAN, SERVICE_STDEV);
		fin = serv_start + proc;
		tsys = fin - arriv;
		if (discard < i) {
			nwait += (tq > 0.0);
			nwait_1m += (tq > 1.0);
			avg_tq += tq;
			max_tq = (tq > max_tq) ? tq : max_tq;
			util += proc;
		}
		#ifdef DEBUG
		// printf("%3.2f %6.2f %9.2f %12.2f %15.2f %18.2f %21.2f\n",
		printf("%8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f\n",
			inter_arriv, arriv, serv_start, tq, proc,fin,tsys);
		#endif /* DEBUG */
	}
	pwait = nwait / (double)(n - discard);
	pwait_1m = nwait_1m / (double)(n - discard);
	avg_tq /= (double)(n - discard);
	util /= (double)(fin);

	printf("*** SUMMARY ***\n");

	printf("Number waiting: %.0f\n", nwait);
	printf("Probability of waiting: %.2f\n", pwait);
	printf("Average waiting time: %.2f\n", avg_tq);
	printf("Maximum waiting time: %.2f\n", max_tq);
	printf("Utilization: %.2f\n", util);
	printf("Number waiting > 1 minute: %.2f\n", nwait_1m);
	printf("Probability wait > 1 minute: %.2f\n", pwait_1m);
}

double norm_cdf(double z) { return 0.5 * erfc(-z * M_SQRT1_2); }

double rational_approx(double t)
{
	static const double c[] = { 2.515517, 0.802853, 0.010328 };
	static const double d[] = { 1.432788, 0.189269, 0.001308 };
	return t - ((c[2]*t + c[1])*t + c[0]) /
			(((d[2]*t + d[1])*t + d[0])*t + 1.0);
}
double norm_inv(double p)
{
	assert(p > 0.0 && p < 1.0 && "p out of domain");
	if (p <= 0.5)
		return -rational_approx(sqrt(-2.0 * log(p)));
	return rational_approx(sqrt(-2.0 * log(1 - p)));
}

double normal_draw(double mu, double sd)
{
	static std::mt19937 engine(NORMDIST_SEED);
	static std::uniform_real_distribution<double> uni(0.000001,0.999999);

	return (norm_inv(uni(engine)) * sd) + mu;
}

