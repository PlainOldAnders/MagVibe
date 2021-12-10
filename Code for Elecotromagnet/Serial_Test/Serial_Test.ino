
//Used for serial debugging
int counter = 0;
char incomingByte;
String incomingMsg = "";
unsigned long long prevTime = 0;
unsigned long long currentTime = 0;
int interval = 1000;
bool shouldCount = false;
int addVal = 100;

int thres[] = {87, 82, 94, 67, 89, 99, 98, 113, 130};

// Used for electromagnet
int FORWARD = 6;
int FORWARDTWO = 9;
int BACKWARD = 10;
int BACKWARDTWO = 11;
bool isAttracting = false;

void setup() {
  Serial.begin(9600);

  //Used for serial debugging
  pinMode(LED_BUILTIN, OUTPUT);

  //Used for electromagnet
  pinMode(FORWARD, OUTPUT);
  pinMode(FORWARDTWO, OUTPUT);
  pinMode(BACKWARD, OUTPUT);
  pinMode(FORWARDTWO, OUTPUT);
}

void loop() {
  currentTime = millis();
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.print("-> ");
    Serial.println(incomingByte);
    if ((int) incomingByte == 10) {
      Serial.println("INPUT: " + incomingMsg);
      if (incomingMsg == "ON" ||
          incomingMsg == "on"  ||
          incomingMsg == "On") {
        Serial.println("LED ON");
        digitalWrite(LED_BUILTIN, HIGH);
      } else if (incomingMsg == "OFF" ||
                 incomingMsg == "off"  ||
                 incomingMsg == "Off") {
        Serial.println("LED OFF");
        digitalWrite(LED_BUILTIN, LOW);
      } else if (incomingMsg == "TOGGLE" ||
                 incomingMsg == "toggle" ||
                 incomingMsg == "Toggle" ||
                 incomingMsg == "TGL" ||
                 incomingMsg == "tgl" ||
                 incomingMsg == "Tgl"
                ) {
        Serial.println("Toggle Counter");
        shouldCount = !shouldCount;
      } else if (incomingMsg == "RESET" ||
                 incomingMsg == "reset" ||
                 incomingMsg == "Reset" ||
                 incomingMsg == "RST" ||
                 incomingMsg == "rst" ||
                 incomingMsg == "Rst"
                ) {
        Serial.println("Reset Counter");
        counter = 0;
      } else if (incomingMsg.indexOf("CLICK") == 0 ||
                 incomingMsg.indexOf("click") == 0 ||
                 incomingMsg.indexOf("Click") == 0) {
        Serial.println("Clicking");
        if (incomingMsg.length() == 5) {
          timedMagnetSwitch(20, 200, true, 4);
        } else {
          int clickNo = incomingMsg.substring(5).toInt();
          handleClick(clickNo, false);
        }

      } else if (incomingMsg.indexOf("CCLICK") == 0 ||
                 incomingMsg.indexOf("cclick") == 0 ||
                 incomingMsg.indexOf("Cclick") == 0) {
        Serial.println("Custom Clicking");
        int clickNo = incomingMsg.substring(6).toInt();
        handleClick(clickNo, true);

      } else {
        Serial.println("No CMD");
      }
      incomingMsg = "";
    } else {
      incomingMsg += incomingByte;
    }

  }

  if (currentTime - prevTime > interval) {
    if (shouldCount) Serial.println("Counter: " + String(counter));
    counter++;
    prevTime = currentTime;
  }
  delay(10);
}

void handleClick(int value, bool isCustom) {
  if (isCustom) {
    Serial.println("Click with: " + String(value));
    timedMagnetSwitch(20, value, true, 4);
  } else {
    if (value > 8) {
      Serial.println("Out of bounds!");
    } else {
      Serial.println("Click with: " + String(thres[value]));
      timedMagnetSwitch(20, thres[value]+addVal, true, 4);
    }
  }

}

void timedMagnetSwitch(int switchTime, int force, bool keepRunning, int noOfRuns) {
  if (keepRunning) {
    int i = 0;
    while (i < noOfRuns) {
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

void magnetSwitch(int force) {
  if (isAttracting) {
    magnetRepel(force);
    isAttracting = !isAttracting;
  } else {
    magnetAttract(force);
    isAttracting = !isAttracting;
  }
}


void magnetAttract(int force) {
  analogWrite(FORWARD, force);
  analogWrite(FORWARDTWO, force);
  analogWrite(BACKWARD, LOW);
  analogWrite(BACKWARDTWO, LOW);

}

void magnetRepel(int force) {
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
