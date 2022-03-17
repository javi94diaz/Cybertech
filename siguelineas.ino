
#include "QTRSensors.h"
#define NUM_SENSORS   8     // number of sensors used
#define TIMEOUT       2500  // waits for 2500 microseconds for sensor outputs to go low
#define EMITTER_PIN   2     // emitter is controlled by digital pin 2

// DEFINE'S  DE BURGOS Y DIAZ

#define vel_alta 150
#define vel_baja 10
#define vel_recta 100
#define deteccion_obstaculo 205
#define delay_salirse 1000
#define delay_recto 900
#define delay_entrar 800

// sensors 0 through 7 are connected to digital pins 3 through 10, respectively
QTRSensorsRC qtrrc((unsigned char[]) {0, 2, 4, 5, 6, 7, 10, 1},
NUM_SENSORS, TIMEOUT, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];


double e;         // error, posicion de la linea
bool carril=1;    //  1 = carril derecho; 0 = carril izquierdo
double vi,vd;   //   velocidades de las ruedas derecha e izquierda

const double V=100; //Velocidad de referencia
const double K=0.025; //Constante de proporcionalidad P (multiplicará al error)

const int velocA = 3;  /* Motor A: derecho */
const int direcA = 12; /* Motor B: izquierdo */
const int frenoA = 9;

const int velocB = 11;
const int direcB = 13;
const int frenoB = 8;


void setup()
{
  delay(500);

  pinMode (A4, INPUT);

  pinMode (velocA, OUTPUT);
  pinMode (direcA, OUTPUT);
  pinMode (frenoA, OUTPUT);

  pinMode (velocB, OUTPUT);
  pinMode (direcB, OUTPUT);
  pinMode (frenoB, OUTPUT);

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);    // turn on Arduino's LED to indicate we are in calibration mode 

  for (int i = 0; i < 400; i++)  // make the calibration take about 10 seconds // la calibracion dura 10 segundos
  {
    qtrrc.calibrate();       // reads all sensors 10 times at 2500 us per read (i.e. ~25 ms per call)
  }
  digitalWrite(13, LOW);     // turn off Arduino's LED to indicate we are through with calibration //


  // print the calibration minimum values measured when emitters were on
  Serial.begin(9600);
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(qtrrc.calibratedMinimumOn[i]);
    Serial.print(' ');
  }
  Serial.println();

  // print the calibration maximum values measured when emitters were on
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(qtrrc.calibratedMaximumOn[i]);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();
  delay(1000);

 //BLOQUEO DE SALIDA (bloqueamos al robot para que no salga, tapando el sensor de distancia de delante[pin analógico 4]. Se desbloquea destapandolo )
  while (analogRead(A4) > 200) { 
    delay(1000);
} 
}


void loop()
{

    Serial.print(analogRead(A4));
    Serial.print('\t');
    Serial.print('\t');
    Serial.print(vi);
    Serial.print('\t');
    Serial.println(vd);

    
  // read calibrated sensor values and obtain a measure of the line position from 0 to 7000
double position = qtrrc.readLine(sensorValues);
  
  // print the sensor values as numbers from 0 to 1000, where 0 means maximum reflectance and
  // 1000 means minimum reflectance, followed by the line position
  for (unsigned char i = 0; i < NUM_SENSORS; i++)
  { 
    Serial.print(sensorValues[i]);
    Serial.print('\t');
  }

  //Calculamos el error, y obtenemos el valor de las velocidades(proporcionales al error, y en referencia a V)
  e=position-3500;
  vi= V-K*e;
  vd= V+K*e;
  //if((sensorValues[1]<=200)&&(sensorValues[2]<=200)&&(sensorValues[3]<=200)&&(sensorValues[4]<=200)&&(sensorValues[5]<=200)&&(sensorValues[6]<=200)) e=0;
  
  Serial.println(e);

  //Bloqueo para adelantar en caso de detectar un obstaculo
   while (analogRead(A4) > deteccion_obstaculo) {  // hemos puesto 'if' y no 'while' para que haga la secuencia de cambio de carril al ver el obstaculo y no MIENTRAS lo este viendo
    if (carril == 1){                        //Cambia del carril derecho al izquierdo
        digitalWrite(direcA, LOW);
        digitalWrite(frenoA, LOW);
        analogWrite(velocA, vel_alta);    // rueda derecha gira mas rapido
        digitalWrite(direcB, HIGH);
        digitalWrite(frenoB, LOW);
        analogWrite(velocB, vel_baja);
       
        delay(delay_salirse);

        digitalWrite(direcA, LOW);
        digitalWrite(frenoA, LOW);
        analogWrite(velocA, vel_recta);  // ambas ruedas a la misma velocidad
        digitalWrite(direcB, HIGH);
        digitalWrite(frenoB, LOW);
        analogWrite(velocB, vel_recta);
        
        delay(delay_recto);

        digitalWrite(direcA, LOW);
        digitalWrite(frenoA, LOW);
        analogWrite(velocA, vel_baja);  //endereza para volver a la linea
        digitalWrite(direcB, HIGH);
        digitalWrite(frenoB, LOW);
        analogWrite(velocB, vel_alta);
        
        delay(delay_entrar);

        carril=0;
        e=0;
        
    }
    else {                                //Cambia del carril izquierdo al derecho
        digitalWrite(direcA, LOW);
        digitalWrite(frenoA, LOW);
        analogWrite(velocA, vel_baja);   //rueda izquierda gira mas rapido 
        digitalWrite(direcB, HIGH);
        digitalWrite(frenoB, LOW);
        analogWrite(velocB, vel_alta);

        delay(delay_salirse);
        
        digitalWrite(direcA, LOW);
        digitalWrite(frenoA, LOW);
        analogWrite(velocA, vel_recta);  // ambas ruedas a la misma velocidad
        digitalWrite(direcB, HIGH);
        digitalWrite(frenoB, LOW);
        analogWrite(velocB, vel_recta);
        
        delay(delay_recto);
        
        digitalWrite(direcA, LOW);
        digitalWrite(frenoA, LOW);
        analogWrite(velocA, vel_alta);  //endereza para volver a la linea
        digitalWrite(direcB, HIGH);
        digitalWrite(frenoB, LOW);
        analogWrite(velocB, vel_baja);
        
        delay(delay_entrar);

        carril=1;
        e=0;
        
    }
    
    }

  //Funcionamiento normal del siguelinas (usando vi y vd)
        digitalWrite(direcA, LOW);
        digitalWrite(frenoA, LOW);
        analogWrite(velocA, vd);
        digitalWrite(direcB, HIGH);
        digitalWrite(frenoB, LOW);
        analogWrite(velocB, vi);

        
  //     digitalWrite(frenoA, HIGH);
  //     digitalWrite(frenoB, HIGH);
  //      delay(2000);

  }
