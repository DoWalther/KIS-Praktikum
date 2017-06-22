#include <Servo.h>

/**
 * Motorkontrolle
 */
class ServoControl
{
  /// Pin des Motors
  int mPin;
  /// Arduino servo library Instanzierung.
  Servo mServo;
  
public:
  /**
   * Instanziert einen neuen Motor.
   */
  ServoControl(int pin)
  : mPin(pin)
  {}

  /**
   * Konfiguration des Motors.
   * 
   */
  void setup()
  {
    mServo.attach(mPin);
    nextBall();
  }

  /**
   * Kugel wird freigegeben.
   */
  void release()
  {
    mServo.write(30);
  }

  /**
   * Schließen des Fallmechanismus, damit die nächste Kugel freigegeben werden kann.
   */
  void nextBall()
  {
    mServo.write(0);
  }

  /**
   * Vorbereitung zur Freigabe der Kugel.
   * 
   * Fallmechanismus wird so weit geöffnet, dass die Kugel beinahe fallen gelassen wird.
   * Dieser Schritt reduziert die tatsächlich benötigte Zeit, um die Kugel fallen zu lassen, wenn es soweit ist.
   */
  void prepareRelease()
  {
    mServo.write(17);
  }
};

