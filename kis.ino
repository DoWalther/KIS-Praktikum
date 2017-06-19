
#include "config.h"
#include "utility.h"

#include "speedmonitor.h"
#include "servocontrol.h"
#include "inhibitor.h"
#include "timecalculation.h"
#include "statemachine.h"
#include "edgedetector.h"

/// Global inhibitor
Inhibitor inhibitor;
/// Global speed monitor using the hall sensor, measuring full cycles
SpeedMonitor hallSpeedMonitor(PIN_HALL_SENSOR);
/// Global speed monitor using the light sensor, measuring half cycles
SpeedMonitor lightSpeedMonitor(PIN_LIGHT_SENSOR, 12, true);
/// Global servo controller
ServoControl servoControl(PIN_SERVO);
/// Global release time calculator
TimeCalculation timeCalculation;
/// Global state machine
StateMachine stateMachine(servoControl);
/// Global button 1 edge detector
EdgeDetector firstButtonEdgeDetector;

/**
 * Setup pin modes for pins used in the \ref loop function directly
 */
void setupPins()
{
  pinMode(PIN_TRIGGER, INPUT);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_BUTTON1, INPUT);
}

/**
 * Call the setup functions of all relevant components
 */
void setupComponents()
{
  hallSpeedMonitor.setup();
  lightSpeedMonitor.setup();
  servoControl.setup();
}

/**
 * Setup component interconnections
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
 * System setup
 * 
 * Instantiate components and connect them.
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
 * Main loop
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
