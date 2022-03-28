void setup()
{
  Serial.begin(9600);
}


void loop()
{
  int sensorValue = analogRead(A0);
  float vInPercentage = (float)sensorValue / 1023.f;

  Serial.println(vInPercentage);
}
