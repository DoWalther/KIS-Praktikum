/**
 * Berechnung des Auslösezeitpunktes
 */
class Time
{
/**
 * Zeit zwischen Auslösen und Landen der Kugel
 * aktueller Wert 
 */
const long BALL_FALL_TIME_US = 391000UL + 27344UL;
  
public:
  /**
   * Berechnet schätzungsweise die Dauer des folgenden Umlaufs der Scheibe
   * auf Basis des letzten gemessenen Umlaufs.
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
   * Berechnet den Wartezeit für den nächsten Zeitunkt des Abwurfs auf Basis des aktuellen Systemzustandes
   * 
   * lastTurnTime - Zeit der letzten Runde
   * timeInRound - Zeit seit letztem möglichen Wurfzeitpunktes
   * return - Zeit bis zum nächstmöglichen Wurfzeitpunkt
   */D
  long operator()(unsigned long lastTurnTime, unsigned long timeInRound) const
  {
	// über 8 Sekunden keine sinnvolle Berechnung möglichen
    if (lastTurnTime > 8000000UL) {
      return -1;      
    }
    
    auto t1 = micros();

    debugprintln("--");
    debugprint("time in round: ");
    debugprintln(timeInRound);
    long fallTimeAcc = - timeInRound;
    auto nextTurnTime = lastTurnTime;
    while (1) {
      debugprint("old time: ");
      debugprint(nextTurnTime);
      
      nextTurnTime = interpolateNextTurnTime(nextTurnTime);
      
      debugprint(" next turn time: ");
      debugprint(nextTurnTime);

      fallTimeAcc += nextTurnTime;
      
      debugprint(" acc time: ");
      debugprintln(fallTimeAcc);
      
      if (fallTimeAcc >= BALL_FALL_TIME_US) {
        auto waitTime = fallTimeAcc - BALL_FALL_TIME_US;
        
        debugprint("-> waitTime: ");
        debugprintln(waitTime);

        auto t2 = micros();
        auto dif = t2 - t1;
        debugprint("calc time: ");
        debugprintln(dif);
        
        return waitTime;
      }
    }
  }
};
