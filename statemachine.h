/**
 * Zustandsmaschine
 * 
 * Zustandsmaschine ist vom Rest des Systems entkoppelt. Der input muss als functor bereit gestellt werden.
 * Welche über \ref setReleaseTimeCalculator, \ref setTriggerProvider und \ref setInhibitionProvider aufgerufen werden können.,
 * sobald die Daten benötigt werden.
 * Der einzige output ist der Servomotor, dessen Zugriff durch \ref ServoControl bereitgestellt wird.
 *
 * \ref advanceState muss in der Arduino Hauptschleife aufgerufen werden.
 */
class StateMachine
{
public:
  /// Zustände
  enum class State {
    /// Fallmechanismus wird komplett geschlossen, dass die Kugel später durchfallen kann (Initialzustand)
    BALL_FALL_THROUGH,
    /// Auf die nächste Kugel warten, welche fallen gelassen wird. Anschließend den Servo für die Freigabe vorbereiten.
    BALL_FALLING_THROUGH,
    /// Darauf warten das der Servo seine Vorbereitung abgeschlossen hat.
    PREPARING,
    /// Wartezustand: Es wird auf den Knopfdruck gewartet um den Ball freizugeben. Anschließend wird die Freigabezeit berechnet.
    ARMED,
    /// Warte auf den Freigabezeitpunkt, dann Kugel freigeben.
    WAIT_RELEASE,
    /// Warten bis der Servo die Freigabe abgeschlossen hat. Danach in den Zustand \ref State::BALL_FALL_THROUGH gehen.
    RELEASING,
  };

  typedef long (*TimeProviderType)(void);
  typedef bool (*StateProviderType)(void);

private:
  /// Aktueller Zustand
  State mState = State::BALL_FALL_THROUGH;

  /** 
   * Zeit in µs seit dem Start des Arduino, in dem der nächste Zustandswechsel stattfinden sollte.
   * Nur zulässig für zeitliche transitionen.
   */
  unsigned long mWaitUntil = 0;
  /// Controller des Freigabemechanismus
  ServoControl& mServoControl;

  /**-.-
   * Function providing the duration that should be waited until release
   * (given the current state of the system)
   *
   * Funktion die die Wartezeit bis zur Freigabe der Kugel bereitstellt.
   * ()
   */
  TimeProviderType mReleaseTimeCalculator = nullptr;
  /// -.- Function providing the state of the trigger button (high for release)

  /// Funktion die den Zusatnd für den "Trigger-Button" bereitstellt (High-Flanke für Freigabe)
  StateProviderType mTriggerProvider = nullptr;
  /// -.- Function providing the state of the release inhibition (high for do not release)

  /// Funktion die den Zustand der Freigabevermeidung bereitstellt (High-Flanke --> keine Freigabe)
  StateProviderType mInhibitionProvider = nullptr;

  /**
   * Schedule a state transition in the future
   * 
   * The next state then needs to check when the transition should take place
   * by \ref isWaitDone.
   * \param us time to wait in µs

   * PLant einen Zustandsübergang in der Zukunft
   * 
   * -.-
   * \param us wartezeit in µs
   */
  void setWaitFromNow(unsigned long us)
  {
    mWaitUntil = micros() + us;
  }

  /**
   * -.-Check whether the wait time previously set with \ref setWaitFromNow has passed
   *
   * Überprüft ob die zuvor gesetzte Wartezeit (\ref setWaitFromNow) vergangen ist.
   */
  bool isWaitDone()
  {
    return (micros() >= mWaitUntil);
  }

public:
  /**
   * -.-Instantiate a new state machine
   * \param servoControl release mechanism controller
   *
   * Intstantiiert eine neue Zustandsmaschine.
   * \param ServoControl Controller des Freigabemechanismus
   */
  StateMachine(ServoControl& servoControl)
  : mServoControl(servoControl)
  {}

  /**
   * -.- Advance the state of the state machine and react to inputs
   *
   *
   */
  void advanceState()
  {
    switch (mState) {
      case State::BALL_FALL_THROUGH:
        mServoControl.nextBall();
        //-.- Wait a bit longer so that the ball has definitely fallen through

        // Ein bisschen länger Warten, dass die Kugel defintiv durchgefallen ist.
        setWaitFromNow(400000);
        mState = State::BALL_FALLING_THROUGH;
      break;
      
      case State::BALL_FALLING_THROUGH:
        if (isWaitDone()) {
          mServoControl.prepareRelease();
          setWaitFromNow(200000);
          mState = State::PREPARING;
        }
      break;
      
      case State::PREPARING:
        if (isWaitDone()) {
          mState = State::ARMED;
        }
      break;
      
      case State::ARMED:
        // -.- Start countdown only when not inhibited

	// Countdown wird nur gestartet, wenn nicht -.- wurde
        if (mTriggerProvider() && !mInhibitionProvider()) {
          auto waitTime = mReleaseTimeCalculator();
          if (waitTime >= 0) {
            mState = State::WAIT_RELEASE;
            setWaitFromNow(waitTime);
          } else {
            Serial.println("Wait time < 0, not releasing");
          }
        }
      break;
      
      case State::WAIT_RELEASE:
        if (mInhibitionProvider()) {
          // -.- Abort release immediately

	  // Freilassen sofort abbrechen
          mState = State::ARMED;
        } else if (isWaitDone()) {
          mServoControl.release();
          setWaitFromNow(200000);
          mState = State::RELEASING;
        }
      break;
      
      case State::RELEASING:
        if (isWaitDone()) {
          mState = State::BALL_FALL_THROUGH;
        }
      break;
    }
  }

  /**
   * -.- Get the state of the state machine
   *
   * Zustand der Zustandsmaschine abrufen.
   */
  State state() const
  {
    return mState;
  }

  /**
   * -.- Set the function to be called to calculate the wait time for releasing the ball
   * 
   * The function should return the time in µs to wait from the current point of time
   * until the ball can be released so it falls through the hole in the turntable.


   * Setzt die Funktion, die aufgerufen werden soll, um die Wartezeit bis zur Freigabe der Kugel zu berechnen.
   * 
   * Die Funktion gibt die Zeit in µs zurück, die vom aktuellen Zeiptunkt bis zum Zeitpunkt der Freigabe der Kugel
   * gewartet werden muss, damit die Kugel durch das Loch im Drehtisch fällt. 
   */
  void setReleaseTimeCalculator(TimeProviderType releaseTimeCalculator)
  {
    mReleaseTimeCalculator = releaseTimeCalculator;
  }

  /**
   * -.-Set the function to be called to get the state of the trigger
   * 
   * The function should return true to trigger a ball release.


   * Setzt die Funktion die aufgerufen werden soll, um den Zustand des Triggers abzurufen.
   * 
   * Die Funktion soll "True" zurückgeben um eine Freigabe der Kugel zu triggern.
   */
  void setTriggerProvider(StateProviderType triggerProvider)
  {
    mTriggerProvider = triggerProvider;
  }

  /**
   *-.- Set the function to be called to get the state of the inhibition
   *
   * The function should return true to prohibit release of the ball.
   */
  void setInhibitionProvider(StateProviderType inhibitionProvider)
  {
    mInhibitionProvider = inhibitionProvider;
  }
};

