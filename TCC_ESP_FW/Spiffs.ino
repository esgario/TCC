void readFlashMemory()
{  
  int cont, contMenor;
  String strAux = "";
  char nomeArquivo[20];

  // listando arquivos na memoria
  listFile();
  
  // ================================= WIFI CONFIG ===============================
  //Serial.println(F("WiFiconfig:"));
  contMenor = 0;
  cont = 0;

  strAux = "";
  if ( readFile(FLASH_WIFICONF_PATH, &strAux) == true ){
    
    // ----------------------------- Atribuindo SSID a variavel
    cont = strAux.indexOf("\r\n");
    strAux.substring(0, cont).toCharArray(ssid, 20);
    //Serial.println(ssid);
    cont+=2;
    contMenor = cont;
    // ----------------------------- Atribuindo PASS
    cont = strAux.indexOf("\r\n", cont);
    strAux.substring(contMenor, cont).toCharArray(pass, 20);
  }
  else
  {
    writeFile(FLASH_WIFICONF_PATH,"Guilherme\r\njose0825\r\n", "w");
  }
  

  // ================== IP CONFIG ==================
  strAux = "";
  if( readFile(FLASH_IPCONFIG_PATH, &strAux) == true )
  {
    // ----------------------------- DHCP
    //Serial.print(strAux);
    contMenor = 0;
    cont = 0;
    cont = strAux.indexOf("\n", cont);
    DHCP = strAux.substring(contMenor, cont).toInt();
    cont++;
    contMenor = cont;
    if ( DHCP == 0 ) {
        // ----------------------------- IP
        for (byte i = 0; i < 3; i++) {
          cont = strAux.indexOf(".", cont);
          IP[i] = strAux.substring(contMenor, cont).toInt();
          cont++;
          contMenor = cont;
        }
        cont = strAux.indexOf("\n", cont);
        IP[3] = strAux.substring(contMenor, cont).toInt();
        cont++;
        contMenor = cont;
        //for(byte i=0; i<4; i++){
        //  Serial.printf("%d.",IP[i]);
        //}
        //Serial.println();
        // ----------------------------- SUBNET
        for (byte i = 0; i < 3; i++) {
          cont = strAux.indexOf(".", cont);
          Subnet[i] = strAux.substring(contMenor, cont).toInt();
          cont++;
          contMenor = cont;
        }
        cont = strAux.indexOf("\n", cont);
        Subnet[3] = strAux.substring(contMenor, cont).toInt();
        cont++;
        contMenor = cont;
        //for(byte i=0; i<4; i++){
        //  Serial.printf("%d.",Subnet[i]);
        //}
        //Serial.println();
        // ----------------------------- GATEWAY
        for (byte i = 0; i < 3; i++) {
          cont = strAux.indexOf(".", cont);
          Gateway[i] = strAux.substring(contMenor, cont).toInt();
          cont++;
          contMenor = cont;
        }
        cont = strAux.indexOf("\n", cont);
        Gateway[3] = strAux.substring(contMenor, cont).toInt();
        cont++;
        contMenor = cont;
        // ----------------------------- DNS
        for (byte i = 0; i < 3; i++) {
          cont = strAux.indexOf(".", cont);
          DNS[i] = strAux.substring(contMenor, cont).toInt();
          cont++;
          contMenor = cont;
        }
        cont = strAux.indexOf("\n", cont);
        DNS[3] = strAux.substring(contMenor, cont).toInt();
        cont++;
        contMenor = cont;
  
        if( IP[0] == 0 )
          DHCP = 1;
      } // SE DHCP == 0
  }
  else
  {
    writeFile(FLASH_IPCONFIG_PATH, "1\r\n", "w");
  }

  // carregar informações dos módulos c.m.
   strAux = "";
  if( readFile(FLASH_MODULOS_PATH, &strAux) == true )
  {
    contMenor = 0;
    cont = strAux.indexOf("#");
    //Serial.println(strAux);
    for(int i=0;i<4;i++)
    {
      strAux.substring(contMenor,cont).toCharArray(ModuloCM[i].nome,20);
      //Serial.println(ModuloCM[i].nome);
      contMenor = cont+1;
      cont = strAux.indexOf("#",contMenor);
      ModuloCM[i].fase = strAux.substring(contMenor,cont).toInt();
      
      contMenor = cont+1;
      cont = strAux.indexOf("#",contMenor);
      ModuloCM[i].tempo = strAux.substring(contMenor,cont).toInt();

      contMenor = cont+1;
      cont = strAux.indexOf("#",contMenor);
      ModuloCM[i].ligar.hora = strAux.substring(contMenor,cont).toInt();

      contMenor = cont+1;
      cont = strAux.indexOf("#",contMenor);
      ModuloCM[i].ligar.minuto = strAux.substring(contMenor,cont).toInt();
      
      contMenor = cont+1;
      cont = strAux.indexOf("#",contMenor);
      ModuloCM[i].desligar.hora = strAux.substring(contMenor,cont).toInt();

      contMenor = cont+1;
      cont = strAux.indexOf("#",contMenor);
      ModuloCM[i].desligar.minuto = strAux.substring(contMenor,cont).toInt();

      contMenor = cont+1;
      cont = strAux.indexOf("#",contMenor);
    }
  }
  else
  {
    writeFile(FLASH_MODULOS_PATH, "Circuito 1#1#0#0#0#0#0#Circuito 2#1#0#0#0#0#0#Circuito 3#1#0#0#0#0#0#Circuito 4#1#0#0#0#0#0#", "w");
  }

}

void saveModulesConfig()
{
  String strAux = "";

  for(int i=0;i<4;i++)
  {
    strAux += ModuloCM[i].nome;
    strAux += F("#");
    strAux += ModuloCM[i].fase;
    strAux += F("#");
    strAux += ModuloCM[i].tempo;
    strAux += F("#");
    strAux += ModuloCM[i].ligar.hora;
    strAux += F("#");
    strAux += ModuloCM[i].ligar.minuto;
    strAux += F("#");
    strAux += ModuloCM[i].desligar.hora;
    strAux += F("#");
    strAux += ModuloCM[i].desligar.minuto;
    strAux += F("#");
  }
  //Serial.println(strAux);
  writeFile(FLASH_MODULOS_PATH, strAux, "w");
}

