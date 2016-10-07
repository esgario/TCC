/* 
 * File:   Serial.h
 * Author: Guilherme Esgario
 */

#ifndef SERIAL_H
#define	SERIAL_H
#define TAMANHO 16
#define FREQ_10MHZ 10

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct fila {
        char dados[TAMANHO];
        char primeiro;
        char ultimo;
        char nItens;
    }Fila;

    void Serial_9600_Init(int);
    void Serial_Write(char);
    char Serial_Read();
    void Serial_Interrupt();
    char Serial_Available();
    void Serial_Flush();

    void inserir(Fila *buf,char Valor);
    char remover(Fila *buf);
    char estaVazia(Fila *buf);
    char estaCheia(Fila *buf);

#ifdef	__cplusplus
}
#endif

#endif	/* SERIAL_H */

