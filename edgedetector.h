/**
 * Call a function every time a rising edge is detected on a signal
 * 
 * \ref provideState must be called every time a new signal state information
 * is available or should be evaluated
 */
class EdgeDetector
{
  /// Previous state information
  bool mLastState = false;
  
public:
  /// Type for functions called when a rising edge is detected
  typedef void (*CallbackType)(void);

private:
  /// Function called when a rising edge is detected
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

