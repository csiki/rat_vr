
enum SIDE {LEFT, RIGHT, BOTH};  // where to puff from
const byte DEVICE_ID = 2;

// msg is received in 2 bytes: first is the msg type, then the msg data containing the load,
//   the load is in microliters for DISPENSE, and in ms for the puffs
// the received/processed msg type id is written back to serial after any kind of returned load
enum MSG_TYPE {NOP = 0, GET_DEV_ID = 1, PREFEED = 2, DISPENSE = 3,
               LEFT_PUFF = 4, RIGHT_PUFF = 5, BOTH_PUFF = 6};
const int DATA_WAIT_LIMIT = 1000;  // ~ms, after this, data part of the msg is ignored

const int LPIN = 4, RPIN = 3;  // L/R puff drive: green, yellow
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
    return -1;  // error

  return Serial.read();
  
}


MSG_TYPE proc_msg(MSG_TYPE msg_type) {

  int msg_data = read_msg_data();  // even NOP needs to have a load

  if (msg_data == -1)  // data load reading error
    return NOP;  // skip
  
  switch (msg_type) {

    case GET_DEV_ID:
      Serial.write(DEVICE_ID);
      break;
    
    case PREFEED:
      prefeed_tube(6, 533);  // thick tube: 21" = 533mm
      prefeed_tube(2, 100);  // short tiny tube: <4" = 100mm
      prefeed_tube(2, 647);  // long tiny tube: 23" 1/4 = 647mm
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

  return msg_type;
  
}


void setup() {

  Serial.begin(115200);
  
  pinMode(LPIN, OUTPUT);
  pinMode(RPIN, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(VPIN, OUTPUT);

  digitalWrite(LPIN, LOW);
  digitalWrite(RPIN, LOW);
  digitalWrite(STBY, LOW);
  digitalWrite(VPIN, LOW);
  
}


void loop() {

  // run
  if (Serial.available() > 0) {
    MSG_TYPE msg_type = (MSG_TYPE) Serial.read();
    msg_type = proc_msg(msg_type);
    Serial.write((byte) msg_type);  // return received msg to master
  }
  
  // calibrate_dispenser();
  // test puffs
  //on();
  //Serial.println("left");
  //puff(LEFT, 1000);
  //delay(500);
  //Serial.println("right");
  //puff(RIGHT, 1000);
  //delay(500);
  //Serial.println("both");
  //puff(BOTH, 1000);
  //off();

  // test prefeed
//  if (ccc == 0) {
//    
//    Serial.println("thick tube start");
//    prefeed_tube(6, 533);  // thick tube: 21" = 533mm
//    Serial.println("thick tube done");
//    delay(8000);
//
//    Serial.println("smol tube start");
//    prefeed_tube(2, 100);  // short tiny tube: <4" = 100mm
//    Serial.println("smol tube done");
//    delay(8000);
//
//    Serial.println("smol tube2 start");
//    prefeed_tube(2, 647);  // long tiny tube: 23" 1/4 = 647mm
//    Serial.println("smol tube2 done");
//    delay(8000);
//    
//    ccc += 1;
//  }

  //dispense(50);
  
  //delay(500);
  
}
