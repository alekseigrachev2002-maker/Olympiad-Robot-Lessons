// УРОК 3: перекрестки

#define L_DIR 7   // левый мотор – канал M2
#define L_PWM 6
#define R_DIR 4   // правый мотор – канал M1
#define R_PWM 5

#define LINE_L A0  // левый датчик линии
#define LINE_R A1  // правый датчик линии

#define THRESHOLD 0  // TODO 1: порог между белым и черным; оба датчика ниже порога – перекресток
#define STOP_AT 3    // на каком перекрестке остановиться

void setup() {
  pinMode(L_DIR, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(R_DIR, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  Serial.begin(115200);
}

bool direction(int16_t speed, bool invDir) {  // с урока 2
  bool dir = (speed > 0) ? true : false;
  return (invDir) ? !dir : dir;
}

void motors(int16_t left, int16_t right, bool invLDir = false, bool invRDir = true) {  // с урока 2
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);
  digitalWrite(L_DIR, direction(left, invLDir));
  digitalWrite(R_DIR, direction(right, invRDir));
  analogWrite(L_PWM, abs(left));
  analogWrite(R_PWM, abs(right));
}

void sensors() {  // с урока 2: для плоттера – левый, правый, ошибка
  Serial.print(analogRead(LINE_L));
  Serial.print(' ');
  Serial.print(analogRead(LINE_R));
  Serial.print(' ');
  Serial.println(analogRead(LINE_L) - analogRead(LINE_R));
}

void followLine() {  // с урока 2
  const float speed = 150;
  const float kp = 0.0;  // перенесите свои kp и kd из урока 2
  const float kd = 0.0;
  static float errOld = 0;
  float err = analogRead(LINE_L) - analogRead(LINE_R);
  float control = kp * err + kd * (err - errOld);
  motors(speed + control, speed - control);
  errOld = err;
}

// возвращает, сколько перекрестков робот уже проехал
uint8_t crossRoad() {
  bool isCross = analogRead(LINE_L) < THRESHOLD && analogRead(LINE_R) < THRESHOLD;
  static bool wasCross = false;  // был ли робот на перекрестке на прошлом шаге
  uint8_t count = 0;             // TODO 2: счетчик обнуляется при каждом вызове функции
  if (isCross) count++;          // TODO 3: +1 только в момент заезда на перекресток
  // TODO 4: запомните текущее состояние для следующего вызова
  return count;
}

void loop() {
  // sensors();  // посмотреть показания на черной линии (TODO 1)
  // TODO 5: по линии до 3-го перекрестка, затем стоп навсегда
  followLine();
  delay(10);
}
