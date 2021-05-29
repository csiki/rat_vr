
enum ROLLER {FORWARD_BACKWARD = 0, LEFT_RIGHT = 1};
enum DIRECTION {BACKWARD = 0, FORWARD = 1, LEFT = 0, RIGHT = 1};

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


// pull solenoid & button analog input pins
const int SOL_PIN = A0;
const int BTN_PIN = A1;
int BTN_STATE = 0;


// pull solenoid & button functions
void sol_pull(int keep) {
  
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

  delay(keep);
}


// roller functions
void on(ROLLER r) {
  digitalWrite(ROLLERS[r].stby, HIGH);
  delay(10);
}

void off(ROLLER r) {
  digitalWrite(ROLLERS[r].stby, LOW);
  delay(10);
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

void roll_lin(ROLLER r, DIRECTION d, int t) {
  roll(ROLLERS[r].lin1, ROLLERS[r].lin2, d, t);
  ROLLERS[r].lin_state = max(0, ROLLERS[r].lin_state + (d * 2 - 1) * t);
  // Serial.println(ROLLERS[r].lin_state);
}

void roll_mot(ROLLER r, DIRECTION d, int t) {  // no pwm
  digitalWrite(ROLLERS[r].pwmm, HIGH);
  roll(ROLLERS[r].min1, ROLLERS[r].min2, d, t);
  digitalWrite(ROLLERS[r].pwmm, LOW);
}

void pull_to_base(ROLLER r) {
  int by = min(MAX_LIN_EXT, max(0, ROLLERS[r].lin_state) + MAX_LIN_INACC);
  roll_lin(r, BACKWARD, by);
  ROLLERS[r].lin_state = 0;
}

void test_roller(ROLLER r, int t) {
  on(r);
  
  roll_lin(r, FORWARD, t);
  
  roll_mot(r, BACKWARD, 1500);
  roll_mot(r, FORWARD, 1000);
  
  roll_lin(r, BACKWARD, t);
  
  pull_to_base(r);
  
  off(r);
}


void setup() {

  Serial.begin(9600);
  
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
    Serial.println("pull to base");
    on((ROLLER) r);
    ROLLERS[r].lin_state = MAX_LIN_EXT;
    pull_to_base((ROLLER) r);  // to 0 state
    off((ROLLER) r);

  }
}


void loop() {
  
  //test_roller(FORWARD_BACKWARD, 2500);
  //test_roller(LEFT_RIGHT, 1500);
  
  //Serial.println("solenoid pull");
  //sol_pull(2000);
  //delay(2000);

  BTN_STATE = digitalRead(BTN_PIN);
  Serial.println(BTN_STATE);
  delay(50);
  
}
