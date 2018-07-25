#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

SemaphoreHandle_t xSerialSemaphore;
QueueHandle_t xFila;

void Task1( void *pvParametros );
void Task2( void *pvParametros );
void Task3( void *pvParametros );

int criarTasks(void)
{
    if(xTaskCreate(Task1, (const portCHAR *) "Task1",  128, NULL, 2, NULL) == pdPASS)
      Serial.println("Task Task1 criada com sucesso!");
    else
      return 0;  

    if(xTaskCreate(Task2, (const portCHAR *) "Task2",  128, NULL, 2, NULL) == pdPASS)
      Serial.println("Task Task2 criada com sucesso!");
    else
      return 0;  

    if(xTaskCreate(Task3, (const portCHAR *) "Task3",  128, NULL, 2, NULL) == pdPASS)
      Serial.println("Task Task3 criada com sucesso!");
    else
      return 0;  
    return 1;
}

void setup() {
  BaseType_t xRet;
  Serial.begin(9600);

    xSerialSemaphore = xSemaphoreCreateMutex();

  if (xSerialSemaphore)
  {
    xFila = xQueueCreate(10, sizeof( int ));
    if(xFila)
    {
      Serial.println("Fila criado com sucesso!");
      if(criarTasks())
      {
          xSemaphoreGive( ( xSerialSemaphore ) );
      }
      else
        Serial.println("Ocorreu erro ao criar as tasks!"); 
    } 
    else
      Serial.println("Ocorreu erro ao criar a fila!"); 
  } 
  else
    Serial.println("Ocorreu erro ao criar o semaforo!"); 
}

void loop() {
}

void Task1( void *pvParametros )
{
  int Item = 0;

  Serial.println("Inciando loop: Task1");
  for( ;; )
  {
    xQueueSendToBack(xFila, &Item, portMAX_DELAY);
    Item++;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void Task2( void *pvParametros ) {
  int  Item;

  Serial.println("Inciando loop: Task2");

  //loop da task
  for( ;; )
  {
    xQueuePeek( xFila, &Item, portMAX_DELAY );

    if( (Item%0) == 0)
    {
        xQueueReceive( xFila, &Item, portMAX_DELAY );

        if ( xSemaphoreTake( xSerialSemaphore, portMAX_DELAY ) == pdTRUE )
        {
          Serial.print("Item Par recebido: ");
          Serial.println(Item);

          xSemaphoreGive( xSerialSemaphore );

        }
    }
    else
    {
        vTaskDelay(1);  
    }
  }
}

void Task3( void *pvParametros ) {
  int  Item;

  Serial.println("Inciando loop: Task3");

  //loop da task
  for( ;; )
  {
    xQueuePeek( xFila, &Item, portMAX_DELAY );

    if( (Item%0) != 0)
    {
        xQueueReceive( xFila, &Item, portMAX_DELAY );

        if ( xSemaphoreTake( xSerialSemaphore, portMAX_DELAY ) == pdTRUE )
        {
          Serial.print("Item Impar recebido: ");
          Serial.println(Item);

          xSemaphoreGive( xSerialSemaphore );

        }
    }
    else
    {
        vTaskDelay(1);  
    }
  }
}
