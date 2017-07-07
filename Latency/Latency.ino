//PIN for Red LED
int LED_PIN = 7;
//ADC pin for photodiode
int ADC_PIN = 0;
//Threshold for change required to trigger sample time calculation
int LIGHT_DELTA_THRESHOLD = 5;
//Time measurements above this are considered invalid
unsigned long TIMEOUT_MS = 1000;
unsigned long TIMEOUT_US = TIMEOUT_MS * 1000;

//Bias to subtract from current sensor value.  Used to calculate delta
long light_bias;


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);

  //Start serial
  Serial.begin(115200);
  
}

// the loop function runs over and over again forever
void loop() {
  int latencySamples = 20;
  long currentLatency = 0;
  unsigned long totalLatencyUs = 0;
  unsigned long samplesMeasured = 0;
  unsigned long minLatency = 100000;
  unsigned long maxLatency = 0;

  calibrate();
  Serial.println("Starting Measurement");
  for (int i=0; i<latencySamples; i++){
    currentLatency = getCurrLatencyUs();
   
    Serial.print("Current Latency: ");
    Serial.print(String(currentLatency/1000));
    Serial.println("ms");

    if (currentLatency > 0){
      minLatency = min(currentLatency, minLatency);
      maxLatency = max(currentLatency, maxLatency);
      totalLatencyUs += currentLatency;
      samplesMeasured++;
    }
  }
  
  Serial.print("Average Latency: ");
  if (totalLatencyUs > 0)
    Serial.print(String(totalLatencyUs/samplesMeasured/1000));
  else
    Serial.print(String(totalLatencyUs));
  Serial.println("ms");

  Serial.print("Max Latency: ");
  Serial.print(String(maxLatency/1000));
  Serial.println("ms");

  Serial.print("Min Latency: ");
  Serial.print(String(minLatency/1000));
  Serial.println("ms");
  
  delay(5000);
}

//returns -1 if timed out. Returns latency if did not timeout
long getCurrLatencyUs(){
  unsigned long startTime = 0;
  unsigned long endTime = 0;
  unsigned long elapsedTime = 0;
  long result;
  
  startTime = micros();
  ledOn();
  while (true){
    //Determine if we timed out
    elapsedTime = micros() - startTime; 
    if (elapsedTime > TIMEOUT_US){
      Serial.println("Wait led on timeout");
      result = -1;
      break;
    }

    //Determine if we got a valid measurement
    if (lightDelta() > LIGHT_DELTA_THRESHOLD){
      result = elapsedTime;
      break;
    }
  }

//  Serial.print("Delta LED On: ");
//  Serial.println(String(lightDelta()));

  ledOff();

  waitLEDMeasuredOff();
//  Serial.print("Delta LED Off: ");
//  Serial.println(String(lightDelta()));


  return result;
}

void waitLEDMeasuredOff(){
  unsigned long elapsedTime;
  unsigned long startTime = millis();

  while(true){
    elapsedTime = millis() - startTime;
    
    if (elapsedTime > TIMEOUT_MS){
      Serial.println("Wait led off timeout");
      break;
    }

    if (lightDelta() < 1){
//      Serial.println("LED measured off");
      break;
    }
    
  }
}

void ledOn(){
  digitalWrite(LED_PIN, HIGH);
}

void ledOff(){
  digitalWrite(LED_PIN, LOW);
}

int lightDelta(){
  int delta = analogRead(ADC_PIN) - light_bias;
  return abs(delta);
}

void calibrate(){
  light_bias = averageSamples(50);
  Serial.print("Calaulated Light Bias:" );
  Serial.println(String(light_bias));
  Serial.print("Current Light Sensor Value: ");
  Serial.println(analogRead(ADC_PIN));
}

int averageSamples(long samples){
  long total = 0;

  for (int i=0; i<samples; i++){
    total += analogRead(ADC_PIN);
    delay(10);
  }

  return total / samples;
}

