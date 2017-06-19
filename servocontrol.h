#include <Servo.h>

/**
 * Control the ball release servo motor
 * 
 * \ref setup must be called in the Arduino main setup function.
 */
class ServoControl
{
  /// Pin number of the servo motor
  int mPin;
  /// Arduino servo library instance
  Servo mServo;
  
public:
  /**
   * Instantiate a new servo controller
   * 
   * \param pin pin number of the servo motor
   */
  ServoControl(int pin)
  : mPin(pin)
  {}

  /**
   * Configure the controller
   * 
   * Attaches and resets the servo to the fully-closed position.
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

