/**
 * Unterdrückt die Freigabe der Kugel, wenn sich die Umdrehungsgeschwindigkeit 
 * zwschen zwei Umdrehungen zu stark verändert.
 *
 * \ref lightSpeedCallback und \ref hallSpeedCallback müssen immer aufgerufen werden, wenn eine neue
 * Geschwindigskeitsmessung des jeweiligen Sensors breit steht.
 *
 * Die Callbackfunktion des Hallsensors wird verwendet, um zu bestimmen, wie lange die Freigabe der Kugel unterdrückt werden soll.
 * Die Callbackfunktion des Lichtsensors wird verwendet, um zu bestimmen, wann genau die Freigabe unterdrück werden soll. 
 * Da der Hallsensor lediglich halbe Umdrehungen der Platte wahrnehmen kann, ist dieser für die genaue Bestimmung ungeeignet. 
 * Nachdem durch den Lichtsensor ein zu großer Geschwindigkeitsunterschied zwischen zwei Runden erkannt wurde, wird die Freigabe der Kugel für zwei Hallsensorsignalewechsel, also eine Runde, unterdrückt.
 *
 */
class Inhibitor
{
  /// Prozentualer Zeitunterschied zwischen zwei Runden, der nicht überschritten werden darf.
  /// Wird dieser überschritten, wird die Freigabe unterdrückt.
  const double RELATIVE_INHIBITION_THRESHOLD = 0.08;
  /// Vorherige Umdrehungszeit
  unsigned long mLastTurnTime = 0;
  
  /** 
   Anzahl der Runden, für die die Freigabe der Kugel gesperrt ist
   */
  unsigned short mInhibitRounds = 0;
  
public:
  /**
   * Überprüfung ob der Schwellwert für die Unterdrückung der Freigabe, überschritten wurde.
   */
  void lightSpeedCallback(unsigned long turnTime)
  {
    if (fabs(static_cast<double> (mLastTurnTime) - static_cast<double> (turnTime)) / static_cast<double> (turnTime) > RELATIVE_INHIBITION_THRESHOLD) {
      debugprintln("=== inhibition");
      debugprint("last turn time: ");
      debugprintln(mLastTurnTime);
      debugprint("this turn time: ");
      debugprintln(turnTime);
      mInhibitRounds = 2;
    }
    mLastTurnTime = turnTime;
  }

  /**
   * Nach einer Runde: Verminderung der verbleibenenden Sperrrunden
   */
  void hallSpeedCallback(unsigned long)
  {
    if (mInhibitRounds > 0) {
      mInhibitRounds--;
    }
  }

  /**
   * Prüfung, ob Auslösung gerade gesperrt ist
   */
  bool isInhibited() const
  {
    return (mInhibitRounds != 0);
  }
};

