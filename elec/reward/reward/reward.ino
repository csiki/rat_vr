
enum SIDE {LEFT, RIGHT, BOTH};

const int LPIN = 3, RPIN = 4;  // L/R puff drive: green, yellow
const int STBY = 2;  // puff enable: orange
const int VPIN = 5;  // water reward valve pin

// calibration data points (~200ml gravity fed pressure, 2mm ID tube)
//   tube len 50mm: 210ms   => 1^2*pi*50 mm3 => 157ul / 210ms = 0.748 ul/ms
//   tube len 28.5mm: 110ms => 1^2*pi*28.5 mm3 => 90ul / 110ms = 0.814 ul/ms
const float UL_PER_MS = (0.748 + 0.814) / 2.0;  // microliter/millisec; needs to be calibrated

const int BUFFER_SIZE = 10;  // for manual calibration
char BUFFER[BUFFER_SIZE];


void on() {
  digitalWrite(STBY, HIGH);
  delay(10);
}

void off() {
  digitalWrite(STBY, LOW);
  delay(10);
}

void puff(SIDE s, int t) {
  if (s == LEFT || s == BOTH)
    digitalWrite(LPIN, HIGH);
  if (s == RIGHT || s == BOTH)
    digitalWrite(RPIN, HIGH);
  
  delay(t);
  
  digitalWrite(LPIN, LOW);
  digitalWrite(RPIN, LOW);
}

void dispense(float ul) {  // in microliters
  dispense_by_t(ul * UL_PER_MS);
}

void dispense_by_t(int t) {  // in milliseconds
  digitalWrite(VPIN, HIGH);
  delay(t);
  digitalWrite(VPIN, LOW);
}

void prefeed_tube(float tube_id, float tube_len) {  // inner diameter and len in mm
  float vol = (tube_id / 2.0) * (tube_id / 2.0) * PI * tube_len;  // mm3
  dispense(vol);  // 1mm3 is 1ul
}

float calibrate_dispenser() {
  Serial.print("Provide dispense time in ms: ");

  while (!Serial.available())
    delay(500);
  
  if (Serial.available() > 0) {
    int rlen = Serial.readBytes(BUFFER, BUFFER_SIZE);
    Serial.print(BUFFER);
    int t = atoi(BUFFER);
    dispense_by_t(t);
    Serial.println("DONE");
  }
}

void setup() {
  
  pinMode(LPIN, OUTPUT);
  pinMode(RPIN, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(VPIN, OUTPUT);

  digitalWrite(LPIN, LOW);
  digitalWrite(RPIN, LOW);
  digitalWrite(STBY, LOW);
  digitalWrite(VPIN, LOW);

  Serial.begin(9600);
  
}

void loop() {
  
  calibrate_dispenser();
  
//  on();
//  Serial.println("L");
//  puff(BOTH, 2000);
//  delay(1000);
//  
//  Serial.println("R");
//  puff(RIGHT, 2000);
//  off();
//  delay(1000);
}
