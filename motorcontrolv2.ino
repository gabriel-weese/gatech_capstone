//DIPS on TB6600: on, on, off, on, off, off -> steps/rev = 200, 2A current
//theoretically: f=1/(540us*2) -> rps = f/(steps/rev) = 4.629rev/s


// define motor controller pins
const int stepPin = 7;
const int dirPin = 9;
const int enPin = 6;
const int dcmotor_in1 = 8;
const int dcmotor_in2 = 9;

// define button/switch pins
const int stopCheck = 2;
const int upExtend = 12;
const int downRetract = 13;
const int emergency = 3;

// define stepper motor variables
const int delay_period = 540; //period of mock PWM wave going into stepper motor driver
const int steps_per_loop = 200; //number of steps needed on stepper for one revolution
const int revolutions = 15;

// define dc motor variables
const int dc_timing_up = 5000; //number of milliseconds to let dc motor on linear actuator to go up
int last_dc1_signal = 0;
int last_dc2_signal = 0;

// define emergency button variable
bool stop_flag = false;

void setup() {
  // initialize stepper motor pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  digitalWrite(dirPin, LOW);

  // initialize dc motor pins
  pinMode(dcmotor_in1,OUTPUT);
  pinMode(dcmotor_in2, OUTPUT);
  digitalWrite(dcmotor_in1, LOW);
  digitalWrite(dcmotor_in2, LOW);
  
  // initialize button/switch pins
  pinMode(upExtend,INPUT);
  pinMode(downRetract,INPUT);
  pinMode(emergency,INPUT);
  pinMode(stopCheck, INPUT);
  
  Serial.begin(9600);
  Serial.println("Starting...");
  attachInterrupt(digitalPinToInterrupt(emergency),emergencyStop,RISING);
}

int emergencyStop(){
  // save last dc motor control to be able to resume where last left off
  last_dc1_signal = digitalRead(dcmotor_in1);
  last_dc2_signal = digitalRead(dcmotor_in2);
  digitalWrite(dcmotor_in1, LOW);
  digitalWrite(dcmotor_in2, LOW);
  digitalWrite(stepPin, LOW);
  stop_flag = true;
  Serial.println("Emergyency pressed");
  // don't exit emergency state until one button is pressed
  while(digitalRead(upExtend) == LOW && digitalRead(downRetract)== LOW){
    continue;
  }
  delay(0.3);
  digitalWrite(dcmotor_in1, last_dc1_signal);
  digitalWrite(dcmotor_in2, last_dc2_signal);
}

// extend tongue and then go up
int upFunction(){
  digitalWrite(dirPin, HIGH);
  runStepper();
  Serial.println("Going up");
  digitalWrite(dcmotor_in1,HIGH);
  digitalWrite(dcmotor_in2, LOW);
  delay(dc_timing_up);
  digitalWrite(dcmotor_in1,LOW);
}

// go down and then retract tongue
int downFunction(){
  Serial.println("Going down");
  digitalWrite(dcmotor_in1,LOW);
  digitalWrite(dcmotor_in2, HIGH);
  while (!digitalRead(stopCheck)) {
    continue;
  }
  digitalWrite(dcmotor_in2,LOW);
  digitalWrite(dirPin, LOW);
  runStepper();
}

// sends mock PWM wave to stepper motor driver
int runStepper(){
  Serial.println("Running Stepper");
  for (int x = 0; x < steps_per_loop*revolutions; x++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(delay_period);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(delay_period);
    }
}

void loop() {
  // read buttons that control SABER
  int upExtendState = digitalRead(upExtend);
  int downRetractState= digitalRead(downRetract);
  if (upExtendState == HIGH) {
    Serial.println("btn up");
    upFunction();
  }
  else if (downRetractState == HIGH) {
    Serial.println("btn down");
    downFunction();
  }
  else {
    Serial.println("else\n");
    digitalWrite(dcmotor_in1, LOW);
    digitalWrite(dcmotor_in2, LOW);
  }
}