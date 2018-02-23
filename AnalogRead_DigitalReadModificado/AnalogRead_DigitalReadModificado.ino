#include <Arduino_FreeRTOS.h>
#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).


//tipo de parametros
typedef struct {
  const char *pcNomeTask;	// Nome da task A SER CRIADA
  const char *pcTexto;		// Texto a ser enviado pela serial antes da leitura, identificando sua fonte
  int SensorID;			// ID da entrada analógica usada
  int TempoDelayTicks;		// Tempo entre envio de leituras, em ticks
  QueueHandle_t xFilaCalculo; 	// Fila para odne mandar valores, caso dif. de NULL
} AnalogReadParametro_t;

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only only one Task is accessing this resource at any time.
SemaphoreHandle_t xSerialSemaphore;

//fila para que as tasks mandem seus valores analógicos para que seja calculada uma média dos valores enviados (acumuladas)
QueueHandle_t xQueueMediaValores = NULL;
//valor inteiro da media dos valores
int giMediaValores = 0;

// define two Tasks for DigitalRead & AnalogRead
void TaskDigitalRead( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
//aceita como parametro AnalogReadParametro_t
void TaskAnalogReadParam( void *pvParameters );
//task que calcula media dos valoresp ostos na fila
void TaskMediaValoresLidos( void *pvParameters );

//define parametrosp ara trues tasks de leitura analógica que acietam paramtros de estrutura
AnalogReadParametro_t xParams[3], *pxParam = xParams;


// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
  // because it is sharing a resource, such as the Serial port.
  // Semaphores should only be used whilst the scheduler is running, but we can set it up here.
  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  // Now set up two Tasks to run independently.
  xTaskCreate(
    TaskDigitalRead
    ,  (const portCHAR *)"DigitalRead"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  (configMAX_PRIORITIES - 3)  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

/*
  xTaskCreate(
    TaskAnalogRead
    ,  (const portCHAR *) "AnalogRead"
    ,  128  // Stack size
    ,  (void *)"Entrada A0" //passa por parametro o texto antes de cada leitura
    ,  (configMAX_PRIORITIES - 2)  // Priority
    ,  NULL );

*/

  //valor inteiro da media dos valores
  giMediaValores = 0;
  
  //cria fila para que as tasks mandem seus valores analógicos para que sejá calculada uma média dos valores enviados ()acumuladas)
  xQueueMediaValores = xQueueCreate( 30 //qtd maximo de itens
    , sizeof( int ) //tamanho em memória de cada item
    );

   //cria task para media de valores
  xTaskCreate(
    TaskMediaValoresLidos
    ,  (const portCHAR *) "TaskMediaValoresLidos"
    ,  128  // Stack size
    ,  (void *)xQueueMediaValores //passa por parametro o texto antes de cada leitura
    ,  (configMAX_PRIORITIES - 1)  // Priority
    ,  NULL );
   

    //define parametrosp ara trues tasks de leitura analógica que acietam paramtros de estrutura

    pxParam->pcNomeTask = "TaskEntradaA1";
    pxParam->pcTexto = "Entrada A1";
    pxParam->SensorID = A1;
    pxParam->TempoDelayTicks = 1; //pdMS_TO_TICKS( 250UL );
    pxParam->xFilaCalculo = xQueueMediaValores;
    pxParam++;

    pxParam->pcNomeTask = "TaskEntradaA2";
    pxParam->pcTexto = "Entrada A2";
    pxParam->SensorID = A2;
    pxParam->TempoDelayTicks = 1; //pdMS_TO_TICKS( 250UL );
    pxParam->xFilaCalculo = xQueueMediaValores;
    pxParam++;

    pxParam->pcNomeTask = "TaskEntradaA3";
    pxParam->pcTexto = "Entrada A3";
    pxParam->SensorID = A3;
    pxParam->TempoDelayTicks = 1; //pdMS_TO_TICKS( 250UL );
    pxParam->xFilaCalculo = xQueueMediaValores;
    pxParam++;


    //xTaskCreate(TaskAnalogReadParam, pxParam->pcNomeTask, 128, (void *)&xParams[0], 1  /* Priority  */, NULL );
    //xTaskCreate(TaskAnalogReadParam, pxParam->pcNomeTask, 128, (void *)&xParams[1], 1  /* Priority  */, NULL );
    //xTaskCreate(TaskAnalogReadParam, pxParam->pcNomeTask, 128, (void *)&xParams[2], 1  /* Priority  */, NULL );
    
    //cria as tasks percorre array de ponteiros até o nulo (para facilitar)
    for(pxParam = &xParams[0]; pxParam < &xParams[3]; pxParam++)
    {
      Serial.println("\n---------------------------------");
      Serial.print("Criando ");
      Serial.println(pxParam->pcNomeTask);
      
      xTaskCreate(TaskAnalogReadParam, pxParam->pcNomeTask, 128, (void *)pxParam, (configMAX_PRIORITIES - 2)  /* Priority  */, NULL );
    }   

    
// Now the Task scheduler, which takes over control of scheduling individual Tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

void SerialDebugComSemaforo(const char *pszTexto)
{
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.print(pszTexto);

      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskDigitalRead( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    DigitalReadSerial
    Reads a digital input on pin 2, prints the result to the serial monitor

    This example code is in the public domain.
  */

  // digital pin 2 has a pushbutton attached to it. Give it a name:
  uint8_t pushButton = 2;

  // make the pushbutton's pin an input:
  pinMode(pushButton, INPUT);

  for (;;) // A Task shall never return or exit.
  {
    // read the input pin:
    int buttonState = digitalRead(pushButton);

    // See if we can obtain or "Take" the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      //Envia o texto recebido por parâmetro
      Serial.print("entrada digital: ");

      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the state of the button:
      Serial.println(buttonState);

      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }

    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskAnalogRead( void *pvParameters )  // This is a Task.
{
  //pegando o parametro como o texto da string
  const char *pcTexto = (const char *) pvParameters;

  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);

    // See if we can obtain or "Take" the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      //Envia o texto recebido por parâmetro
      Serial.print(pcTexto);
      Serial.print(": ");
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the value you read:
      Serial.println(sensorValue);

      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }

    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskAnalogReadParam( void *pvParameters )  // This is a Task.
{
  //pegando o parametro como o texto da string
  AnalogReadParametro_t *pxParams = (AnalogReadParametro_t *) pvParameters;

  for (;;)
  {
    // read the input on analog pin:
    int sensorValue = analogRead(pxParams->SensorID);

    // See if we can obtain or "Take" the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      //Envia o texto recebido por parâmetro
      Serial.print(pxParams->pcTexto);
      Serial.print(": ");
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the value you read:
      Serial.println(sensorValue);

      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.

      //se definida uam fila, envia valor ligo apra ela
      if(pxParams->xFilaCalculo != NULL)
      {
        /* Envia o vlaor lido apra a fila1. Especifica um tempo maximo de espera pela eventual 
         *  liberação de espaço, caso não haja. Usa o próprio valor do delay
         *  (pxParams->TempoDelayTicks). A task que le esta fila tem maior prioridade
         *  e poderá liberar um item rapidamente.
         Notar que é passado o endereço do local onde está o valor */
        if(xQueueSendToBack( pxParams->xFilaCalculo, &sensorValue, pxParams->TempoDelayTicks ) != pdPASS)
        {
          SerialDebugComSemaforo("Erro ao enviar valor para fila!\n");
        }
        
      }
    }

    vTaskDelay(pxParams->TempoDelayTicks);  // one tick delay (15ms) in between reads for stability
  }
}



void TaskMediaValoresLidos( void *pvParameters )  // This is a Task.
{
  int sensorValue;

  const QueueHandle_t xFilaCalculo = (QueueHandle_t *) pvParameters;

  for (;;)
  {
    /*Se a fila foi definida, espera indefinidamente até que tenha um valor para 
     * ler. Uma vbez lido, faz a média do valor lido com o acumulado e envia pela serial.  
     */
    if(xFilaCalculo != NULL)
    {
      if(xQueueReceive( xFilaCalculo, &sensorValue, portMAX_DELAY ) != pdPASS)
      {
        SerialDebugComSemaforo("Erro ao receber valor para fila!\n");
      }
      else
      {
        giMediaValores = (giMediaValores + sensorValue) / 2;
        // See if we can obtain or "Take" the Serial Semaphore.
        // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
        if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
        {
          //Envia o texto recebido por parâmetro
          Serial.print("Recebido: ");
          Serial.print(sensorValue);
          Serial.print(" - Nova média: ");
          Serial.println(giMediaValores);

          // We were able to obtain or "Take" the semaphore and can now access the shared resource.
          // We want to have the Serial Port for us alone, as it takes some time to print,
          // so we don't want it getting stolen during the middle of a conversion.
          // print out the value you read:
    
          xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
        }
      }
    } 

    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

