/**
 * Funktionsaufruf bei jeder Hochflanke eines Signales
 */
class HighDetector
{
  bool mLastState = false;
  
public:
  /// Typ für Funktionen die bei Hochflanken ausgelöst werden
  typedef void (*CallbackType)(void);

private:
  /// Funktion die bei Hochflanken ausgelöst wird
  CallbackType mCallback = nullptr;

public:
  /**
   * Stellt eine neue Zustandsinformation für den High-Detektor bereit.
   *
   * Nach einem Zustandswechsel von false auf true, 
   * wird die callback Funktion aufgerufen.
   * 
   * \param state Aktueller Zustand des Signals.
   */
  void provideState(bool state)
  {
    if (state != mLastState) {
      if (state) {
        mCallback();
      }
      mLastState = state;
    }
  }

  /**
   * Setzt die callback Funktion die bei aufsteigender Flanke aufgerufen wird.
   *
   * \param callback Funktion die aufgerufen wird.
   */
  void setCallback(CallbackType callback)
  {
    mCallback = callback;
  }
};

