void processMode4() {
  
}

void findAndTakePackage() {
  // 1. Сложить руку в исходное состояние
  // 2. Вертеть влево и вправо, запомнить расстояние до ближайшего объекта на каждом градусе
  // 3. Повернуться в сторону ближайшего объекта
  // 4. Раскрыть клюв и начать приближать клюв к объекту, пока расстояние клюва до объекта не станет примерно равно CLAW_DISTANCE_HOLD
  // 5. Закрыть клюв
  // 6. Медленно поднимаем коробку и проверяем каждые несколько градусов поднятия, что коробка в клюве (CLAW_DISTANCE_HOLD)
  // 7. Если коробка выпала, то переключаемся в ручной режим и посылаем сигнал в Андроид, что нужна помощь оператора
  // 8. Если коробка не выпала, то разворачиваемся на 360 градусов и включаем режим слежения за линией

  armToDefaultPosition();
  armTurnRightMax();
  findObjectAndTurnThere();
  
//  if (findPackageAndHoverAboveIt()) {
//    takePackage();
//  }
}

bool takePackage() {
  openClaw();

  for (int armPositionKey = 0; armPositionKey < ARM_TAKE_PACKAGE_POSITIONS_COUNT; armPositionKey++) {
    int left = ARM_TAKE_PACKAGE_POSITIONS[armPositionKey][0];
    int right = ARM_TAKE_PACKAGE_POSITIONS[armPositionKey][1];

    int leftDistance = servoPositions.armLeft - left;
    leftDistance = leftDistance < 0 ? leftDistance * -1 : leftDistance;
    
    int rightDistance = servoPositions.armRight - right;
    rightDistance = rightDistance < 0 ? rightDistance * -1 : rightDistance;
    
    int factor = (float)rightDistance / (float)leftDistance;
    factor = factor < 0 ? 0 : factor;

    int currentLeftPause = 0;

    Serial.println(String(left) + " - " + String(right) + " - " + String(factor));

    while (servoPositions.armRight != right || servoPositions.armLeft != left) {
      if (servoPositions.armRight > right) {
        servoPositions.armRight--;
      } else if (servoPositions.armRight < right) {
        servoPositions.armRight++;
      }

      if (currentLeftPause < factor) {
        currentLeftPause++;
      } else {
        currentLeftPause = 0;
        
        if (servoPositions.armLeft < left) {
          servoPositions.armLeft++;
        } else if (servoPositions.armLeft > left) {
          servoPositions.armLeft--;
        }
      }
  
      armServoRightRotateToPositionWithoutEeprom();
      armServoLeftRotateToPositionWithoutEeprom();
      delay(20);
      
      clawDistance = analogRead(PIN_INFRARED_CLAW_DISTANCE);
      Serial.println(String(servoPositions.armLeft) + " - " + String(servoPositions.armRight) + ": " + clawDistance);
 
      if (clawDistance < CLAW_DISTANCE_HOLD) {
        if ((armPositionKey + 1) < ARM_TAKE_PACKAGE_POSITIONS_COUNT) {
          int left = ARM_TAKE_PACKAGE_POSITIONS[armPositionKey + 1][0];
          int right = ARM_TAKE_PACKAGE_POSITIONS[armPositionKey + 1][1];

          buzz(50);
          delay(500);
          armToPosition(left, right, 100);
          delay(500);
        }
        
        closeClaw();
        armToDefaultPosition();

        return true;
      }    
    }
  
    EEPROM.put(0, servoPositions);
  }

  armToDefaultPosition();
  buzz(500);

  return false;
}

