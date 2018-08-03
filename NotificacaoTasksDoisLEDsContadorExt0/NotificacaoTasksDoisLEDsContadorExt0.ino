#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

//Estrutura para leitura da interrpcao (com 16 bits)
typedef union {
	struct {
		uint16_t nflagEventoOcorreu     : 1;
		uint16_t nID                    : 4;
		uint16_t nTempoEntreAcionamento : 11;
	};
	uint16_t nValor;
} botao_leitura_t;

//Mapa de bits com o bit nflagEventoOcorreu setado para uso em mascaras
#define BOTAO_LEITURA_MASCARA16_BIT_FLAGEVENTO_OCORREU ((uint16_t)0x0001)

#define BOTAO_LEITURA_MASCARA32_BITS_FLAGEVENTO_OCORREU \
	                         (((uint32_t)BOTAO_LEITURA_MASCARA16_BIT_FLAGEVENTO_OCORREU)<<16) | \
							 (((uint32_t)BOTAO_LEITURA_MASCARA16_BIT_FLAGEVENTO_OCORREU))

//Definicao dos pinos dos LEDs
#define LED_PLACA LED_BUILTIN
#define LED_ADICIONAL 12

static TaskHandle_t gxTaskNotificacao = NULL;
static TaskHandle_t gxTaskContadorExt0 = NULL;

SemaphoreHandle_t xSerialSemaphore;

static void EXTI0_IRQHandler(void);
static void EXTI1_IRQHandler(void);
void SerialDebugComSemaforo(const char *pszTexto);

void setup() 
{
  Serial.begin(9600);
  
  pinMode(LED_PLACA, OUTPUT);
  pinMode(LED_ADICIONAL, OUTPUT);

  //Para controlar acesso a serial
  if ( xSerialSemaphore == NULL )
  {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) ); 
  }


	xTaskCreate(taskBotaoUsuariioFunc, "taskBotaoUsuariioFunc",
		128, NULL, 2, &gxTaskNotificacao );

	xTaskCreate(taskContadorExt0Func, "gxTaskContadorExt0",
		128, NULL, 1, &gxTaskContadorExt0 );
		

    attachInterrupt(0, EXTI0_IRQHandler, RISING);
    attachInterrupt(1, EXTI1_IRQHandler, RISING);
}

void loop()
{
  //nada a fazer
}

void EXTI0_IRQHandler(void)
{

	static TickType_t xTempodaUltimaInterrupcao = 0;
	TickType_t xAgora;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t ulValorTaskNotificacao;
	botao_leitura_t leitura;
	xAgora = xTaskGetTickCountFromISR();

	leitura.nID = 0;
	leitura.nTempoEntreAcionamento = xAgora - xTempodaUltimaInterrupcao;
	leitura.nflagEventoOcorreu = 1;

	xTempodaUltimaInterrupcao = xAgora;

	if(gxTaskNotificacao)
	{
		ulValorTaskNotificacao = leitura.nValor;
		ulValorTaskNotificacao <<= 0;
		ulValorTaskNotificacao &= 0x0000FFFF;

		//Envia uma notifica��o diretamente para a task indicada por gxTaskNotificacao
		xTaskNotifyFromISR( gxTaskNotificacao,
			         	 ulValorTaskNotificacao, /* ulValue */
				         eSetBits, /* eAction parameter. */
		                 &xHigherPriorityTaskWoken );


	}

    //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    if( xHigherPriorityTaskWoken )
    {
    	taskYIELD ();
    }
}

void EXTI1_IRQHandler(void)
{
	static TickType_t xTempodaUltimaInterrupcao = 0;
	TickType_t xAgora;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t ulValorTaskNotificacao;
	botao_leitura_t leitura;


	xAgora = xTaskGetTickCountFromISR();

	leitura.nID = 1;
	leitura.nTempoEntreAcionamento = xAgora - xTempodaUltimaInterrupcao;
	leitura.nflagEventoOcorreu = 1;

	xTempodaUltimaInterrupcao = xAgora;

	if(gxTaskNotificacao)
	{
		ulValorTaskNotificacao = leitura.nValor;
		ulValorTaskNotificacao <<= 16;
		ulValorTaskNotificacao &= 0xFFFF0000;

		//Envia uma notifica��o diretamente para a task indicada por gxTaskNotificacao
		xTaskNotifyFromISR( gxTaskNotificacao,
		         	 ulValorTaskNotificacao, /* ulValue */
			         eSetBits, /* eAction parameter. */
	                 &xHigherPriorityTaskWoken );

	}

    //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    if( xHigherPriorityTaskWoken )
    {
    	taskYIELD ();
    }
}


void SerialDebugComSemaforo(const char *pszTexto)
{
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.println(pszTexto);

      xSemaphoreGive( xSerialSemaphore ); 
    }
}


void taskBotaoUsuariioFunc( void *pvParameters )
{
	union {
		uint32_t ulLeituras;
		botao_leitura_t leitura[2];
	} xDados;

	char szMsgLeitura[100];
	BaseType_t xTempos[2] = {portMAX_DELAY, portMAX_DELAY};
    int LEDs[2] = {LED_PLACA, LED_ADICIONAL};
    int contador = 0;

	//inicia colocando o contador 0
	if( gxTaskContadorExt0 )
	{
		xTaskNotify(gxTaskContadorExt0, 0, eSetValueWithOverwrite);
	}
    
	for (;;)
	{
        //pisca LEDS, exceto se n� otiver tempo
        for(int i=0; i<2; i++)
        {
            if(xTempos[i] == portMAX_DELAY) continue;

            digitalWrite(LEDs[i], contador&0x01 ? HIGH : LOW);
            vTaskDelay( xTempos[i] );
        }
        contador++;

		//espera notificao com os eventos
		xTaskNotifyWait(0, //ulBitsToClearOnEntry
		        0xFFFFFFFF, //ulBitsToClearOnExit
		        &xDados.ulLeituras, //pulNotificationValue
			portMAX_DELAY);

		for(int i=0; i<2; i++)
		{
			if(!xDados.leitura[i].nflagEventoOcorreu)
				continue;
            
            //ajusta tempo do led correspondente
            xTempos[i] = xDados.leitura[i].nTempoEntreAcionamento;

            sprintf(szMsgLeitura, "Evento ID %d, Tempo: %d ms", xDados.leitura[i].nID, xDados.leitura[i].nTempoEntreAcionamento);

			//se a ext 0 vei evento amnda notifica�� oapra outra task
			if( (i == 0) && (gxTaskContadorExt0) )
			{
				xTaskNotify(gxTaskContadorExt0, 0, eIncrement);
			}

			SerialDebugComSemaforo(szMsgLeitura);
		} //for(int i=0...
    }
}


void taskContadorExt0Func( void *pvParameters )
{
	uint32_t ulLeituras;
	char szMsgContador[100];
    
	for (;;)
	{
		//Verifica se tem notifica��o, sem esperar 
		if( xTaskNotifyWait(0, 0, &ulLeituras, 0) == pdPASS)
		{
			//imprime contador
			sprintf(szMsgContador, "Contador: %d", ulLeituras);

			SerialDebugComSemaforo(szMsgContador);

		};

		//espera 5 sergundos
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
