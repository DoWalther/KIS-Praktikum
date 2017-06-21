#include <Servo.h>

/**
 * Motorkontrolle
 */
class ServoControl
{
  /// Pin des Motors
  int mPin;
  /// Arduino servo library Instanzierung
  Servo mServo;
  
public:
  /**
   * instanziert neuen Motor
   */
  ServoControl(int pin)
  : mPin(pin)
  {}

  /**
   * Konfiguration des Motors
   * 
   */
  void setup()
  {
    mServo.attach(mPin);
    nextBall();
  }

  /**
   * Release the ball
   */
  void release()
  {
    mServo.write(30);
  }

  /**
   * Fully close the release mechanism so the next ball can fall through
   */
  void nextBall()
  {
    mServo.write(0);
  }

  /**
   * Prepare to release the ball
   * 
   * Opens the release mechanism so that the ball almost falls down. This
   * preparation reduces the time needed to actually perform the release
   * when the time comes.
   */
  void prepareRelease()
  {
    mServo.write(17);
  }
};

