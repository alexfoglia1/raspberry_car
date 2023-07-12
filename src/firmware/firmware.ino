typedef struct
{
uint8_t  sync;
uint32_t data;
} __attribute__((packed)) msg_out;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  int sensorValue = analogRead(A5);
  float vInPercentage = (float)sensorValue / 1023.f;

  msg_out out;
  out.sync = 0xff;
  out.data = *(uint32_t*)(&vInPercentage);

  Serial.write((uint8_t*)&out, sizeof(msg_out));
}
