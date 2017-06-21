/**
 * Funktionsaufruf bei jeder Hochflanke eines Signales
 */
class EdgeDetector
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
   * Provide a new state information to the edge detector
   * 
   * If the state was false previously and is now true, the
   * callback function will be called.
   * 
   * \param state current state of the signal
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
   * Set the function to be called when a rising edge is detected on the signal
   * \param callback function to be called
   */
  void setCallback(CallbackType callback)
  {
    mCallback = callback;
  }
};

