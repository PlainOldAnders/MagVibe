// Used for electromagnet
int FORWARD = 6;
int FORWARDTWO = 9; //Unused for now
int BACKWARD = 10;
int BACKWARDTWO = 11; //Unused for now
bool isAttracting = false;

//Used for interfacing
int greenLed = 4;
int redLed = 5;
int startBut = 2;
int functionBut = 3;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTimeF = 0;
int buttonStateF;
int lastButtonStateF = LOW;

//Used for testing
int minRange[2] = {0, 10};
int maxRange[2] = {240, 255};
int increase = 10;
int feedbackWaitTime = 800;
bool isGoingUp = true;
bool isStopped = false;
int noOfRuns = 10;
int values[10];
int progressCounter = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //Used for electromagnet
  pinMode(FORWARD, OUTPUT);
  pinMode(FORWARDTWO, OUTPUT); //Unused for now
  pinMode(BACKWARD, OUTPUT);
  pinMode(FORWARDTWO, OUTPUT); //Unused for now

  //Used for interfacing
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(startBut, INPUT_PULLUP);
  pinMode(functionBut, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(functionBut), funcInt, CHANGE);
  randomSeed(analogRead(0));
  digitalWrite(redLed, LOW);
}

void loop() {
  int isStart = !digitalRead(startBut);
  digitalWrite(greenLed, LOW);

  if (isStart) {
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);
    isStopped = false;
    int value = random(maxRange[0], maxRange[1]);
    if (isGoingUp) value = random(minRange[0], minRange[1]);
    progressCounter++;
    Serial.print(String(progressCounter) + ": ");
    while (isGoingUp && value <= maxRange[1] || !isGoingUp && value >= minRange[0]) {
      //.....Replace following with vibrate...
      Serial.print(String(value) + "->");

      timedMagnetSwitch(20, value, true, 20);
      //delay(feedbackWaitTime);
      //......................................
      if (isStopped) {
        magnetStop();
        Serial.println(" Stopped with: " + String(value));
        isGoingUp = !isGoingUp;
        delay(500);
        digitalWrite(redLed, LOW);
        values[progressCounter - 1] = value;
        if (progressCounter == 10) {
          int finalMean = 0;
          Serial.println("Finished with:");
          Serial.print("(");
          for(int i = 0; i < noOfRuns; i++) {
            finalMean += values[i];
            Serial.print(String(values[i]));
            if(i != noOfRuns-1) Serial.print("+");
          }
          
          finalMean = finalMean/noOfRuns;
          Serial.println(")/" + String(noOfRuns) + "=" + String(finalMean));
          Serial.println("DONE!");
        }
        return;
      }

      if (isGoingUp) value += increase;
      else  value -= increase;
    }
  }



  //startClick();
  //funcClick();
  delay(10);
}

void startClick() {
  int reading = !digitalRead(startBut);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        //Call function here!!
        Serial.println("Start but!");
        //--------------------
      }
    }
  }

  lastButtonState = reading;
}

void funcInt() {
  digitalWrite(redLed, HIGH);
  isStopped = true;
}

void funcClick() {
  int reading = !digitalRead(functionBut);
  if (reading != lastButtonStateF) {
    lastDebounceTimeF = millis();
  }

  if ((millis() - lastDebounceTimeF) > debounceDelay) {
    if (reading != buttonStateF) {
      buttonStateF = reading;
      if (buttonStateF == HIGH) {
        //Call function here!!
        Serial.println("Function but!");
        //--------------------
      }
    }
  }

  lastButtonStateF = reading;
}

void timedMagnetSwitch(int switchTime, int force, bool keepRunning, int noOfRuns) {
  if (keepRunning) {
    int i = 0;
    while (i < noOfRuns) {
      //Serial.println("HERE?");
      magnetAttract(force);
      delay(switchTime);
      magnetRepel(force);
      delay(switchTime);
      i++;
    }
    magnetStop();
  } else {
    magnetAttract(force);
    delay(switchTime);
    magnetRepel(force);
  }

}

void magnetAttract(int force) {
  //Serial.println("Attract");
  analogWrite(FORWARD, force);
  analogWrite(FORWARDTWO, force);
  analogWrite(BACKWARD, LOW);
  analogWrite(BACKWARDTWO, LOW);

}

void magnetRepel(int force) {
  //Serial.println("Repel");
  analogWrite(FORWARD, LOW);
  analogWrite(FORWARDTWO, LOW);
  analogWrite(BACKWARD, force);
  analogWrite(BACKWARDTWO, force);

}

void magnetStop() {
  analogWrite(FORWARD, LOW);
  analogWrite(FORWARDTWO, LOW);
  analogWrite(BACKWARD, LOW);
  analogWrite(BACKWARDTWO, LOW);

}
