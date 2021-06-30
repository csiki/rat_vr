
enum ROLLER {FORWARD_BACKWARD = 0, LEFT_RIGHT = 1};
enum DIRECTION {BACKWARD = 0, FORWARD = 1, LEFT = 0, RIGHT = 1};
const byte DEVICE_ID = 1;

// msg is received in 3 bytes: first is the msg type, then the msg data containing the load
// the load is 3 bytes, first byte is separate, second and two bytes are concatenated MSB first:
//   PULL_TO_BASE load is ignored; lin extension/contraction, and solenoid pull loads are in ms and first byte is ignored
//   motor rolls get speed (0-255) as first byte and time to roll in ms as the concat of last two bytes
// the received/processed msg type id is written back to serial after any kind of returned load
enum MSG_TYPE {NOP = 0, GET_DEV_ID = 1, GET_BTN_STATE = 2, PULL_TO_BASE = 3, EXTEND_FB_LIN = 4, CONTRACT_FB_LIN = 5,
               EXTEND_LR_LIN = 6, CONTRACT_LR_LIN = 7, ROLL_FORWARD = 8, ROLL_BACKWARD = 9,
               ROLL_LEFT = 10, ROLL_RIGHT = 11, PULL_SOL = 12};
const int DATA_WAIT_LIMIT = 1000;  // ~ms, after this, data part of the msg is ignored

// roller stuff
typedef struct {
  const int lin1, lin2;  // lin act: grey, white (Ain1, Ain2)
  const int min1, min2;  // motor: brown, blue (Bin1, Bin2) (be sure it's the right brown/blue)
  const int pwmm;  // motor pwm: yellow
  const int stby;  // enable: orange
  int lin_state;
} roller_t;

roller_t ROLLERS[] = {{lin1: 5, lin2: 4, min1: 6, min2: 7, pwmm: 3, stby: 2, lin_state: 0},  // forward-backward (pitch)
                      {lin1: 11, lin2: 10, min1: 12, min2: 13, pwmm: 9, stby: 8, lin_state: 0}};  // left-right (yaw)

const int MAX_LIN_EXT = 4000, MAX_LIN_INACC = 200;
const int FB_EXT = 800, LR_EXT = 1100;
const int MAX_LIN_PRAC_EXT = max(FB_EXT, LR_EXT) + MAX_LIN_INACC;


// pull solenoid & button analog input pins
const int SOL_PIN = A0;
const int BTN_PIN = A1;
int BTN_STATE = 0;


// pull solenoid & button functions
void sol_pull(int keep_t) {  // keep in pulled position in ms
  
  const int state_t = 150;  // ms
  const int nstate = 20;
  const int act_res = 5;
  const int atom_t = state_t / nstate;

  digitalWrite(SOL_PIN, LOW);  // safety
  
  for (int state = 1; state <= nstate; ++state) {
    int act_t = atom_t * state;
    int nact = act_t / act_res;
    int inact_delay = (state_t - act_t) / nact;

    for (int act_i = 0; act_i < nact; ++act_i) {
      digitalWrite(SOL_PIN, LOW);
      delay(inact_delay);
      
      digitalWrite(SOL_PIN, HIGH);  // ends with high
      delay(act_res);
    }
  }

  delay(keep_t);
  digitalWrite(SOL_PIN, LOW);  // back to ernyett
}


// roller functions
void on(ROLLER r) {
  digitalWrite(ROLLERS[r].stby, HIGH);
  delay(1);
}

void off(ROLLER r) {
  digitalWrite(ROLLERS[r].stby, LOW);
  delay(1);
}

void roll(int in1, int in2, DIRECTION d, int t) {
  if (d == FORWARD) {
    digitalWrite(in2, LOW);
    digitalWrite(in1, HIGH);
    delay(t);
    digitalWrite(in1, LOW);
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    delay(t);
    digitalWrite(in2, LOW);
  }
}

void drive_lin(ROLLER r, DIRECTION d, int t) {
  roll(ROLLERS[r].lin1, ROLLERS[r].lin2, d, t);
  ROLLERS[r].lin_state = max(0, ROLLERS[r].lin_state + (d * 2 - 1) * t);
  // Serial.println(ROLLERS[r].lin_state);
}

void roll_mot(ROLLER r, DIRECTION d, int t) {  // no pwm
  digitalWrite(ROLLERS[r].pwmm, HIGH);
  roll(ROLLERS[r].min1, ROLLERS[r].min2, d, t);
  digitalWrite(ROLLERS[r].pwmm, LOW);
}

void prec_roll_mot(ROLLER r, DIRECTION d, byte sp, int t) {
  analogWrite(ROLLERS[r].pwmm, sp);
  roll(ROLLERS[r].min1, ROLLERS[r].min2, d, t);
  digitalWrite(ROLLERS[r].pwmm, LOW);
}

void pull_to_base(ROLLER r) {
  int by = min(MAX_LIN_PRAC_EXT, max(0, ROLLERS[r].lin_state) + MAX_LIN_INACC);
  drive_lin(r, BACKWARD, by);
  ROLLERS[r].lin_state = 0;
}

