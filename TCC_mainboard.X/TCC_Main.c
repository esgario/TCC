#include <xc.h>
#include <math.h>
#include "Serial.h"
#include "ConversorAD.h"

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF      // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON      // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define ReadBit(a,n) ((a>>n)&0x01)
#define SetBit(a,n) (a |= (1<<n))
#define ClearBit(a,n) ( a &= ~(1<<n))

#define CMD_PORTB 0xF0
#define CMD_CONNECTION 0x0F
#define TRUE  1
#define FALSE 0
#define NUM_AMOSTRAS 500

typedef char boolean;

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
unsigned int TemporizadorLed=0;
unsigned int TemporizadorSerialW=0;
unsigned int TemporizadorSerialR=0;
unsigned int TemporizadorPortb = 0;
char Tensao[3] = {0,0,0};

boolean clearPORTB = FALSE;
boolean conectado = FALSE;

//================ FUNÇÕES ================
void interrupt Interrupcao(void)
{
    Serial_Interrupt();
    ADC_Interrupt();

    if (INTCONbits.T0IF == 1)
    {
       INTCONbits.T0IF = 0;
       TMR0 += 130;
       if (TemporizadorLed > 0)
           TemporizadorLed --;
       if (TemporizadorSerialW > 0)
           TemporizadorSerialW--;
       if (TemporizadorSerialR > 0)
           TemporizadorSerialR--;
       if (TemporizadorPortb > 0)
           TemporizadorPortb--;
    }
}

void EnvioDadosSerial()
{
    if(TemporizadorSerialW == 0)
    {
        char i;
        ModuloCM.status = 0xFF;
        /*for(i=0;i<4;i++)
        {
            if(ModuloCM.corrente[i] > 200)
                SetBit(ModuloCM.status,i);
        }*/
        
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
    static char Estado = 0;

    if(TemporizadorSerialR == 0)
    {
        TemporizadorSerialR = 5000;
        char cmd;

        while(Serial_Available())
        {
            switch(Estado)
            {
                case 0:
                    cmd = Serial_Read();
                    if(cmd == CMD_PORTB)
                        Estado = 1;
                    else if(cmd == CMD_CONNECTION)
                        Estado = 2;
                    break;
                case 1:
                    clearPORTB = TRUE;
                    TemporizadorPortb = 10000;
                    PORTB = Serial_Read();
                    Estado = 0;
                    break;
                case 2:
                    if(Serial_Read() == 1)
                        conectado = TRUE;
                    else
                        conectado = FALSE;
                    Estado = 0;
                    break;
            }
        }
    }

    if(clearPORTB == TRUE && TemporizadorPortb == 0)
    {
        clearPORTB = FALSE;
        PORTB = 0;
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

void ControleLed()
{
    static char Estado = 0;
    if (TemporizadorLed == 0)
    {
        switch(Estado)
        {
            case 0:
                TemporizadorLed = 2000;
                PORTDbits.RD0 = 1; //ACENDE LED
                Estado = 1;
                break;
            case 1:
                PORTDbits.RD0 = 0; //APAGA LED
                if(conectado) {
                    TemporizadorLed = 2000;
                    Estado = 2;
                } else {
                    TemporizadorLed = 10000;
                    Estado = 0;
                }
                break;
            case 2:
                TemporizadorLed = 2000;
                PORTDbits.RD0 = 1; //ACENDE LED
                Estado = 3;
                break;
            case 3:
                TemporizadorLed = 10000;
                PORTDbits.RD0 = 0; //ACENDE LED
                Estado = 0;
                break;
        }
    }
}

void SensoresAnalogicos()
{
    static int n = 0;
    static char index = 0;
    signed long valorAux = 0;
    static long somatorio = 0;

    if( n < NUM_AMOSTRAS )
    {
        if(ADC_Available())
        {
            if(index < 4){
                //valorAux = ADC_Read() - 511;
                //somatorio += valorAux*valorAux;
                valorAux = ADC_Read();
                if( somatorio < valorAux ) somatorio = valorAux;
            }
            else{
                valorAux = ADC_Read();
                somatorio += valorAux*valorAux;
            }
            n++;
        }
    }
    else
    {
        if(index < 4)
            //ModuloCM.corrente[index] = sqrt(somatorio/NUM_AMOSTRAS)*4.883;
            ModuloCM.corrente[index] = somatorio;
        else
        {
            Tensao[index-4] = sqrt(somatorio/NUM_AMOSTRAS)/3.2;
            if(Tensao[index-4] < 70) Tensao[index-4] = 0;
        }
        somatorio = 0;
        index++;
        index = index%7;

        if(index < 4)
            ADC_Select_Channel(index);
        else
            ADC_Select_Channel(index+1);
        n = 0;
    }
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
    ADC_Init(0xFF, ANALOG_CONF_8_0);
    ADC_Select_Channel(0);
    //I2C_Master_Init();
    //----------- inicializando variaveis

    ModuloCM.status = 0b00000011;
    ModuloCM.corrente[0] = 0;
    ModuloCM.corrente[1] = 0;
    ModuloCM.corrente[2] = 0;
    ModuloCM.corrente[3] = 0;

    TemporizadorSerialW = 20000;
}

int main(void)
{
    Setup();
    
    while(1)
    {
        ControleLed();
        EnvioDadosSerial();
        TrataComandoESP();
        ADC_Run();
        SensoresAnalogicos();
    }
}