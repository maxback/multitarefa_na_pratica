#include <Arduino_FreeRTOS.h>
#include <semphr.h>

const byte PinoChave = 2;

//Identificador da fila
QueueHandle_t xFila;

char szTextoPrimeiroItem[] = "Primeiro acionamento";
char szTextoDemaisItem[] = "Chave acionada X";
char nContador = 0;

//Função da task para notificar o ligamento da chave
void TaskEnvio( void *pvParametros );


void setup() {

  BaseType_t xRet;
  
  Serial.begin(9600);
  
  pinMode(PinoChave, INPUT_PULLUP);

  // Cria a fila de envio de caracteres
  xFila = xQueueCreate(
    10,                  // Tamanho da fila: 10
    21 *sizeof( char )   // Tamanho do item: 21
  );   

  if(xFila) Serial.println("Fila criado com sucesso!");

  xRet = xTaskCreate(TaskEnvio, (const portCHAR *) "TaskEnvio",  128, NULL, 2, NULL); 

  if(xRet == pdPASS) Serial.println("Task criada com sucesso:");
}

void loop() {
  //Monitoramento da flag saiu daqui
}

void tratarInterrpcaoPinoChave() {
  BaseType_t xHigherPriorityTaskWoken;  

  nContador++;
  if(nContador > 9) nContador = 2;
  
  if(nContador == 1)
  {
    //coloca o item na fila passando o texto especifico para o primeiro item
    xQueueSendToBackFromISR( xFila, &szTextoPrimeiroItem, &xHigherPriorityTaskWoken);
  }
  else
  {
    //Monta o texto a ser copiado apra o item da fila
    szTextoDemaisItem[15] = '0' + nContador;
  
    //coloca o item na fila passando o texto a ser copiado
    xQueueSendToBackFromISR( xFila, &szTextoDemaisItem, &xHigherPriorityTaskWoken);  
  }

  if( xHigherPriorityTaskWoken )
  {
    taskYIELD ();
  }  
}

//Função da task para notificar o ligamento da chave
void TaskEnvio( void *pvParametros ) {
  //Armazenada o texto que será recebido
  char szTexto[21];  
  
  Serial.println("Inciando loop: TaskEnvio");
  
  //loop da task
  for( ;; )
  {
    //Espera idefinidamente até ler um item da fila
    xQueueReceive( xFila, &szTexto, portMAX_DELAY );
  
    Serial.print(szTexto);
  }
}

