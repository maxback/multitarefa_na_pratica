#ifndef _FUNCOES_APOIO_H_
#define _FUNCOES_APOIO_H_

#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).


SemaphoreHandle_t xfuncoesapoio_SerialSemaphore = NULL;

void funcoesapoio_init(SemaphoreHandle_t &pxSerialSemaphore)
{
  xfuncoesapoio_SerialSemaphore = pxSerialSemaphore;
  
}

void SerialDebugComSemaforo(const char *pszTexto)
{
    if ( xSemaphoreTake( xfuncoesapoio_SerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.print(pszTexto);

      xSemaphoreGive( xfuncoesapoio_SerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
}

void SerialDebugComSemaforo(const int pnValor)
{
    if ( xSemaphoreTake( xfuncoesapoio_SerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.print(pnValor);

      xSemaphoreGive( xfuncoesapoio_SerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
}

void SerialDebugComSemaforo(const char *pszTexto, const int pnValor)
{
    if ( xSemaphoreTake( xfuncoesapoio_SerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.print(pszTexto);
      Serial.print(pnValor);

      xSemaphoreGive( xfuncoesapoio_SerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
}

void SerialDebugComSemaforo(const char *pszTexto, const int pnValor, const char *pszTexto2)
{
    if ( xSemaphoreTake( xfuncoesapoio_SerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.print(pszTexto);
      Serial.print(pnValor);
      Serial.print(pszTexto2);

      xSemaphoreGive( xfuncoesapoio_SerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
}

void SerialDebugComSemaforo(const char *pszTexto, const char *pszTexto2, const char *pszTexto3, const int pnValor)
{
    if ( xSemaphoreTake( xfuncoesapoio_SerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial.print(pszTexto);
      Serial.print(pszTexto2);
      Serial.print(pszTexto3);
      Serial.print(pnValor);

      xSemaphoreGive( xfuncoesapoio_SerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
}

#endif
