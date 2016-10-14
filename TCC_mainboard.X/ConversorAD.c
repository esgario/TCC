#include "ConversorAD.h"
#include <xc.h>

char dadoDisponivel = 0;
char ADC_Temporizador = 0;

void ADC_Interrupt()
{
    if (INTCONbits.T0IF == 1)
    {
        if(ADC_Temporizador > 0)
            ADC_Temporizador--;
    }

    if( PIR1bits.ADIF == 1 )
    {
        dadoDisponivel = 1;
        PIR1bits.ADIF = 0;
    }
}

void ADC_Init(char trisa, char config)
{
    TRISA = trisa;
    ADCON0 = 0b10000001;
    ADCON1 = 0b10000000;
    ADCON1 |= config;

    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
}

void ADC_Run()
{
  static char Estado = 0;

  switch(Estado)
  {
      case 0:
          if(ADC_Temporizador == 0)
          {
              ADC_Temporizador = 1;
              Estado = 1;
          }
          break;
      case 1:
          ADCON0bits.GO = 1;
          Estado = 0;
          break;
  }
}

int ADC_Read()
{
    dadoDisponivel = 0;
    return ((ADRESH<<8) | ADRESL);
}

void ADC_Select_Channel(char ch)
{
    ADCON0 &= 0b11000111;
    ADCON0 |= (ch<<3);
}

char ADC_Available()
{
    return dadoDisponivel;
}