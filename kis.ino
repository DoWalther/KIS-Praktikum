
#include "config.h"
#include "utility.h"

#include "speedmonitor.h"
#include "servocontrol.h"
#include "inhibitor.h"
#include "timecalculation.h"
#include "statemachine.h"
#include "edgedetector.h"

Inhibitor inhibitor;
SpeedMonitor hallSpeedMonitor(PIN_HALL_SENSOR); // misst volle Runden
SpeedMonitor lightSpeedMonitor(PIN_LIGHT_SENSOR, 12, true); // misst halbe Runde
ServoControl servoControl(PIN_SERVO);
TimeCalculation timeCalculation;
StateMachine stateMachine(servoControl);
EdgeDetector firstButtonEdgeDetector;

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
  hallSpeedMonitor.setup();
  lightSpeedMonitor.setup();
  servoControl.setup();
}

/**
 * Setup der Komponenten-Verbindungen
 */
void setupComponentConnections()
{
  hallSpeedMonitor.setCallback([](unsigned long turnTime) {
    inhibitor.hallSpeedCallback(turnTime);
  });
  lightSpeedMonitor.setCallback([](unsigned long turnTime) {
    inhibitor.lightSpeedCallback(turnTime);
  });

  stateMachine.setTriggerProvider([]() -> bool {
    return digitalRead(PIN_TRIGGER);
  });
  stateMachine.setInhibitionProvider([]() {
    return inhibitor.isInhibited();
  });
  stateMachine.setReleaseTimeCalculator([]() {
    auto now = micros();
    auto lastCrossing = hallSpeedMonitor.lastMeasurementTime();
    auto timeInRound = now - lastCrossing;
    return timeCalculation(hallSpeedMonitor.turnTime(), timeInRound);
  });

  firstButtonEdgeDetector.setCallback([]() {
    Serial.println(hallSpeedMonitor.turnTime());
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
  firstButtonEdgeDetector.provideState(digitalRead(PIN_BUTTON1));
  
  hallSpeedMonitor.loop();
  lightSpeedMonitor.loop();
  
  stateMachine.advanceState();

  // Output inhibition state to LED
  digitalWrite(PIN_LED1, inhibitor.isInhibited());
  // Output release state to LED
  digitalWrite(PIN_LED2, (stateMachine.state() == StateMachine::State::WAIT_RELEASE));
}
