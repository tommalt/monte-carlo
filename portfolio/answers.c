/*
 * Tom Maltese
 * Portfolio Simulation - Conclusion Questions
 *
 * (a) What is the mean and the standard deviation of
 * the accumulation?
 *
 * For the first scenario, the mean accumulation
 * is $284,464 and the standard deviation is $54,429
 *
 * (b) What is the probability that the accumulation is
 * over $240,000, $270,000, and $300,000?
 *
 * P(acc > $240,000) = 0.78
 * P(acc > $270,000) = 0.57
 * P(acc > $300,000) = 0.36
 *
 * (c) What is the accumulation if the standard deviations
 * are both zero? (fixed return)
 *
 * The accumulation is $283,920.
 *
 * (d) Can you compute a 95% confidence estimate of the
 * accumulation?
 *
 * Yes. For the first scenario, the confidence interval is
 * ($119,220 , $404,965) with a confidence level of 95%.
 * For the second scenario, it is exactly $283,920 (it does not
 * have an 'interval' because returns do not vary)
 *
 * (e) Given the limited information, there is not much we can
 * do other than simulate future price movements. That is not
 * to say that there aren't other methods of evaluating investments,
 * but they would be inappropriate for this particular problem, given
 * the limited data.
 *
 * ------------
 * EXTRA CREDIT
 * ------------
 * (f)
 *   (a) For the scenario where funds are rebalanced every year, the
 *   mean accumulation is $280,283 and the standard deviation is
 *   $50,576.
 *
 *   (b)
 *   P(acc > $240,000) = 0.78
 *   P(acc > $270,000) = 0.54
 *   P(acc > $300,000) = 0.33
 *   
 *   (c) The accumulation is $280,155
 *
 * (g)
 *   I expected the results of rebalancing the portfolio to result
 *   in a decrease in the standard deviation of accumulation.
 *
 *     Consider the case in which we do not rebalance the portfolio.
 *     Over time, it may occur that the returns due to equities surpass
 *     the returns due to bonds. We suspect this because equities have a
 *     higher rate of return. (note that the ROI is reinvested back into
 *     the same asset class).
 *
 *     Additionally, when each year comes to a close, we add our
 *     additional capital ($10k) to the portfolio by devoting
 *     70% ($7k) to stocks and 30% ($3k) to bonds.
 *     As you can see, the amount of money we have allocated
 *     to the equities in our portfolio will increase
 *     much faster than the amount we add to bonds.
 *
 *     The combination of these two properties will
 *     over expose us to the riskier asset (stocks)
 *     which will in turn lead to greater variability of
 *     our returns.
 *
 *  Now, if we rebalance our portfolio each year, such that 70% is
 *  allocated to equities and 30% is allocated to bonds, we will
 *  not be over exposed, and the potential losses we incur from
 *  equities are hedged by our bonds.
 *
 *  As expected, the standard deviation is lower when we rebalance
 *  our portfolio (it decreased from $54.4k to $50.6k). We should
 *  also note that the mean accumulation decreased in this scenario
 *  as well. This is likely due to the lower exposure to the riskier
 *  asset class, equity.
 */
