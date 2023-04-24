/*
   CODIGO CON LOS SERVOS, BLUETOOTH, INFRAS, 74HC595, LCD CON I2C Y CUENTA REGRRESIVA
   SOLO HAY QUE ARREGLAR QUE SE BORRE LA CUENTA REGRESIVA CUANDO TERMINE
   HAY QUE PROBAR EN FISICO PARA SABER LA CONFIGURACION DE  LOS SERVOS Y S LA PARTE DE LOS LEDS ALEATORIOS FUNCIONA
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>
#include <Servo.h>

#define infra1 A0
#define infra2 A1
#define infra3 A2
#define infra4 A3
#define infra5 0
// Comentario para prueba 2

#define pinLatch 8  // es la patita SH_CP
#define clockPin 13 // es la patita DS
#define dataPin 12  // es la patita ST_CP

#define incremento 5
#define inicio 4

Servo miservo_1; // servo 1 derecha izquierda
Servo miservo_2; // Servo 2 y 3 hacen lo msimo por que es para estabilizacion
Servo miservo_3; // servo 2 y 3 arriab y abajo

LiquidCrystal_I2C lcd(0x3F, 16, 2);

volatile int numViajes = 0;
volatile int aceptacion = 0;

volatile int flagRegresion = 0;
volatile int contadorViajes = 0;

volatile int activacionJuego = 0; // flag para iniciar el juego
volatile int aleatorio = 0;
volatile byte myByte = 0;

volatile int regresion;
volatile int aux = 0;

int grados = 90;
int state = 0;

void cantViajes();
void cuentaRegresiva();
void juego();
void finDelJuego();

void setup()
{

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("  Bienvenido a  ");
  lcd.setCursor(0, 1);
  lcd.print("  Super Guanti  ");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Una creacion de:");
  lcd.setCursor(0, 1);
  lcd.print("     M.A.L.     ");
  delay(1000);

  pinMode(incremento, INPUT);
  pinMode(inicio, INPUT);

  pinMode(infra1, INPUT);
  pinMode(infra2, INPUT);
  pinMode(infra3, INPUT);
  pinMode(infra4, INPUT);
  pinMode(infra5, INPUT);
  pinMode(pinLatch, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  miservo_1.attach(3, 750, 1800); // EL 2 ES EL PIN DONDE ESTA CONECTADO EL 750 ES EL 0 Y EL 1800 POR LOS 180°
  miservo_1.write(grados);

  miservo_2.attach(5, 750, 1800); // EL 2 ES EL PIN DONDE ESTA CONECTADO EL 750 ES EL 0 Y EL 1800 POR LOS 180°
  miservo_2.write(grados);

  miservo_3.attach(6, 750, 1800); // EL 2 ES EL PIN DONDE ESTA CONECTADO EL 750 ES EL 0 Y EL 1800 POR LOS 180°
  miservo_3.write(grados);
}

void loop()
{

  if (aceptacion == 0)
  {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cant de viajes:");
    cantViajes();
  }
  else
  {

    switch (flagRegresion)
    {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("El juego inicia");
      lcd.setCursor(0, 1);
      lcd.print("     en: ");
      flagRegresion = 1;
      break;
    case 1:
      Timer1.initialize(1000000); // 1s
      Timer1.attachInterrupt(cuentaRegresiva);

      lcd.setCursor(10, 1);
      lcd.print(regresion);
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("    A JUGAR!    ");
      lcd.setCursor(0, 1);
      lcd.print("                ");

      if (Serial.available() > 0)
      {

        state = Serial.read();
        Serial.write(state);
      }

      if (state == '1')
      {

        grados++;

        if (grados >= 180)
        { // Protege el sero de no sobreexigirlos

          grados = 180;
        }
        miservo_1.write(grados);
        delay(10);
        state = 0;
      }

      if (state == '2')
      {

        grados--;
        if (grados <= 0)
        { // Protege el servo

          grados = 0;
        }
        miservo_1.write(grados);
        delay(10);
      }
      if (state == '3')
      {

        grados++;

        if (grados >= 180)
        { // Protege el sero de no sobreexigirlos

          grados = 180;
        }
        miservo_2.write(grados);
        delay(10);

        miservo_3.write(grados);
        delay(10);
      }
      if (state == '4')
      {

        grados--;
        if (grados <= 0)
        { // Protege el servo

          grados = 0;
        }
        miservo_2.write(grados);
        delay(10);

        miservo_3.write(grados);
        delay(10);
      }
      if (activacionJuego == 0)
      {
        juego();
      }
      if (digitalRead(infra1) == LOW || digitalRead(infra2) == LOW || digitalRead(infra3) == LOW || digitalRead(infra4) == LOW || digitalRead(infra5) == LOW)
      {
        delay(300); // retencion del pulsador
        contadorViajes++;

        if (contadorViajes < numViajes)
        {
          activacionJuego = 0;
        }
        if (contadorViajes >= numViajes)
        {
          lcd.clear();
          finDelJuego();
        }
      }
      break;
    }
  }
}

void cantViajes()
{

  do
  {

    if (digitalRead(incremento) == LOW)
    {

      numViajes++; // Esto se podria hacer de 5 en 5
      delay(300);  // Este delay hay que sacarlo en un futuro, es para la retencion del pulsador
      lcd.setCursor(0, 1);
      lcd.print(numViajes);
    }

  } while (digitalRead(inicio) == HIGH);

  if (digitalRead(inicio) == LOW)
  {
    delay(300);
    aceptacion = 1;
    lcd.clear(); // el clear esta aca para que se ejecute solo una vez
  }
}
void juego()
{

  aleatorio = random(0, 5);

  switch (aleatorio)
  {
  case 0:
    digitalWrite(pinLatch, LOW);              // sube cada 8 ciclos
    shiftOut(dataPin, clockPin, MSBFIRST, 1); // le mandamos cuatro datos primero la data despues el pin del reloj , el bit menos significativo
    digitalWrite(pinLatch, HIGH);

    activacionJuego = 1;
    break;
  case 1:
    digitalWrite(pinLatch, LOW);              // sube cada 8 ciclos
    shiftOut(dataPin, clockPin, MSBFIRST, 2); // le mandamos cuatro datos primero la data despues el pin del reloj , el bit menos significativo
    digitalWrite(pinLatch, HIGH);

    activacionJuego = 1;
    break;
  case 2:
    digitalWrite(pinLatch, LOW);              // sube cada 8 ciclos
    shiftOut(dataPin, clockPin, MSBFIRST, 4); // le mandamos cuatro datos primero la data despues el pin del reloj , el bit menos significativo
    digitalWrite(pinLatch, HIGH);

    activacionJuego = 1;
    break;
  case 3:
    digitalWrite(pinLatch, LOW);              // sube cada 8 ciclos
    shiftOut(dataPin, clockPin, MSBFIRST, 8); // le mandamos cuatro datos primero la data despues el pin del reloj , el bit menos significativo
    digitalWrite(pinLatch, HIGH);

    activacionJuego = 1;
    break;
  case 4:
    digitalWrite(pinLatch, LOW);               // sube cada 8 ciclos
    shiftOut(dataPin, clockPin, MSBFIRST, 16); // le mandamos cuatro datos primero la data despues el pin del reloj , el bit menos significativo
    digitalWrite(pinLatch, HIGH);

    activacionJuego = 1;
    break;
  }
}
void cuentaRegresiva()
{
  /* Esto funciona de forma interrumpida por el Timer1 cada 1seg

     regresion sera la cuenta propiamente dicho
     cuando llegue a 0 se cambiara flagRegresion y ya no se volvera a esta funcion (se utiliza <=0 porque sino empieza a mostrar numeros negativos)
  */
  regresion = 5 - (aux++);

  if (regresion <= 0)
  {
    flagRegresion = 2;
  }
}
void finDelJuego()
{

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Felicidades!  ");
  lcd.setCursor(0, 1);
  lcd.print(" Fin del juego! ");
  delay(2000); // hay que sacar estos delay
}