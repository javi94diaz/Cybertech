#define kp_delante 0.2

#define kp_atras 0.5

#define sens_deteccion_paredD 175 //valores originales: D,I=175; F=150
#define sens_deteccion_paredI 175
#define sens_deteccion_paredF 150
#define sens_deteccion_paredcallejonF 500

#define vel_giro_rapida 130
#define vel_giro_lenta 20
#define vel_giro_rapida2 150
#define vel_giro_lenta2 0

#define velBase 90 //velocidad desde la que se incrementa/decrementa
#define error_limite 50 //ESTABA EN 50
#define error_alto 150
#define incremento 50

#define ref_regulador 250
#define delay_atras 1000 //milisegundos
//#define delay_girando 250 //milisegundos

//-----------------------------------------------------------------------------//
//-------------------------- DEFINE PINES DE ARDUINO -------------------------- //
//-----------------------------------------------------------------------------//


int velocA = 3; //A es el derecho joder
int direcA = 12;
int frenoA = 9;

int velocB = 11;
int direcB = 13;
int frenoB = 8;

int sensorI = A3;
int sensorD = A5;
int sensorF = A4;

int valorF = 0;
int valorD = 0;
int valorI = 0;


//-----------------------------------------------------------------------------//
//-------------------------- VARIABLES DE CONTROL-- -------------------------- //
//-----------------------------------------------------------------------------//

int error = 0;
int deltaV = 0; //diferencia de velocidades entre derecho e izquierdo

int velD = 0;
int velI = 0;
int dirA = 0; // Necesario poder controlar la marcha atrás del robot para callejón sin salida
int dirB = 1; // Necesario poder controlar la marcha atrás del robot para callejón sin salida

//--Variables para determinar en que zona del laberinto estamos.--//
//Si están a 1 tenemos pared, si están a 0 no la tenemos.
int paredD = 0;
int paredCF = 0;
int paredI = 0;
int paredF = 0;

int prioridadderecha = 1; //Siempre seguirá la pared de la dcha. En caso de callejón sin salida, pasa a seguir la pared de la izquierda.
int marchaatras = 0; //En caso de ir hacia atrás es necesario poner un delay para que el sensor no detecte pared libre teniendo la parte delantera del robot tocando pared.

int bandera_palante = 0;


//----------------------------------------------------------------------------------//
//------------------------------------ SET UP --------------------------------------//
//----------------------------------------------------------------------------------//

void setup() {

  pinMode (velocA, OUTPUT);
  pinMode (direcA, OUTPUT);
  pinMode (frenoA, OUTPUT);

  pinMode (velocB, OUTPUT);
  pinMode (direcB, OUTPUT);
  pinMode (frenoB, OUTPUT);

  pinMode (sensorI, INPUT);
  pinMode (sensorD, INPUT);
  pinMode (sensorF, INPUT);

  Serial.begin (9600);

//----- Prueba para que en el inicio esté parado ----//

while(analogRead(sensorF)>50){
  digitalWrite(frenoA, HIGH);
  digitalWrite(frenoB, HIGH);
}


}


//----------------------------------------------------------------------------------//
//---------------------------------- FUNCIONES -------------------------------------//
//----------------------------------------------------------------------------------//

void sensoresyerror() {
     valorD = analogRead(sensorD);
     valorI = analogRead(sensorI);
      valorF = analogRead(sensorF);

      if (valorD > valorI && valorD > ref_regulador) error = valorD - ref_regulador;
      else if (valorI > valorD && valorI > ref_regulador) error = -valorI + ref_regulador;
      else error = 0;
}


void haypared() {

      if (valorI < sens_deteccion_paredI) paredI = 0; //Valor de comparación de ejemplo, COMPROBAR
      else paredI = 1;
      if (valorD < sens_deteccion_paredD) paredD = 0;
      else paredD = 1;
      if (valorF < sens_deteccion_paredF) paredF = 0;
      else paredF = 1;
      if (valorF < sens_deteccion_paredcallejonF) paredCF = 0;
      else paredCF = 1;
}


