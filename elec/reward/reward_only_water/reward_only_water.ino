int water_pin = 3;

void setup() {

  Serial.begin(9600);
  pinMode(water_pin, OUTPUT);

}

void loop() {

  Serial.println("OFF");
  digitalWrite(water_pin, LOW);
  delay(2000);

  Serial.println("ON");
  digitalWrite(water_pin, HIGH);
  delay(2000);

}
