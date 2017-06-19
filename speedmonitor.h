/**
 * Monitor the time duration of high/low cycles on a pin
 * 
 * \ref setup and \ref loop must be called in their respective Arduino main counterparts.
 * 
 * Measurement data can be polled with \ref turnTime or optionally pushed to a callback
 * function by using \ref setCallback.
 */
class SpeedMonitor
{
  /// Number of the pin monitored
  const int mPin;

  /// Time of the last transition
  unsigned long mLastTime = 0;
  /// State of the pin that was measured last
  bool mLastState = 0;
  /// Saved cycle duration
  unsigned long mTurnTime = 0;

  /// Factor to multiple the raw cycle duration with to get \ref mTurnTime
  unsigned short mFactor;
  /// Whether to measure full cycles (high to low transition) or half cycles (low to high and high to low)
  /// \note Half cycles should only be measured when the duty cycle is approx. 50 %
  bool mMeasureHalfCycles;

public:
  /// Type for functions called when a new measurement is ready
  typedef void (*CallbackType)(unsigned long);
  
private:
  /// Function to be called when a new measurement is ready
  CallbackType mCallback = nullptr;
  
public:
  /**
   * Instantiate a new SpeedMonitor
   * 
   * \param pin number of the pin to monitor
   * \param factor factor to multiple the raw cycle duration with to get the turn time
   * \param measureHalfCycles Whether to measure full cycles (low to high transition) or half cycles (low to high and high to low)
   */
  SpeedMonitor(int pin, unsigned short factor = 1, bool measureHalfCycles = false)
  : mPin(pin), mFactor(factor), mMeasureHalfCycles(measureHalfCycles)
  {}

  /**
   * Configure the monitoring
   */
  void setup()
  {
    pinMode(mPin, INPUT);
  }

  /**
   * Loop function to be called in the arduino main loop
   * 
   * Monitors the pin for state transitions and measures the time
   * accordingly
   */
  void loop()
  {
    int state = digitalRead(mPin);
    
    if (state != mLastState) {
      // State transition happened
      // Only trigger on high to low transition for full cycles
      if (mMeasureHalfCycles || !state) {
        unsigned long now = micros();
        unsigned long dif = now - mLastTime;
        mTurnTime = dif * mFactor;
        
        debugprint("tt new: ");
        debugprintln(mTurnTime);
        
        mLastTime = now;
        if (mCallback) {
          mCallback(mTurnTime);
        }
      }
      mLastState = state;
    }
  }

  /**
   * Set a function to be called when a new measurement is ready
   */
  void setCallback(CallbackType callback)
  {
    mCallback = callback;
  }

  /**
   * Get the last cycle measurement
   * \return turn time (last raw cycle time multiplied by \ref mFactor) in µs
   */
  unsigned long turnTime() const
  {
    return mTurnTime;
  }

  /**
   * Get the time the last measurement was taken
   * \return when the last measurement was taken in µs since Arduino boot
   */
  unsigned long lastMeasurementTime() const
  {
    return mLastTime;
  }
};

