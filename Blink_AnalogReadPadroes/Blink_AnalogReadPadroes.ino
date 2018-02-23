#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );

/* Varios padrões de blink, baseados em texto:
 *  L - Led ligado por um período de um segundo
 *  D - Led desligado por um período de um segundo
 *  Ao chegar ao final do último caractere, repete o padrão.
 *  São definidos 4 padroes diferentes.
 */
 #define QTD_PADROES 4
char *padroes[QTD_PADROES] = {
  "LDDDDLDDDDLDDDDLDDDDLDDDDLDDDDLDDDDLDDDDLDDDDLDDDD", //aceso por 1 segundo e apagado por 4
  "LDDDDDDDDDLDDDDDDDDDLDDDDDDDDDLDDDDDDDDDLDDDDDDDDD", //aceso por 1 segundo e apagado por 9
  "LLLLLDDDDDLLLLLDDDDDLLLLLDDDDDLLLLLDDDDDLLLLLDDDDD", //aceso por 5 segundo e apagado por 5
  "DLLLLDLLLLDLLLLDLLLLDLLLLDLLLLDLLLLDLLLLDLLLLDLLLL", //apagado por 1 segundo e o aceso 4 (contrario do primeiro)
};

int nIndice = 0;
char *pcItemPadrao = NULL;


// the setup function runs once when you press reset or power the board
void setup() {
  
  // Aponta para o primeiro caracter do padrão
  pcItemPadrao = padroes[nIndice];
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

//  xTaskCreate(
//    TaskAnalogRead
//    ,  (const portCHAR *) "AnalogRead"
//    ,  128  // Stack size
//    ,  NULL
//    ,  1  // Priority
//    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, LEONARDO, MEGA, and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN takes care 
  of use the correct LED pin whatever is the board used.
  
  The MICRO does not have a LED_BUILTIN available. For the MICRO board please substitute
  the LED_BUILTIN definition with either LED_BUILTIN_RX or LED_BUILTIN_TX.
  e.g. pinMode(LED_BUILTIN_RX, OUTPUT); etc.
  
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
*/

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    //decide de acordo com o caracter do padão
    char nivel = *pcItemPadrao == 'L' ? HIGH : LOW; 
    
    digitalWrite(LED_BUILTIN, nivel);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  
    pcItemPadrao++;
    if(*pcItemPadrao == '\0') {
      //Vai para o proximo indice do padrão
      nIndice++;
      if(nIndice == QTD_PADROES) {
        nIndice = 0;
      }
      
      // Aponta para o primeiro caracter do padrão
      pcItemPadrao = padroes[nIndice];    
    }

    //envia o padrão por prepduzir (parcial ou desde o inicio, se mudou)
    Serial.println(pcItemPadrao);
  }
}

void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    // print out the value you read:
    Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}
