// УРОК 1: питание и движение
// Iskra Uno + Motor Shield: у каждого мотора пин направления (HIGH / LOW) и пин скорости (ШИМ 0-255)

#define L_DIR 7   // левый мотор – канал M2
#define L_PWM 6
#define R_DIR 4   // правый мотор – канал M1
#define R_PWM 5

void setup() {
  pinMode(L_DIR, OUTPUT);  // все пины управляют нагрузкой – режим OUTPUT
  pinMode(L_PWM, OUTPUT);
  pinMode(R_DIR, OUTPUT);
  pinMode(R_PWM, OUTPUT);
}

// left, right – скорость от -255 до 255, знак задает направление вращения
void motors(int16_t left, int16_t right) {
  left = constrain(left, -255, 255);    // ШИМ не бывает больше 255
  right = constrain(right, -255, 255);
  digitalWrite(L_DIR, left > 0);        // TODO 1: робот крутится на месте – исправьте направление левого мотора
  digitalWrite(R_DIR, right > 0);       // направление: true – HIGH, false – LOW
  analogWrite(L_PWM, abs(left));        // скорость – модуль числа
  analogWrite(R_PWM, abs(right));
}

void loop() {
  // TODO 1: оба колеса должны крутиться вперед
  motors(150, 150);
  delay(1500);

  // TODO 2: подберите скорости левого и правого мотора, чтобы робот ехал прямо
  // TODO 3: разворот на месте на 90 градусов – колеса крутятся в разные стороны, время подберите
  // TODO 4 (дополнительно): квадрат – четыре раза «прямо + поворот на 90 градусов», используйте цикл for

  motors(0, 0);
  while (true) {}  // стоп навсегда: loop() больше не повторится
}
