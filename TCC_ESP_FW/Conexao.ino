
// Controla acesso ao acess point, indentificando perdas de conexao e forçando reconexões

void controleConexao() {
  static char Estado = 0;

  switch (Estado) {

    case 0: // Verificando conexao
      if (WiFi.status() != WL_CONNECTED)
        Estado = 1;
      break;
    case 1: // Inicia conexao
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, pass);
      if (DHCP == 0)
        WiFi.config(IP, Gateway, Subnet, DNS);
        
      temporizadorWifi = 10000;
      Estado = 2;
      break;
    case 2: // Aguarda
      if ( temporizadorWifi <= 0 ) {
        if (WiFi.status() == WL_CONNECTED) {
          Serial.print(0x0F);
          Serial.print(0x01);
          server2.begin();
          //Serial.println(WiFi.localIP());
        } else {
          WiFi.disconnect();
          Serial.print(0x0F);
          Serial.print(0x00);
        }
        Estado = 0;
      }
      break;
  }
}
