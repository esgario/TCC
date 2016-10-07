#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <FS_control.h>

#define FLASH_IPCONFIG_PATH   "/IPconfig.txt"
#define FLASH_WIFICONF_PATH   "/WiFiconfig.txt"
#define FLASH_MODULOS_PATH    "/Modulos.txt"

#define SET   1
#define CLEAR 0

extern "C" {
	#include "os_type.h"
}

// ====================== ESTRUTURAS

typedef struct Fase {
  int tensao = 0;
  int potencia = 0;
  int corrente = 0;
} fase;

typedef struct ModuloCM {
  boolean existe = true;
  char nome[20] = "Modulo";
  byte estado = 0; // 0 - off | 1 - on
  int corrente = 0;
  int potencia = 0;
  byte fase = 0; // 1-R, 2-S ou 3-T
  boolean tempo = false;
  struct ligar{
    byte hora = 0;
    byte minuto = 0;
  }ligar;
  struct desligar{
    byte hora = 0;
    byte minuto = 0;
  }desligar;
} modulos;

struct RTC {
  int hora = 0;
  int minuto = 0;
} RTC;

// ====================== PROTÓTIPOS
void controleConexao();
void comunicacaoSerial();
void servidorWeb();
void ControleModulos(byte funcao, byte indice);
void setup_wificonf();
void indexHandler();
void handleNotFound();
void quitPage();
void readFlashMemory();
void openWebPage();
void saveModulesConfig();

// ====================== VARIÁVEIS

// --------------------------- rede
char ssid[20] = "Guilherme";
char pass[20] = "jose0825";

byte IP[4] = {192, 168, 0, 155};
byte DNS[4] = {8, 8, 8, 8};
byte Gateway[4] = {192, 168, 0, 1};
byte Subnet[4] = {255, 255, 255, 0};
byte DHCP = 0;

WiFiServer server2(8090);
ESP8266WebServer server = ESP8266WebServer(80);

// ---------------------------- temporizadores
os_timer_t myTimer;

int temporizadorWifi = 5000;

// ----------------------------- Grandezas elétricas
fase Fase[3];
modulos ModuloCM[4];

// ===========================================

void timerCallback(void *pArg) {
  if ( temporizadorWifi > 0 ) temporizadorWifi--;
}

void setup() {

  Serial.begin(9600);

  // prepare GPIO2
  pinMode(0, INPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  // configurando ponteiro de função callback
  os_timer_setfn(&myTimer, timerCallback, NULL);
  
  // habilitando o temporizador
  os_timer_arm(&myTimer, 1, true);

  Fase[0].tensao = 127;
  Fase[0].corrente = 10;
  Fase[0].potencia = 1270;

  if(!SPIFFS.begin())
    Serial.println(F("Erro no spiffs."));
  
  readFlashMemory();
}

void loop() {
  controleConexao();
  servidorWeb();
  openWebPage();
}

void openWebPage()
{
  setup_wificonf();
  if( digitalRead(0) == LOW )
  {
    delay(50);
    if( digitalRead(0) == LOW )
      setup_wificonf();
  }
}

