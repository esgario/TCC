/* 
 * File:   ConversorAD.h
 * Author: guilh
 *
 * Created on 11 de Outubro de 2016, 11:06
 */

#ifndef CONVERSORAD_H
#define	CONVERSORAD_H

#define ANALOG_CONF_8_0 0x00


#ifdef	__cplusplus
extern "C" {
#endif

void ADC_Init(char,char);
void ADC_Interrupt();
void ADC_Run();
void ADC_Select_Channel(char);
int  ADC_Read();
char ADC_Available();

#ifdef	__cplusplus
}
#endif

#endif	/* CONVERSORAD_H */

