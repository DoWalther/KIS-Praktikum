/**
 * Release time calculation
 */
class TimeCalculation
{
/**
 * Time in µs the ball needs fall from the release mechanism until the hole
 * in the turntable
 * 
 * This includes the delay of the servo motor. The current value was determined
 * by testing values in the real system. A binary search was performed over the
 * value range [391 ms, 491 ms] to get the optimum.
 * 391 ms is the time the ball needs for falling (ignoring the servo motor)
 * derived by simple calculation from the fall distance.
 */
const long BALL_FALL_TIME_US = 391000UL + 27344UL;
  
public:
  /**
   * Calculate the time the next round of the turntable will approximately take
   * given the time the last round took
   * 
   * The next round time is approximated by a polynomial of degree two for turn
   * times below 2 s and a polynomial of degree four for times above 2 s.
   * The coefficients were determined by linear regression applied to actual measurement
   * data of the turntable round times.
   */
  unsigned long interpolateNextTurnTime(unsigned long lastTurnTime) const
  {
    double x = lastTurnTime;
    if (lastTurnTime < 500000UL) {
      const double a2 = 7.73699143114947e-9;
      const double a1 = 0.998685885;
      const double a0 = 268.2445651096;
      auto ret = a2 * pow(x, 2) + a1 * x + a0;
      return ret;
    } else if (lastTurnTime < 2000000UL) {
      const double a2 = 1.28978803947511e-8;
      const double a1 = 0.9910487474;
      const double a0 = 2585.5449062138;
      auto ret = a2 * pow(x, 2) + a1 * x + a0;
      return ret;
    } else {
      const double a4 = 3.10525846921924e-21;
      const double a3 = -3.96812534603555e-14;
      const double a2 = 2.19266935692185e-7;
      const double a1 = 0.5422132849;
      const double a0 = 343733.782110879;
      auto ret = a4 * pow(x, 4) + a3 * pow(x, 3) + a2 * pow(x, 2) + a1 * x + a0;
      return ret;
    }
  }

  /**
   * Calculate the time to wait for releasing the ball so it falls through the 
   * hole in the turntable given the current system state
   * 
   * \param lastTurnTime time in µs the last full round of the turntable took
   * \param timeInRound time in µs that has elapsed since the hole in the turntable
   *                    last passed the designated fall point
   * \return time in µs to wait until the ball should be released, negative in case
   *                    no calculation is possible
   */
  long operator()(unsigned long lastTurnTime, unsigned long timeInRound) const
  {
    // First check the input: Bail out if turn time is above 8 s, no meaningful
    // calculation is possible then
    if (lastTurnTime > 8000000UL) {
      return -1;      
    }
    
    // Save a timestamp on function entry to measure the time spent in this function
    auto t1 = micros();

    debugprintln("--");
    debugprint("time in round: ");
    debugprintln(timeInRound);

    // signed long because it is initially negative
    long fallTimeAcc = - timeInRound;
    // Start with the last known round time
    auto nextTurnTime = lastTurnTime;
    while (1) {
      debugprint("old time: ");
      debugprint(nextTurnTime);
      
      nextTurnTime = interpolateNextTurnTime(nextTurnTime);
      
      debugprint(" next turn time: ");
      debugprint(nextTurnTime);

      // Add current round time to the accumulated fall time
      fallTimeAcc += nextTurnTime;
      
      debugprint(" acc time: ");
      debugprintln(fallTimeAcc);
      
      if (fallTimeAcc >= BALL_FALL_TIME_US) {
        // Enough time accumulated so the ball can actually fall
        // -> calculate the difference to the fall time
        auto waitTime = fallTimeAcc - BALL_FALL_TIME_US;
        
        debugprint("-> waitTime: ");
        debugprintln(waitTime);

        // Calculate how long the calculation took
        auto t2 = micros();
        auto dif = t2 - t1;
        debugprint("calc time: ");
        debugprintln(dif);
        
        return waitTime;
      }
    }
  }
};