void haciadelante() {

      if (abs(error) < error_alto) {
    
          if (abs(error) < error_limite) deltaV = 0;
          else  deltaV = floor(kp_delante * error);
          
          velD = velBase + deltaV / 2;
          velI = velBase - deltaV / 2;
      }
     
      else  {
          if (error > 0)   giroizquierda();
          else  giroderecha();
      }
    
      if (velD < 0) velD = 0;
      if (velI < 0) velI = 0;
      dirA = 0;
      dirB = 1;
    
      prioridadderecha = 1;
}


void giroizquierda() {
      velI = vel_giro_lenta2; //valores de ejemplo (comprobar)
      velD = vel_giro_rapida2; //valores de ejemplo  (comprobar)
      dirA = 0;
      dirB = 1;
      marchaatras = 0;
}


void giroderecha() {
      velI = vel_giro_rapida2;
      velD = vel_giro_lenta2; 
      dirA = 0;
      dirB = 1;
}


void giro_bloqueo() {
      if (valorD > valorI) {
        velD = vel_giro_lenta2;
        velI = vel_giro_rapida2;
        prioridadderecha = 0;
      }
      else {
        velI = vel_giro_lenta2;
        velD = vel_giro_rapida2;
      }
    
      dirA = 1;
      dirB = 0;
      marchaatras = 1;
 
}


//----------------------------------------------------------------------------------//
//------------------------------------ LOOP ----------------------------------------//
//----------------------------------------------------------------------------------//

void loop() {

  Serial.print(analogRead(sensorI));
  Serial.print('\t');
  Serial.print(analogRead(sensorF));
  Serial.print('\t');
  Serial.print(analogRead(sensorD));
  Serial.print('\t');
  Serial.print('\t');
  Serial.print(velI);
  Serial.print('\t');
  Serial.println(velD);


  sensoresyerror();

  haypared();


  if (paredD == 1 && paredI == 1 && paredF == 0) { //Sólo un camino libre (recto)
    haciadelante();
  }

  else if (paredD == 0 && paredI == 1 && paredF == 1) { //Sólo un camino libre (derecha)
    //--¿? ERROR: En caso de que en un giro a izq detecte pared delantera y derecha
    if(prioridadderecha=1)  giroderecha();
    else giroizquierda();
  }

  else if (paredD == 1 && paredI == 0 && paredF == 1) { //Sólo un camino libre (izq)
    bandera_palante = 1;
    giroizquierda();
  }

  else if (paredD == 1 && paredI == 1 && paredCF == 1) { //Callejón sin salida, damos marcha atras
    giro_bloqueo();
  }

  else if (paredD == 0 && paredI == 0 && paredF == 1) { //Derecha o izquierda
    if (bandera_palante == 1) {
      prioridadderecha = 0;
      bandera_palante = 0;
    }
    if (prioridadderecha == 1) giroderecha();
    else giroizquierda();
  }

  else if (paredD == 0 && paredI == 1 && paredF == 0) { //En frente o derecha
    if (prioridadderecha == 1) giroderecha();
    else haciadelante(); // ¿? ERROR: Estaba puesto ir hacia la izq----------------
  }

  else if (paredD == 1 && paredI == 0 && paredF == 0) { //Izquierda o en frente

  //-- ¿Para qué se hace esto? --//------------------------------------------- 
  //-- Se podría dejar solo haciadelante.
    
    prioridadderecha = 1;

    if (prioridadderecha == 1) haciadelante();
    else giroizquierda;
  }

  else if (paredD == 0 && paredI == 0 && paredF == 0) {
    haciadelante();
  }



  digitalWrite(direcA, dirA);
  digitalWrite(frenoA, LOW);
  analogWrite(velocA, velD);
  digitalWrite(direcB, dirB);
  digitalWrite(frenoB, LOW);
  analogWrite(velocB, velI);

  if (marchaatras == 1) {
    delay(delay_atras);
    marchaatras = 0;
  }


}



