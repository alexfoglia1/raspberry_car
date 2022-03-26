// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  float vInPercentage = (float)sensorValue / 1023.f;

  // print out the value you read:
  Serial.println(vInPercentage);
}
