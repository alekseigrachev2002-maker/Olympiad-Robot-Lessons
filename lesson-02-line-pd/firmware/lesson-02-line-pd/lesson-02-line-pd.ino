// УРОК 2: датчики линии и ПД-регулятор

#define L_DIR 7   // левый мотор – канал M2
#define L_PWM 6
#define R_DIR 4   // правый мотор – канал M1
#define R_PWM 5

#define LINE_L A0  // левый датчик линии (Troyka, S-V-G)
#define LINE_R A1  // правый датчик линии

void setup() {
  pinMode(L_DIR, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(R_DIR, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  Serial.begin(115200);  // такая же скорость – в плоттере
}

// направление вращения: HIGH или LOW; invDir – поменять направление на обратное
bool direction(int16_t speed, bool invDir) {
  bool dir = (speed > 0) ? true : false;
  return (invDir) ? !dir : dir;
}

// моторы стоят зеркально и подключены одинаково, поэтому левый для движения вперед
// крутится в обратную сторону – invLDir = true (ответ к TODO 1 урока 1)
void motors(int16_t left, int16_t right, bool invLDir = true, bool invRDir = false) {
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);
  digitalWrite(L_DIR, direction(left, invLDir));
  digitalWrite(R_DIR, direction(right, invRDir));
  analogWrite(L_PWM, abs(left));
  analogWrite(R_PWM, abs(right));
}

// TODO 1: калибровка – робот стоит, датчики над белым, потом над черным; запишите показания
void sensors() {  // для плоттера: левый, правый, ошибка
  Serial.print(analogRead(LINE_L));
  Serial.print(' ');
  Serial.print(analogRead(LINE_R));
  Serial.print(' ');
  Serial.println(analogRead(LINE_L) - analogRead(LINE_R));
}

// один шаг регулятора; вызывается раз в 10 мс (delay(10) в loop) – kd подбирается под этот шаг
void followLine() {
  const float speed = 150;  // базовая скорость
  const float kp = 0.0;     // TODO 3: поднимайте до раскачки на прямой, затем спуститесь по повороту
  const float kd = 0.0;     // TODO 4: гасит раскачку, робот снова вписывается в поворот

  static float errOld = 0;  // ошибка на прошлом шаге: static – не стирается между вызовами
  // на белом датчик выдает больше, чем на черном: линия слева → L меньше → err < 0
  float err = analogRead(LINE_L) - analogRead(LINE_R);
  float control = kp * err + kd * (err - errOld);
  // TODO 2: проверьте знак – при err < 0 (линия слева) левое колесо должно крутиться медленнее
  motors(speed + control, speed - control);
  errOld = err;
}

void loop() {
  sensors();         // калибровка (TODO 1); перед ездой закомментируйте – вывод в порт тормозит цикл
  // followLine();   // езда по линии (TODO 2-4)
  delay(10);
}
