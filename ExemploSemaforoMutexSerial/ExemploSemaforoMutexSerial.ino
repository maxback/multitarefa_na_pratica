#include <Arduino_FreeRTOS.h>
#include <semphr.h>

SemaphoreHandle_t xSerialSemaphore;

void TaskEnviaTextoSerial( void *pvParameters );

void setup() 
{
  Serial.begin(9600);
  while (!Serial) ;


  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  if ( xSerialSemaphore != NULL )
  {
    //Cria duas tasks com a mesma função, passando textos diferentes por parametro
    xTaskCreate(TaskEnviaTextoSerial, "TaskEnviaTextoSerial1", 128, (void *)"Texto 1", 1, NULL );
    xTaskCreate(TaskEnviaTextoSerial, "TaskEnviaTextoSerial2", 128, (void *)"Texto 2", 1, NULL );
    xTaskCreate(TaskEnviaTextoSerial, "TaskEnviaTextoSerial3", 128, (void *)"Texto 3", 1, NULL );
    xTaskCreate(TaskEnviaTextoSerial, "TaskEnviaTextoSerial4", 128, (void *)"Texto 4", 1, NULL );
    xTaskCreate(TaskEnviaTextoSerial, "TaskEnviaTextoSerial5", 128, (void *)"Texto 5", 1, NULL );    
  }
  else
  {
    Serial.println("**** Erro ao criar semaforo!");
  }
}

void loop()
{
  // Nada no loop. Tudo nas tasks!
}

void TaskEnviaTextoSerial( void *pvParameters )
{
  //pegando o parametro como o texto da string
  const char *pszTexto = (const char *) pvParameters;

  for (;;)
  {
    //Trocar portMAX_DELAY por ( TickType_t ) 5 para ticks e deposi desistir
    if ( xSemaphoreTake( xSerialSemaphore, portMAX_DELAY ) == pdTRUE )
    {
      Serial.println("-------------------------");
      Serial.println(pszTexto);

      xSemaphoreGive( xSerialSemaphore );

    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}



