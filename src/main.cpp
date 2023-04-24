/* https://html.alldatasheet.com/html-pdf/12198/ONSEMI/74HC595/181/1/74HC595.html (HOJA DE DATOS DEL 74HC595)

 * CODIGO CON :
 * - I2C
 * - 74hc595
 * - Bluetooth
 * - Servos
 * 
 * - Cuenta regresiva
 * 
 * NO TIENE:
 * - Cuenta general
 *
 *  
 * PROBADO
 * - funcionamiento de leds 
 * - funcionamiento de pulsadores 
 * NO FUNCIONA 
 * - El incremento de viajes
 *
 * FALTA
 * - servos
 * - bluetooth
 * - infras
 * 
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>
#include <Servo.h>

#define infra1 A0
/*#define infra2 A1
#define infra3 A2
#define infra4 A3
#define infra5 0*/

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
volatile int estadoIncremento;

volatile int flagRegresion = 0;
volatile int contadorViajes = 0;

volatile int activacionJuego = 0; // flag para iniciar el juego
volatile int aleatorio = 0;
volatile int numAnterior;

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
  //Mensaje de bienvenida
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
 /* pinMode(infra2, INPUT);
  pinMode(infra3, INPUT);
  pinMode(infra4, INPUT);
  pinMode(infra5, INPUT);*/

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

void loop(){

  while(aceptacion == 0){
    /*
    * Este mensaje solo debe aparecer una vez 
    * para eso se utiliza la flag aceptacion
    * el estado de esta flag se cambia en cantViajes()
    */
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cant de viajes:");
    cantViajes();
  }
  switch (flagRegresion){
    /*
     * flagRegresion se utiliza para ir avanzando en los pasos del programa 
    */
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("El juego inicia");
      lcd.setCursor(0, 1);
      lcd.print("     en: ");
      flagRegresion = 1;
    break;
    case 1:
      /*
       * La funcion cuentaRegresiva empieza a interrumpirse con el timer a partir de ahora
       * 
       * tambien se muestra en pantalla
      */
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
      if (digitalRead(infra1) == LOW)
      {
        delay(500); // retencion del pulsador
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

void cantViajes(){
  /* Al pulsar el boton de incremento se aumenta la cantidad de viajes
   *
   * Si se pulsa el boton inicio se termina la configuracion de cantidad de viajes e inicia la cuenta regresiva
  */
  while(digitalRead(inicio) == HIGH){

    estadoIncremento = digitalRead(incremento);
    if (estadoIncremento == HIGH){

      delay(500);  // Este delay hay que sacarlo en un futuro, es para la retencion del pulsador
      numViajes++; // Esto se podria hacer de 5 en 5
      
      lcd.setCursor(0, 1);
      lcd.print(numViajes);
    }
  }

  if (digitalRead(inicio) == LOW)
  {
    delay(500);
    aceptacion = 1;
    lcd.clear(); // el clear esta aca para que se ejecute solo una vez
  }
}
void juego(){
  /* 
   * Genera un numero aleatorio, el cuel no puede ser igual al anterior
   * 
   * Dependiendo de ese numero se marca que salida debe tener el 74hc595
   * 
   * Los numeros en binario tienen un unico cero dentro de los primeros 5 digitos
   * 
   * Se ulizan las salidas QA, QB, QC, QD, QE del 74hc595
   * 
   * activacionJuego es una flag que se modifica para salir de la funcion juego y volver al codigo principal
  */

  do{
    aleatorio = random(0, 5);
  }while(aleatorio == numAnterior);
  
  switch (aleatorio)
  {
    case 0:
      digitalWrite(pinLatch, LOW);              
      shiftOut(dataPin, clockPin, MSBFIRST, 1); 
      digitalWrite(pinLatch, HIGH);
      numAnterior = 0;
      activacionJuego = 1;
    break;
    case 1:
      digitalWrite(pinLatch, LOW);              
      shiftOut(dataPin, clockPin, MSBFIRST, 2); 
      digitalWrite(pinLatch, HIGH);
      numAnterior = 1;
      activacionJuego = 1;
    break;
    case 2:
      digitalWrite(pinLatch, LOW);              
      shiftOut(dataPin, clockPin, MSBFIRST, 4); 
      digitalWrite(pinLatch, HIGH);
      numAnterior = 2;
      activacionJuego = 1;
    break;
    case 3:
      digitalWrite(pinLatch, LOW);              
      shiftOut(dataPin, clockPin, MSBFIRST, 8);
      digitalWrite(pinLatch, HIGH);
      numAnterior = 3;
      activacionJuego = 1;
    break;
    case 4:
      digitalWrite(pinLatch, LOW);               
      shiftOut(dataPin, clockPin, MSBFIRST, 16); 
      digitalWrite(pinLatch, HIGH);
      numAnterior = 4;
      activacionJuego = 1;
    break;
  }
}
void cuentaRegresiva()
{
  /* Esto funciona de forma interrumpida por el Timer1 cada 1seg
   *
   * regresion sera la cuenta propiamente dicho
   * cuando llegue a 0 se cambiara flagRegresion y ya no se volvera a esta funcion (se utiliza <=0 porque sino empieza a mostrar numeros negativos)
  */
  regresion = 5 - (aux++);

  if (regresion <= 0)
  {
    flagRegresion = 2;
  }
}
void finDelJuego(){
/*Es el mensaje que se mostrara al finalizar el juego
  
  Aca tambien se pueden agregar para mostrar la cantidad de veces que se toco cada dedo y la cuenta general
*/
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Felicidades!  ");
  lcd.setCursor(0, 1);
  lcd.print(" Fin del juego! ");
  delay(2000); // hay que sacar estos delay
}