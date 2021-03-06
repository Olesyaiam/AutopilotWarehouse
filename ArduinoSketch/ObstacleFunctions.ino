void mode3TurnRightFromObstruction() {
  analogWrite(PIN_WHEELS_ENA, WHEELS_SPEED_TO_GO_AROUND_OBSTACLE_OUTSIDE);
  digitalWrite(PIN_WHEELS_IN2, LOW);
  
  do {
    digitalWrite(PIN_WHEELS_IN1, HIGH);
    delay(240);
    processBluetooth();
    digitalWrite(PIN_WHEELS_IN1, LOW);
    
    updateDistanceCm();

    if (distance < DISTANCE_WARNING) {
      Serial.println("2. I SEE IT " + String(distance));
    } else {
      Serial.println("2. I DON'T SEE IT " + String(distance));
    }
  } while (distance < DISTANCE_WARNING && mode == MODE_AROUND_OBSTACLE);

  if (mode == MODE_AROUND_OBSTACLE) {
    analogWrite(PIN_WHEELS_ENA, WHEELS_SPEED_DEFAULT);
    analogWrite(PIN_WHEELS_ENB, WHEELS_SPEED_DEFAULT);
    
    Serial.println("mode3TurnRightFromObstruction done " + String(distance));
    delay(100);
  }
}

bool mode3TurnUltrasonicLeftToObstruction() {
  do {
    armTurnLeft();
    updateDistanceCm();
    processBluetooth();

    if (distance < DISTANCE_WARNING) {
      Serial.println("1. I SEE IT " + String(distance));
      return true;
    } 
    else {
      Serial.println("1. I DON'T SEE IT " + String(distance));
    }
  } while (servoPositions.armMain < ARM_POSITION_MAIN_MAX && mode == MODE_AROUND_OBSTACLE);

  Serial.println("mode3TurnUltrasonicLeftToObstruction done " + String(distance));
  processBluetooth();

  if (mode == MODE_AROUND_OBSTACLE) {
    delay(90);
  }
  
  return false;
}

void processMode3() {
  rightForwardStart();
  leftForwardStart();
  
  int center = digitalRead(PIN_TRACING_CENTER) == HIGH ? 1 : 0;
  int right = digitalRead(PIN_TRACING_RIGHT) == HIGH ? 1 : 0;
  int left = digitalRead(PIN_TRACING_LEFT) == HIGH ? 1 : 0;

  if ((center + right + left) >= 2) {
    bothStop();
    armTurnCenter();
    
    analogWrite(PIN_WHEELS_ENA, WHEELS_SPEED_TO_GO_AROUND_OBSTACLE_OUTSIDE);
    analogWrite(PIN_WHEELS_ENB, WHEELS_SPEED_TO_GO_AROUND_OBSTACLE_INSIDE);
    
    leftForwardStart();
    rightForwardStart();

    processBluetooth();

    if (mode == MODE_AROUND_OBSTACLE) {
      delay(680);
      processBluetooth();
      bothStop();
  
      if (mode == MODE_AROUND_OBSTACLE) {
        setMode(MODE_FOLLOW_LINE, SWITCH_MODE_REASON_FOUND_LINE);
      }
    }
  }
}

void initMode3() {
  bothStop();
  setMode(MODE_AROUND_OBSTACLE, SWITCH_MODE_REASON_FOUND_OBSTACLE);
  
  analogWrite(PIN_WHEELS_ENA, WHEELS_SPEED_DEFAULT);
  analogWrite(PIN_WHEELS_ENB, WHEELS_SPEED_DEFAULT);
  
  digitalWrite(PIN_WHEELS_IN1, LOW);
  digitalWrite(PIN_WHEELS_IN2, HIGH);
  digitalWrite(PIN_WHEELS_IN3, LOW);
  digitalWrite(PIN_WHEELS_IN4, HIGH);

  processBluetooth();

  if (mode == MODE_AROUND_OBSTACLE) {
    delay(280);
    processBluetooth();

    if (mode == MODE_AROUND_OBSTACLE) {
      digitalWrite(PIN_WHEELS_IN1, LOW);
      digitalWrite(PIN_WHEELS_IN2, LOW);
      digitalWrite(PIN_WHEELS_IN3, LOW);
      digitalWrite(PIN_WHEELS_IN4, LOW);
    
      do {
        mode3TurnRightFromObstruction();
      } while (mode3TurnUltrasonicLeftToObstruction() && mode == MODE_AROUND_OBSTACLE);
    
      if (mode == MODE_AROUND_OBSTACLE) {
        addMoveToLastMovesArray(1);
        rightForwardStart();
        leftForwardStart();
      
        processBluetooth();
    
        if (mode == MODE_AROUND_OBSTACLE) {
          delay(480);
          processBluetooth();
    
          if (mode == MODE_AROUND_OBSTACLE) {
            analogWrite(PIN_WHEELS_ENA, WHEELS_SPEED_TO_GO_AROUND_OBSTACLE_INSIDE);
            analogWrite(PIN_WHEELS_ENB, WHEELS_SPEED_TO_GO_AROUND_OBSTACLE_OUTSIDE);
          
            rightForwardStart();
            leftForwardStart();
          }
        }
      }
    }
  }
}

void updateDistanceCm() {
  int distanceNew = ultrasonic.Distance();
  int distanceMin = distance * 0.9;
  int distanceMax = distance * 1.1;
  
  if (distanceNew >= distanceMin && distanceNew <= distanceMax) {
    distance = distanceNew;
  } else {
    delay(5);
    int distanceNew2 = ultrasonic.Distance();
    //Serial.println("distance try 2, old was " + String(distance) + "; new is " + String(distanceNew) + " and newest is " + String(distanceNew2));
  
    if (distanceNew >= distanceMin && distanceNew <= distanceMax) {
      distance = distanceNew2;
    } else {
      delay(5);
      int distanceNew3 = ultrasonic.Distance();
      //Serial.println("distance try 3: " + String(distanceNew3));

      float diff1 = float(distanceNew) / float(distance) - 1;
      float diff2 = float(distanceNew2) / float(distance) - 1;
      float diff3 = float(distanceNew3) / float(distance) - 1;

      diff1 = diff1 < 0 ? diff1 * -1 : diff1;
      diff2 = diff2 < 0 ? diff2 * -1 : diff2;
      diff3 = diff3 < 0 ? diff3 * -1 : diff3;

      if (diff1 <= diff2 && diff1 <= diff3) {
        distance = distanceNew;
      } else if (diff2 <= diff1 && diff2 <= diff3) {
        distance = distanceNew2;
      } else {
        distance = distanceNew3;
      }
    }
  }
}
