/**
 * Aufzeichnung der Dauer eines high/low Zyklus eines Pins.
 * 
 * \ref setup und \ref loop müssen in ihren jeweilgen Arduino Gegenstück aufgerufen werden.
 * 
 * Die überprüften Daten können mit \ref turnTime gepullt oder optional zu einer callback 
 * Funktion mit \ref setCallback gepusht werden.
 */
class Observer
{
  /// ID des überwachten Pins.
  const int mPin;

  /// Zeit des letzten übergangs.
  unsigned long mLastTime = 0;
  /// Zustand des Pins, der als letztes aufgezeichnet wurde.
  bool mLastState = 0;
  /// Zyklusdauer
  unsigned long mTurnTime = 0;

  /// Faktor, mit dem die Zyklusdauer multipliziert wird um \ref mTurnTime zu erhalten.
  unsigned short mFactor;
  /// Ob komplette Zyklen (high/low transitionen) oder halbe Zyklen (low -> high und high -> low) gemessen werden sollen.
  /// Halbe Zyklen sollten nur gemmesn werden, wenn der Arbeitszyklus ca. 50% beträgt.
  bool mMeasureHalfCycles;

public:
  typedef void (*CallbackType)(unsigned long);
  
private:
  /// Funktion die aufgerufen wird, wenn eine neue Messung bereit steht.
  CallbackType mCallback = nullptr;
  
public:
  /**
   * Instanziierung eines neuen SpeedMontors.
   *
   *\param pin ID des überwachten Pins.
   *\param factor Faktor mit dem die Zyklusdauer multipliziert wird, um die Zeit für eine volle Umdrehung zu erhalten.
   *\measureHalfCycles Ob komplette Zyklen (high/low transitionen) oder halbe Zyklen (low -> high und high -> low) gemessen werden sollen.
   */
  Observer(int pin, unsigned short factor = 1, bool measureHalfCycles = false)
  : mPin(pin), mFactor(factor), mMeasureHalfCycles(measureHalfCycles)
  {}

  /**
   * Einstellung des Monitors.
   */
  void setup()
  {
    pinMode(mPin, INPUT);
  }

  /**
   *
   * Schleife die durch die Arduino Hauptschleife aufgerufen wird.
   *
   * Überwacht den Pin, nach Zustandsübergängen und misst die Zeit zwischen diesen.
   */
  void loop()
  {
    int state = digitalRead(mPin);
    
    if (state != mLastState) {
      /// Zustandsübergang aufgetreten.
      /// Wird nur bei high -> low Übergängen oder kompletten Zyklen getriggert. 
      if (mMeasureHalfCycles || !state) {
        unsigned long now = micros();
        unsigned long dif = now - mLastTime;
        mTurnTime = dif * mFactor;
        mLastTime = now;
        if (mCallback) {
          mCallback(mTurnTime);
        }
      }
      mLastState = state;
    }
  }

  /**
   * Setzt eine Funktion die aufgerufen wird, wenn eine neue Messung bereit steht.
   */ 
  void setCallback(CallbackType callback)
  {
    mCallback = callback;
  }

  /**
   * Gibt die letzte Zyklusmessung zurück.
   * \return turnTime (Letzte Zykluszeit multipliziert mit \ref mFactor) in µs
   */
  unsigned long turnTime() const
  {
    return mTurnTime;
  }

  /**
   *Gibt den Zeitpunkt der letzten Messung zurück.
   *\return lastMeasurementTime Zeitpunkt der letzten Messung, seit dem Boot des Arduino. (in µs)
   */
  unsigned long lastMeasurementTime() const
  {
    return mLastTime;
  }
};

