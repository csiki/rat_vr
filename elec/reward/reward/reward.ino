
enum SIDE {LEFT, RIGHT, BOTH};  // where to puff from

// msg is received in 2 bytes: first is the msg type, then the msg data containing the load
// the load is in microliters for DISPENSE, and in ms for the puffs
enum MSG_TYPE {NOP, PREFEED, DISPENSE, LEFT_PUFF, RIGHT_PUFF, BOTH_PUFF};
const int DATA_WAIT_LIMIT = 1000;  // ~ms, after this, data is ignored

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


int read_msg_data() {
  
  int data_read_counter = 0;
  while (Serial.available() == 0 && data_read_counter < DATA_WAIT_LIMIT) {
    delay(1);
    data_read_counter += 1;
  }

  if (data_read_counter >= DATA_WAIT_LIMIT)
    return 0;  // error

  return Serial.read();
  
}


void proc_msg() {

  MSG_TYPE msg_type = NOP;
  int msg_data = 0;
  
  if (Serial.available() > 0) {

    msg_type = Serial.read();
    msg_data = read_msg_data();  // even NOP needs to have a load

    if (msg_data == 0)
      return;  // error
    
    switch (msg_type) {

      case PREFEED:
        prefeed_tube(6, );  // thick tube
        prefeed_tube(2, );  // tiny tube
        break;
      
      case DISPENSE:
        dispense(msg_data);
        break;

      case LEFT_PUFF:
        on();
        puff(LEFT, msg_data);
        off();
        break;

      case RIGHT_PUFF:
        on();
        puff(RIGHT, msg_data);
        off();
        break;

      case BOTH_PUFF:
        on();
        puff(BOTH, msg_data);
        off();
        break;
    }
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

  Serial.begin(115200);
  
}


void loop() {

  // calibrate_dispenser();

  
  
}
