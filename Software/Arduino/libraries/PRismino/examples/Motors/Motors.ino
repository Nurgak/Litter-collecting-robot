/***************************************************************************************
 *
 * Title:       Motors
 * Description: Use the motors to move your robot. The setSpeed() function uses timer4.
 *              setSpeed takes some time to have an effect, when calling setSpeed too
 *              often (for a line follower for example) a delay must follow the call
 *              (100ms should do it).
 *
 ***************************************************************************************/
#include <prismino.h>

void setup()
{
  // nothing to set up
}

void loop()
{
  // set both motors (left and right) at 20% forwards
  setSpeed(20, 20);
  // wait at least 100ms or the setting won't have any effect before the next setting
  delay(500);
  setSpeed(-20, -20);
  delay(500);
}