bool findPackageAndHoverAboveIt() {
  openClaw();

  int const MAIN_POSITIONS_COUNT = 7;
  int mainPositions[MAIN_POSITIONS_COUNT] = {servoPositions.armMain - 15, servoPositions.armMain - 10, servoPositions.armMain - 5, servoPositions.armMain, servoPositions.armMain + 5, servoPositions.armMain + 10, servoPositions.armMain + 15};

  for (int mainPositionKey = 0; mainPositionKey < MAIN_POSITIONS_COUNT; mainPositionKey++) {
    armServoMainRotateSlowToPosition(mainPositions[mainPositionKey]);

    for (int armPositionKey = 0; armPositionKey < ARM_HOVER_POSITIONS_COUNT; armPositionKey++) {
      int left = ARM_HOVER_POSITIONS[armPositionKey][0];
      int right = ARM_HOVER_POSITIONS[armPositionKey][1];

      int leftDistance = servoPositions.armLeft - left;
      leftDistance = leftDistance < 0 ? leftDistance * -1 : leftDistance;
      
      int rightDistance = servoPositions.armRight - right;
      rightDistance = rightDistance < 0 ? rightDistance * -1 : rightDistance;
      
      int factor = (float)rightDistance / (float)leftDistance;
      factor = factor < 0 ? 0 : factor;
  
      int currentLeftPause = 0;

      if (left == 0 && right == 0) {
        continue;
      }
  
      while (servoPositions.armRight != right || servoPositions.armLeft != left) {
        if (servoPositions.armRight > right) {
          servoPositions.armRight--;
        } else if (servoPositions.armRight < right) {
          servoPositions.armRight++;
        }

        armServoRightRotateToPositionWithoutEeprom();

        if (currentLeftPause < factor) {
          currentLeftPause++;
        } else {
          currentLeftPause = 0;
          
          if (servoPositions.armLeft < left) {
            servoPositions.armLeft++;
          } else if (servoPositions.armLeft > left) {
            servoPositions.armLeft--;
          }
  
          armServoLeftRotateToPositionWithoutEeprom();
        }

        delay(20);
        
        clawDistance = analogRead(PIN_INFRARED_CLAW_DISTANCE);
        Serial.println(String(servoPositions.armLeft) + " - " + String(servoPositions.armRight) + ": " + clawDistance);
   
        if (clawDistance < CLAW_DISTANCE_HOVER) {
          EEPROM.put(0, servoPositions);
  
          return true;
        }    
      }

      Serial.println();
      Serial.println();
      Serial.println();
  
      EEPROM.put(0, servoPositions);
    }
  }

  buzz(500);

  return false;
}

int findObjectRightToLeft(int startDegree) {
  if (startDegree < ARM_POSITION_MAIN_MIN) {
    startDegree = ARM_POSITION_MAIN_MIN;
  }
  
  int degreePackageStart = -1;
  int degreePackageEnd = -1;
  
  while (degreePackageStart == -1 || degreePackageEnd == -1) {
    //Serial.println(String(startDegree) + " - right to left, current is " + String(servoPositions.armMain));

    armServoMainRotateSlowToPosition(startDegree);
    
    for (servoPositions.armMain = startDegree; servoPositions.armMain <= ARM_POSITION_MAIN_MAX; servoPositions.armMain++) {
      armServoMainRotateToPositionWithoutEeprom();
      distance = ultrasonic.Distance();
      Serial.println(distance);
  
      if (distance < MAX_DISTANCE_TO_PACKAGE) {
        buzz(1);

        if (degreePackageStart < 0) {
          buzz(500);
          degreePackageStart = servoPositions.armMain;
        } else {
          degreePackageEnd = servoPositions.armMain;
        }
      } else if (degreePackageEnd > 0) {
        buzz(500);
        break;
      } else {
        servoPositions.armMain += 2;
      }
    }
  }

  return degreePackageStart + (degreePackageEnd - degreePackageStart) / 2;
}

int findObjectLeftToRight(int startDegree) {
  if (startDegree > ARM_POSITION_MAIN_MAX) {
    startDegree = ARM_POSITION_MAIN_MAX;
  }
  
  int degreePackageStart = -1;
  int degreePackageEnd = -1;
  
  while (degreePackageStart == -1 || degreePackageEnd == -1) {
    //Serial.println(String(startDegree) + " - left to right, current is " + String(servoPositions.armMain));

    armServoMainRotateSlowToPosition(startDegree);
    
    for (servoPositions.armMain = startDegree; servoPositions.armMain >= ARM_POSITION_MAIN_MIN; servoPositions.armMain--) {
      armServoMainRotateToPositionWithoutEeprom();
      distance = ultrasonic.Distance();
      Serial.println(distance);
  
      if (distance < MAX_DISTANCE_TO_PACKAGE) {
        buzz(1);

        if (degreePackageEnd < 0) {
          buzz(500);
          degreePackageEnd = servoPositions.armMain;
        } else {
          degreePackageStart = servoPositions.armMain;
        }
      } else if (degreePackageStart > 0) {
        buzz(500);
        break;
      } else {
        servoPositions.armMain -= 2;
      }
    }
  }

  return degreePackageStart + (degreePackageEnd - degreePackageStart) / 2;
}

int findObject() {
  int degreePackageCenter1 = findObjectRightToLeft(ARM_POSITION_MAIN_MIN);
  int degreePackageCenter2 = findObjectLeftToRight(servoPositions.armMain + 15);
  int degreePackageCenter3 = findObjectRightToLeft(servoPositions.armMain - 15);
  int degreePackageCenter = (degreePackageCenter1 + degreePackageCenter2 + degreePackageCenter3) / 3;
  Serial.println(String(degreePackageCenter) + ": " + String(degreePackageCenter1) + " - " + String(degreePackageCenter2) + " - " + String(degreePackageCenter3));

  return degreePackageCenter;
}

void findObjectAndTurnThere() {
  int degreePackageCenter = findObject();
  armServoMainRotateSlowToPosition(degreePackageCenter);
}
