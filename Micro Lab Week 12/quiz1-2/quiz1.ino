#define MOTOR_D1_PIN 7
#define MOTOR_D2_PIN 8
#define MOTOR_PWM_PIN 6
int interruptChannel1APin = 2;
int interruptChannel1BPin = 3;
int delay_count = 0;
int setpoint= 0;
int toggle = 1;
int fb_speed;
String mode;
char pin[10];
int state =0;
volatile int encoderCount = 0;
volatile int encoderDir = 1;
void moveForward (int speed) 
{
  digitalWrite(MOTOR_D1_PIN,HIGH);
  digitalWrite(MOTOR_D2_PIN,LOW);
  analogWrite(MOTOR_PWM_PIN,speed);
}

void moveBackward (int speed) 
{
  digitalWrite(MOTOR_D1_PIN,LOW);
  digitalWrite(MOTOR_D2_PIN,HIGH);
  analogWrite(MOTOR_PWM_PIN,speed);
}
void setSpeed (int speed)
{
  if (speed>0)
  {
    if(speed>100)
    {speed=100;}
    moveForward(speed);
  }
 else if (speed<0)
 {
   speed=speed*(-1);
   if (speed>100)
   {speed=100;}
   moveBackward(speed);
 }
 else
 {moveForward(1);}
}
void Timer1_initialize (int period) {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = (16000000.0 / (256.0 * 1000.0)) * period -1;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}
void setup()
{
  pinMode(MOTOR_D1_PIN,OUTPUT);
  pinMode(MOTOR_D2_PIN,OUTPUT);
  pinMode(MOTOR_PWM_PIN,OUTPUT);
  pinMode(A0, INPUT);
  
  pinMode(interruptChannel1APin,INPUT_PULLUP);
  pinMode(interruptChannel1BPin,INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(interruptChannel1APin),
                 Channel1A_callback, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptChannel1BPin),
                 Channel1B_callback, RISING);
  Serial.begin(9600);
  
  Timer1_initialize(100);
}

int errori = 0;
float kp = 1.0;
float ki = 0.1;

void loop()
{
    int error = setpoint - fb_speed;
  int pid = kp*(float)(error) + ki*(float)(errori);
  errori+=error;
  setSpeed(pid);
  Serial.print(mode);
  Serial.print(setpoint);
  Serial.print(" , ");
  Serial.print(fb_speed);
  Serial.print(" , ");
  Serial.print(-(fb_speed*100.0*2.0*60.0)/5500.0);//Added
  Serial.println(" rpm");//Added
  delay(200);
  
  if(Serial.available()){
    String inString = Serial.readStringUntil('\n');
    inString.toCharArray(pin,10);
    state = 1;
    String buff;
    if(pin[0]=='s')
    {
      int i = 1;
      while(pin[i] != 'n'){
     buff = String(buff + pin[i]);
        i++;
      }
     int temp = buff.toInt();
      if(temp > 0)
      {
      if(temp > 100)
      {
        temp = 100;
      }
      mode = "clockwise :";
      setpoint = temp;
      }
      if(temp < 0)
      {
      if(temp < -100)
      {
        temp = -100;
      }
      mode = "counter clockwise :";
      setpoint = temp;
      }
    }      
  }
}

void Channel1A_callback()
{
  if (digitalRead(interruptChannel1APin)==1 && 
  digitalRead(interruptChannel1BPin)==0)
   {
     encoderCount++;
    //encoderDir = 1;
   }
}

void Channel1B_callback()
{
  if (digitalRead(interruptChannel1APin)==0 && 
  digitalRead(interruptChannel1BPin)==1)
   {
     encoderCount--;
    //encoderDir = 0;
   }
}

ISR(TIMER1_COMPA_vect)
{
  fb_speed = encoderCount;
  encoderCount = 0;
}
