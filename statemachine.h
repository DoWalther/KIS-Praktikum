/**
 * Zustandsmaschine
 * 
 * Zustandsmaschine ist vom Rest des Systems entkoppelt. Der input muss als functor bereit gestellt werden.
 * Welche über \ref setReleaseTimeCalculator, \ref setTriggerProvider und \ref setSuppressionProvider aufgerufen werden können.,
 * sobald die Daten benötigt werden.
 * Der einzige output ist der Servomotor, dessen Zugriff durch \ref EngineControl bereitgestellt wird.
 *
 * \ref advanceState muss in der Arduino Hauptschleife aufgerufen werden.
 */
class StateMachine
{
public:
  /// Zustände
  enum class State {
    /// Fallmechanismus wird komplett geschlossen, dass die Kugel später durchfallen kann (Initialzustand)
    CLOSED,
    /// Auf die nächste Kugel warten, welche fallen gelassen wird. Anschließend den Servo für die Freigabe vorbereiten.
    WAIT_BALL,
    /// Darauf warten das der Servo seine Vorbereitung abgeschlossen hat.
    PREPARE,
    /// Wartezustand: Es wird auf den Knopfdruck gewartet um den Ball freizugeben. Anschließend wird die Freigabezeit berechnet.
    READY,
    /// Warte auf den Freigabezeitpunkt, dann Kugel freigeben.
    WAIT,
    /// Warten bis der Servo die Freigabe abgeschlossen hat. Danach in den Zustand \ref State::CLOSED gehen.
    RELEASE,
  };

  typedef long (*TimeProviderType)(void);
  typedef bool (*StateProviderType)(void);

private:
  /// Aktueller Zustand
  State mState = State::CLOSED;

  /** 
   * Zeit in µs seit dem Start des Arduino, in dem der nächste Zustandswechsel stattfinden sollte.
   * Nur zulässig für zeitliche transitionen.
   */
  unsigned long mWaitUntil = 0;
  /// Controller des Freigabemechanismus
  EngineControl& mEngineControl;

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
  /// -.- Function providing the state of the release suppression (high for do not release)

  /// Funktion die den Zustand der Freigabevermeidung bereitstellt (High-Flanke --> keine Freigabe)
  StateProviderType mSuppressionProvider = nullptr;

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
   * \param EngineControl release mechanism controller
   *
   * Intstantiiert eine neue Zustandsmaschine.
   * \param EngineControl Controller des Freigabemechanismus
   */
  StateMachine(EngineControl& EngineControl)
  : mEngineControl(EngineControl)
  {}

  /**
   * -.- Advance the state of the state machine and react to inputs
   *
   *
   */
  void advanceState()
  {
    switch (mState) {
      case State::CLOSED:
        mEngineControl.nextBall();
        //-.- Wait a bit longer so that the ball has definitely fallen through

        // Ein bisschen länger Warten, dass die Kugel defintiv durchgefallen ist.
        setWaitFromNow(400000);
        mState = State::WAIT_BALL;
      break;
      
      case State::WAIT_BALL:
        if (isWaitDone()) {
          mEngineControl.prepareRelease();
          setWaitFromNow(200000);
          mState = State::PREPARE;
        }
      break;
      
      case State::PREPARE:
        if (isWaitDone()) {
          mState = State::READY;
        }
      break;
      
      case State::READY:
        // -.- Start countdown only when not inhibited

	// Countdown wird nur gestartet, wenn nicht -.- wurde
        if (mTriggerProvider() && !mSuppressionProvider()) {
          auto waitTime = mReleaseTimeCalculator();
          if (waitTime >= 0) {
            mState = State::WAIT;
            setWaitFromNow(waitTime);
          } else {
            Serial.println("Wait time < 0, not releasing");
          }
        }
      break;
      
      case State::WAIT:
        if (mSuppressionProvider()) {
          // -.- Abort release immediately

	  // Freilassen sofort abbrechen
          mState = State::READY;
        } else if (isWaitDone()) {
          mEngineControl.release();
          setWaitFromNow(200000);
          mState = State::RELEASE;
        }
      break;
      
      case State::RELEASE:
        if (isWaitDone()) {
          mState = State::CLOSED;
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
   *-.- Set the function to be called to get the state of the suppression
   *
   * The function should return true to prohibit release of the ball.
   */
  void setSuppressionProvider(StateProviderType suppressionProvider)
  {
    mSuppressionProvider = suppressionProvider;
  }
};

