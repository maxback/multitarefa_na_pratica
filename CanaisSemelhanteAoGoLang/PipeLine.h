#ifndef _PIPELINE_H_
#define _PIPELINE_H_

typedef struct {
  CanalEntreTasks<int> *entrada;
  CanalEntreTasks<int> *saida;
} CanaisDaTask_t;

// Define 3 canais e 3 funções de tasks
CanalEntreTasks<int> xCanalMultiplica(10, "CanalEntMultiplica");
//CanalEntreTasks<int> xCanalDiminui(10);
//CanalEntreTasks<int> xCanalImprime(30);

CanaisDaTask_t C1 = {&xCanalMultiplica, new CanalEntreTasks<int>(10, "CanalEntDiminui")};
CanaisDaTask_t C2 = {C1.saida, new CanalEntreTasks<int>(30, "CanalImprime")};
CanaisDaTask_t C3 = {C2.saida, NULL};


void TaskMultiplica( void *pvParameters );
void TaskDiminui( void *pvParameters );
void TaskImprime( void *pvParameters );


void pipeline_init(void)
{
  xTaskCreate(TaskMultiplica, (const portCHAR *) "TaskMultiplica", 128, (void *)&C1, (configMAX_PRIORITIES - 1),  NULL );
  xTaskCreate(TaskDiminui, (const portCHAR *) "TaskDiminui", 128, (void *)&C2, (configMAX_PRIORITIES - 2),  NULL );
  xTaskCreate(TaskImprime, (const portCHAR *) "TaskImprime", 128, (void *)&C3, (configMAX_PRIORITIES - 3),  NULL );
}


CanalEntreTasks<int> *pipeline_getCanalEntrada(void)
{
  return &xCanalMultiplica;
}

void TaskMultiplica( void *pvParameters )  // This is a Task.
{
  int mult = 1;
  int valor;

  CanaisDaTask_t *C = (CanaisDaTask_t *) pvParameters;

  for (;;)
  {
    //valor = C->entrada->receber();
    *(C->entrada) >> valor;

    SerialDebugComSemaforo("\nTaskMultiplica recebeu ", valor);
   
    valor *= mult; 

    mult++;
    mult = (mult % 5) + 1;

    if(C->saida)
    {
      SerialDebugComSemaforo("\n -> ANTES nTaskMultiplica enviando para ", C->saida->getNome(), ": ", valor);

      //C->saida->enviar(mult);
     *(C->saida) << valor;

      SerialDebugComSemaforo("\n -> DEPOIS nTaskMultiplica enviando: ", C->saida->getNome(), ": ", valor);
    
    }
    SerialDebugComSemaforo("\n");
  }
}


void TaskDiminui( void *pvParameters )  // This is a Task.
{
  int diminuido = 10000;
  int diminutor = 0;
  int valor;

  CanaisDaTask_t *C = (CanaisDaTask_t *) pvParameters;

  for (;;)
  {
    //valor = C->entrada->receber();
    *(C->entrada) >> valor;

    SerialDebugComSemaforo("\n  TaskDiminui recebeu ", valor, "\n");
   
    valor -= diminutor; 

    diminutor++;
    diminutor = diminutor % 10;

    //xCanalImprime << diminuido;
    if(C->saida)
    {
      SerialDebugComSemaforo("\n   -> ANTES TaskDiminui enviando para ", C->saida->getNome(), ": ", valor);

      //C->saida->enviar(diminuido);
     *(C->saida) << valor;
  
      SerialDebugComSemaforo("\n   -> DEPOIS TaskDiminui enviando para", C->saida->getNome(), ": ",  valor);
    }
  }
}


void TaskImprime( void *pvParameters )  // This is a Task.
{
  int valor, valor2;
  //int contRealimentacao = 5;

  CanaisDaTask_t *C = (CanaisDaTask_t *) pvParameters;

  //seta timeout desta fila
  C->entrada->setTempoTimeoutMs(100);

  for (;;)
  {
    /*
    //ESPIA, SEM TIMEPUT
    *(C->entrada) > valor;

    if(C->entrada->getResultadoOperacao())
    {
      SerialDebugComSemaforo("\n    TaskImprime vai receber o valor ", valor);
    }
    */
    
    *(C->entrada) >> valor;

    //para tem timeout setado com C->entrada->setTempoTimeoutMs(100);
    //valor = C->entrada->receber(true);
    
    if(!C->entrada->getResultadoOperacao())
    {
      SerialDebugComSemaforo("\n    TaskImprime não recebeu nada...");
      continue;
    }

    //*(C->entrada) >> valor;

    SerialDebugComSemaforo("\n    TaskImprime recebeu ", valor);
   
    SerialDebugComSemaforo("\n    IMPRESSÃO: -> ",valor, "\n");

    if(C->saida)
    {
      //C->saida->enviar(valor);
      *(C->saida) << valor;
    }    

/*
    contRealimentacao--;
    if(!contRealimentacao)
    {
      contRealimentacao = 5;

      SerialDebugComSemaforo("\n    -> ANTES TaskImprime enviando para ", pipeline_getCanalEntrada()->getNome(), ": ", valor);
      
      *(pipeline_getCanalEntrada()) << valor;

      SerialDebugComSemaforo("\n    -> DEPOIS TaskImprime enviando para ", pipeline_getCanalEntrada()->getNome(), ": ", valor);
    }
*/  
  }
}

#endif
