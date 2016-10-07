/*
 * ROTEIRO DE IMPLEMENTAÇÃO
 * OK  - Timer0 para contagem do tempo 0,1ms
 * OK  - Comunicação serial TX e RX
 * NOK - Tratar comandos recebidos pela porta serial
 */

#include <xc.h>
#include "Serial.h"

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON      // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define ReadBit(a,n) ((a>>n)&0x01)
#define SetBit(a,n) (a |= (1<<n))
#define ClearBit(a,n) ( a &= ~(1<<n))

#define COMMAND 0xF0
#define TRUE  1
#define FALSE 0

// ============ ESTRUTURAS ===============

struct ModuloCM {
    char status;
    int corrente[4];
}ModuloCM;

struct RTC {
    char hora;
    char minuto;
}RTC;

//=============== VARIÁVEIS ===============
unsigned int Temporizador1=0;
unsigned int TemporizadorSerialW=0;
unsigned int TemporizadorSerialR=0;
char Tensao[3] = {123,122,0};

unsigned int temporizador = 0;

//================ FUNÇÕES ================
void interrupt Interrupcao(void)
{
    if (INTCONbits.T0IF == 1)
    {
       INTCONbits.T0IF = 0;
       TMR0 += 130;
       if (temporizador > 0)
           temporizador --;
       if (Temporizador1 > 0)
           Temporizador1 --;
       if (TemporizadorSerialW > 0)
           TemporizadorSerialW--;
       if (TemporizadorSerialR > 0)
           TemporizadorSerialR--;
    }

    Serial_Interrupt();
}

void EnvioDadosSerial()
{
    if(TemporizadorSerialW == 0)
    {
        Serial_Write(0xFF); // Cabeçalho
        Serial_Write(0xFF);
        Serial_Write(Tensao[0]); // tensao
        Serial_Write(Tensao[1]);
        Serial_Write(Tensao[2]);
        Serial_Write(ModuloCM.status);
        Serial_Write(ModuloCM.corrente[0]>>8);
        Serial_Write(ModuloCM.corrente[0]&0x00FF);
        Serial_Write(ModuloCM.corrente[1]>>8);
        Serial_Write(ModuloCM.corrente[1]&0x00FF);
        Serial_Write(ModuloCM.corrente[2]>>8);
        Serial_Write(ModuloCM.corrente[2]&0x00FF);
        Serial_Write(ModuloCM.corrente[3]>>8);
        Serial_Write(ModuloCM.corrente[3]&0x00FF);
        Serial_Write(RTC.hora);
        Serial_Write(RTC.minuto);
        TemporizadorSerialW = 30000;
    }
}

void TrataComandoESP()
{
    if(TemporizadorSerialR == 0)
    {
        TemporizadorSerialR = 10000;
        while(Serial_Available())
        {
            if(Serial_Read() == COMMAND)
            {
                PORTB = Serial_Read();
                Serial_Flush();
            }
        }
    }
}

void I2C_Master_Start()
{
  SSPCON2bits.SEN = 1;  //Initiate start condition
}

void I2C_Master_Init()
{
  SSPCON = 0b00101000;  //SSP Module as Master
  SSPCON2 = 0;
  SSPADD = 24;          //Clock speed - 100kHz
  SSPSTAT = 0;
  TRISC3 = 1;           //Setting as input as given in datasheet
  TRISC4 = 1;           //Setting as input as given in datasheet
}

void Setup()
{
    TRISB = 0;
    PORTB = 0;
    TRISD = 0;
    PORTD = 0;
    // configurando o timer 0,1ms
    OPTION_REG = 0b00000000; // prescaler 1:2
    TMR0 = 130;
    INTCON = 0b11100000;
    //PIE1bits.TXIE = 1;
    Serial_9600_Init(FREQ_10MHZ);
    //I2C_Master_Init();
    //----------- inicializando variaveis

    ModuloCM.status = 0b00001100;
    ModuloCM.corrente[0] = 50;
    ModuloCM.corrente[1] = 34;
    ModuloCM.corrente[2] = 0;
    ModuloCM.corrente[3] = 0;

    TemporizadorSerialW = 20000;
}

int main(void)
{
    Setup();
    
    while(1)
    {
        EnvioDadosSerial();
        TrataComandoESP();

        if (Temporizador1 == 0)
        {
           Temporizador1 = 5000;
           PORTDbits.RD0 ^= 1; //PISCA LED
        }
    }
}