void test_roller(ROLLER r, int t) {
  on(r);
  
  drive_lin(r, FORWARD, t);
  
  roll_mot(r, BACKWARD, 1500);
  roll_mot(r, FORWARD, 1000);
  
  drive_lin(r, BACKWARD, t);
  
  pull_to_base(r);
  
  off(r);
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

  byte msg_data1 = read_msg_data();  // even NOP needs to have a load
  int msg_data2 = read_msg_data() << 8;  // MSB first
  msg_data2 |= read_msg_data();

  if (msg_data2 == -1)  // data load reading error; msg_data1 can be -1 (=255)
    return NOP;  // skip
  
  switch (msg_type) {

    case GET_DEV_ID:
      Serial.write(DEVICE_ID);
      break;
    
    case GET_BTN_STATE:
      Serial.write((byte) digitalRead(BTN_PIN));
      break;

    case PULL_TO_BASE:
      for (int r = FORWARD_BACKWARD; r <= LEFT_RIGHT; ++r) {
        on((ROLLER) r);
        ROLLERS[r].lin_state = MAX_LIN_PRAC_EXT;
        pull_to_base((ROLLER) r);  // to 0 state
        off((ROLLER) r);
      }
      break;
    
    case EXTEND_FB_LIN:
      on(FORWARD_BACKWARD);
      drive_lin(FORWARD_BACKWARD, FORWARD, msg_data2);
      off(FORWARD_BACKWARD);
      break;

    case CONTRACT_FB_LIN:
      on(FORWARD_BACKWARD);
      drive_lin(FORWARD_BACKWARD, BACKWARD, msg_data2);
      off(FORWARD_BACKWARD);
      break;

    case EXTEND_LR_LIN:
      on(LEFT_RIGHT);
      drive_lin(LEFT_RIGHT, FORWARD, msg_data2);
      off(LEFT_RIGHT);
      break;

    case CONTRACT_LR_LIN:
      on(LEFT_RIGHT);
      drive_lin(LEFT_RIGHT, BACKWARD, msg_data2);
      off(LEFT_RIGHT);
      break;

    case ROLL_FORWARD:
      on(FORWARD_BACKWARD);
      prec_roll_mot(FORWARD_BACKWARD, FORWARD, msg_data1, msg_data2);
      off(FORWARD_BACKWARD);
      break;

    case ROLL_BACKWARD:
      on(FORWARD_BACKWARD);
      prec_roll_mot(FORWARD_BACKWARD, BACKWARD, msg_data1, msg_data2);
      off(FORWARD_BACKWARD);
      break;

    case ROLL_LEFT:
      on(LEFT_RIGHT);
      prec_roll_mot(LEFT_RIGHT, LEFT, msg_data1, msg_data2);
      off(LEFT_RIGHT);
      break;

    case ROLL_RIGHT:
      on(LEFT_RIGHT);
      prec_roll_mot(LEFT_RIGHT, RIGHT, msg_data1, msg_data2);
      off(LEFT_RIGHT);
      break;

    case PULL_SOL:
      sol_pull(msg_data2);
      break;
  }

  return msg_type;
  
}


void setup() {

  Serial.begin(115200);
  
  for (int r = FORWARD_BACKWARD; r <= LEFT_RIGHT; ++r) {

    // setup roller pins
    pinMode(ROLLERS[r].lin1, OUTPUT);
    pinMode(ROLLERS[r].lin2, OUTPUT);
    pinMode(ROLLERS[r].min1, OUTPUT);
    pinMode(ROLLERS[r].min2, OUTPUT);
    pinMode(ROLLERS[r].pwmm, OUTPUT);
    pinMode(ROLLERS[r].stby, OUTPUT);

    digitalWrite(ROLLERS[r].lin1, LOW);
    digitalWrite(ROLLERS[r].lin2, LOW);
    digitalWrite(ROLLERS[r].min1, LOW);
    digitalWrite(ROLLERS[r].min2, LOW);
    digitalWrite(ROLLERS[r].pwmm, LOW);
    digitalWrite(ROLLERS[r].stby, LOW);

    // setup pull solenoid & button pins
    pinMode(SOL_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT);
    digitalWrite(SOL_PIN, LOW);

    // pull linear actuator back fully
    //Serial.println("pull to base");
    on((ROLLER) r);
    ROLLERS[r].lin_state = MAX_LIN_PRAC_EXT;
    pull_to_base((ROLLER) r);  // to 0 state
    off((ROLLER) r);

  }
}


void loop() {

  if (Serial.available() > 0) {
    MSG_TYPE msg_type = (MSG_TYPE) Serial.read();
    msg_type = proc_msg(msg_type);
    Serial.write((byte) msg_type);  // return received msg to master
  }
  
  //test_roller(FORWARD_BACKWARD, FB_EXT);
  //test_roller(LEFT_RIGHT, LR_EXT);
  
  //Serial.println("solenoid pull");
  //sol_pull(2000);
  //delay(2000);

  //BTN_STATE = digitalRead(BTN_PIN);
  //Serial.println(BTN_STATE);
  //delay(50);
  
}
