#ifndef _CANALENTRETASKS_H_
#define _CANALENTRETASKS_H_

#include "funcoes_apoio.h"
/**
 * Class que implementa um canal, semelhante aos do golange ecapsulando uma fila
 * CanalEntreTasks<T>
 */
template <typename T>
class CanalEntreTasks {
public:

  CanalEntreTasks(uint16_t pnTamanho): 
    nTempoTimeoutMs(0), bResultadoOperacao(false)
  {
        xFila = xQueueCreate(pnTamanho, sizeof(T));
  };

  CanalEntreTasks(uint16_t pnTamanho, const char *pszNome): 
    CanalEntreTasks(pnTamanho) 
  {
        setNome(pszNome);
  };

  ~CanalEntreTasks() {
     
  if(xFila != NULL)
      vQueueDelete( xFila );
  };

  bool getResultadoOperacao(void) const
  {
    return bResultadoOperacao;
  }

  QueueHandle_t getFilaInterna()
  {
    return xFila;
  }

  void setNome(const char *pszNome)
  {
    strcpy(szNome, pszNome);
  };

  char * getNome(void)
  {
    return szNome;
  };

  void setTempoTimeoutMs(const int pnTempoTimeoutMs)
  {
    nTempoTimeoutMs = pnTempoTimeoutMs;
  };
  
    
  void enviar(const T &pxValor, bool pbTimeOut = false)
  {
    bResultadoOperacao = xQueueSendToBack( xFila, &pxValor, 
      pbTimeOut ? pdMS_TO_TICKS(nTempoTimeoutMs) : portMAX_DELAY ) == pdPASS;
  };

  T receber(bool pbTimeOut = false) {

    T xValor;
    
     bResultadoOperacao = xQueueReceive( xFila, &xValor, 
       pbTimeOut ? pdMS_TO_TICKS(nTempoTimeoutMs) : portMAX_DELAY ) == pdPASS;

     return xValor;

  };

  T ver() {

    T xValor;
    
     bResultadoOperacao = xQueuePeek( xFila, &xValor, portMAX_DELAY ) == pdPASS;

     return xValor;

  };
  
void sobrescrever(T &pxValor) {

    T xValor;
    
     bResultadoOperacao = xQueueOverwrite( xFila, &pxValor ) == pdPASS;

  };
  
private:
  QueueHandle_t xFila;
  char szNome[33];
  int nTempoTimeoutMs;
  bool bResultadoOperacao;
};



//para permitir encadear o operador << (escrita sem timeout, bloqeuia enquanto não houver espaço) 
//Equivale a chamar CanalEntreTasks<T>::enviar(T xValor);
template <typename T>
CanalEntreTasks<T>& operator<<(CanalEntreTasks<T> &c, T pxValor)
{
    c.enviar(pxValor);
    return c;
}


//para permitir encadear o operador >> (leitura sem timeout, bloqeuia enquanto não houver dado)
//Equivale a chamar T CanalEntreTasks<T>::receber();
template <typename T>
CanalEntreTasks<T>& operator>>(CanalEntreTasks<T> &c, T &pxValor)
{
    pxValor = c.receber();
    return c;
}


//para permitir encadear o operador > para ler (peek) o primeiro item que sairia
//Equivale a chamar T CanalEntreTasks<T>::ver();
template <typename T>
CanalEntreTasks<T>& operator>(CanalEntreTasks<T> &c, T &pxValor)
{
    pxValor = c.ver();
    return c;
}

//para permitir encadear o operador < (override) apra sobrescrever o ultimo elemento
//Equivale a chamar CanalEntreTasks<T>::sobrescrever(T &xValor);
template <typename T>
CanalEntreTasks<T>& operator<(CanalEntreTasks<T> &c, T &pxValor)
{
    c.sobrescrever(pxValor);
    return c;
}


#endif
