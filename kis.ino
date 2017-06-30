
#include "pins.h"
#include "debug.h"
#include "observer.h"
#include "enginecontrol.h"
#include "suppression.h"
#include "time.h"
#include "statemachine.h"
#include "highdetector.h"

Suppression suppressor;
Observer hallObserver(PIN_HALL_SENSOR); // misst volle Runden
Observer lightObserver(PIN_LIGHT_SENSOR, 12, true); // misst halbe Runde
EngineControl EngineControl(PIN_SERVO);
Time timeCalculation;
StateMachine stateMachine(EngineControl);
HighDetector firstButtonHighDetector;

/**
 * Festlegung Input/Output Pins
 */
void setupPins()
{
  pinMode(PIN_TRIGGER, INPUT);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_BUTTON1, INPUT);
}

/**
 * Setup aller relevanten Komponenten
 */
void setupComponents()
{
  hallObserver.setup();
  lightObserver.setup();
  EngineControl.setup();
}

/**
 * Setup der Komponenten-Verbindungen
 */
void setupComponentConnections()
{
  hallObserver.setCallback([](unsigned long turnTime) {
    suppressor.hallSpeedCallback(turnTime);
  });
  lightObserver.setCallback([](unsigned long turnTime) {
    suppressor.lightSpeedCallback(turnTime);
  });

  stateMachine.setTriggerProvider([]() -> bool {
    return digitalRead(PIN_TRIGGER);
  });
  stateMachine.setSuppressProvider([]() {
    return suppressor.suppressionState();
  });
  stateMachine.setReleaseTimeCalculator([]() {
    auto now = micros();
    auto lastCrossing = hallObserver.lastMeasurementTime();
    auto timeInRound = now - lastCrossing;
    return timeCalculation(hallObserver.turnTime(), timeInRound);
  });

  firstButtonHighDetector.setCallback([]() {
    Serial.println(hallObserver.turnTime());
  });
}

/**
System Setup - iniziert alle Komponenten und verbindet diese
 */
void setup()
{
  Serial.begin(250000);
  
  setupPins();
  setupComponents();
  setupComponentConnections();

  Serial.println("Setup done");
}

/**
 * Hauptschleife
 */
void loop()
{
  firstButtonHighDetector.provideState(digitalRead(PIN_BUTTON1));
  
  hallObserver.loop();
  lightObserver.loop();
  
  stateMachine.advanceState();

  // Output suppression state to LED
  digitalWrite(PIN_LED1, suppressor.suppressionState());
  // Output release state to LED
  digitalWrite(PIN_LED2, (stateMachine.state() == StateMachine::State::WAIT));
}
