void servidorWeb()
{
  // Checa se algum cliente conectou ao servidor.
  WiFiClient client = server2.available();
  if (!client)
    return;
  
  // Aguarda até o cliente enviar algum dado
  while(!client.available()) {
    delay(1);
  }
  // Lê a primeira linha da requisição
  String req = client.readStringUntil('\r');
  //Serial.println(req);
  client.flush();

  // Montagem da resposta
  
  String msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  int aux, aux2, index;
  
  if (req.indexOf("Connect") != -1)
  {
    msg += F("Connected");
    msg += F("\r\n");
  }
  else if(req.indexOf("Get") != -1)
  {
    aux = req.indexOf("Get");
    aux2 = req.indexOf(":",aux);
    index = req.substring(aux+3,aux2).toInt();
    
    if( index == 0 )
    { 
      for(int i=0;i<3;i++) {
        msg += Fase[i].tensao;
        msg += F("#");
      }
      for(int i=0;i<3;i++) {
        msg += Fase[i].corrente;
        msg += F("#");
      }
      for(int i=0;i<3;i++) {
        msg += Fase[i].potencia;
        msg += F("#");
      }

      if( RTC.hora < 10 )
        msg += F("0");
      msg += RTC.hora;
      msg += F(":");
      if( RTC.minuto < 10 )
        msg += F("0");
      msg += RTC.minuto;
      msg += F("#");
  
      for(int i=0;i<4;i++)
      {
        if( ModuloCM[i].existe )
        {
          msg += ModuloCM[i].nome;
          msg += F("#");
          msg += ModuloCM[i].estado;
          msg += F("#");
          msg += float(ModuloCM[i].corrente)/10;
          msg += F("#");
          msg += float(ModuloCM[i].potencia)/100;
          msg += F("#");
        }
      }
      msg += F("\r\n");
    }
    else
    {
      index--;

      for(byte i=0;i<4;i++)
      {
        if( ModuloCM[i].existe && index > 0 )
        {
          index--;
        }
        else if( ModuloCM[i].existe )
        {
          msg += ModuloCM[i].nome;
          msg += F("#");
          msg += ModuloCM[i].estado;
          msg += F("#");
          msg += float(ModuloCM[i].corrente)/10;
          msg += F("#");
          msg += float(ModuloCM[i].potencia)/100;
          msg += F("#");
        }
      }
    }
  }
  else if( req.indexOf("Set") != -1 )
  {
    aux = req.indexOf("Set");
    aux2 = req.indexOf(":",aux);
    index = req.substring(aux+3,aux2).toInt();

    if(req.indexOf("Off") != -1)
    {
      ControleModulos(CLEAR, index);
      ModuloCM[index-1].estado = 0;
    }
    else if(req.indexOf("On") != -1)
    {
      ControleModulos(SET, index);
      ModuloCM[index-1].estado = 1;
    }
    
    msg = F("HTTP/1.1 204 OK\r\n");
  }
  else if( req.indexOf("gConf") != -1 )
  {
    aux = req.indexOf("gConf");
    aux2 = req.indexOf(":",aux);
    index = req.substring(aux+5,aux2).toInt();

    index--;

    for(byte i=0;i<4;i++)
    {
      if( ModuloCM[i].existe && index > 0 )
      {
        index--;
      }
      else if( ModuloCM[i].existe )
      {
        msg += ModuloCM[i].nome;
        msg += F("#");
        msg += ModuloCM[i].fase;
        msg += F("#");
        msg += ModuloCM[i].tempo;
        msg += F("#");
        msg += ModuloCM[i].ligar.hora;
        msg += F("#");
        msg += ModuloCM[i].ligar.minuto;
        msg += F("#");
        msg += ModuloCM[i].desligar.hora;
        msg += F("#");
        msg += ModuloCM[i].desligar.minuto;
        msg += F("#");
      }
    }

  }
  else if( req.indexOf("sConf") != -1 )
  {
    aux = req.indexOf("sConf");
    aux2 = req.indexOf(":",aux);
    index = req.substring(aux+5,aux2).toInt();
    index--;
    
    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    req.substring(aux,aux2).toCharArray(ModuloCM[index].nome,20);
    
    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    ModuloCM[index].fase = req.substring(aux,aux2).toInt();

    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    if( req.substring(aux,aux2) == "true" )
      ModuloCM[index].tempo = true;
    else
      ModuloCM[index].tempo = false;
      
    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    ModuloCM[index].ligar.hora = req.substring(aux,aux2).toInt();

    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    ModuloCM[index].ligar.minuto = req.substring(aux,aux2).toInt();

    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    ModuloCM[index].desligar.hora = req.substring(aux,aux2).toInt();

    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    ModuloCM[index].desligar.minuto = req.substring(aux,aux2).toInt();

    saveModulesConfig();
    
    msg = F("HTTP/1.1 204 OK\r\n");
  }
  else if( req.indexOf("RTC") != -1 )
  {
    aux = req.indexOf("RTC");
    aux2 = req.indexOf(":",aux);
    RTC.hora = req.substring(aux+3,aux2).toInt();
    aux = aux2 + 1;
    aux2 = req.indexOf(":",aux);
    RTC.minuto = req.substring(aux,aux2).toInt();
    
    msg = F("HTTP/1.1 204 OK\r\n");
    // SALVAR NOVO HORARIO NO RTC
  }
  else
  {
    msg = F("HTTP/1.1 204 OK\r\n");
  }

  client.println(msg);
  
  client.flush();
  client.stop();
}

// ===================================================================================

boolean quitWebServer = false;
boolean atualizaDados = false;

void setup_wificonf()
{
  IPAddress ipAP(192, 168, 4, 1);
  IPAddress ipAPmask(255, 255, 255, 0);

  WiFi.disconnect();
  WiFi.begin();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ipAP, ipAP, ipAPmask);
  WiFi.softAP("Sisgen", "12345678");

  IPAddress myIP = WiFi.softAPIP();

  server.on("/",indexHandler);
  server.on("/Quit",quitPage);
  server.onNotFound(handleNotFound);
  server.begin();

  while( quitWebServer == false )
  {
    server.handleClient();

    if( atualizaDados )
    {
      atualizaDados = false;
      readFlashMemory();
    }
  }
  quitWebServer = false;
  readFlashMemory();
}

// 404 error
void handleNotFound()
{
  //Serial.println(F("Page not found"));
  server.send(404,"text/html","<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"> <body><h1>Page Not Found</h1></body></html>");
}

// pagina web inicial
void indexHandler()
{
  String webPage = "";
  String webPageFirstPart="";
  String strAux = "";
  int8_t salvar = 0;
  int8_t _opdhcp = 1;

  if(server.hasArg("dhcpOP"))
  {
    //Serial.printf("dhcpOP: %s\n",server.arg("dhcpOP").c_str());
    _opdhcp = server.arg("dhcpOP").toInt();
    strAux = "";
    strAux = _opdhcp;
    strAux +="\r\n";
  }
  if(server.hasArg("wifiName"))
  {
    //Serial.printf("wifiName: %s\n",server.arg("wifiName").c_str());
    strAux = server.arg("wifiName").c_str();
    strAux.toCharArray(ssid, 30);
    salvar = 1;
  }
  if(server.hasArg("password"))
  {
    //Serial.printf("\npass: %s\n",server.arg("password").c_str());
    strAux = server.arg("password").c_str();
    strAux.toCharArray(pass, 20);
    salvar = 1;
  }
  if(server.hasArg("staticIp"))
  {
    //Serial.printf("staticIp: %s\n",server.arg("staticIp").c_str());
    strAux += server.arg("staticIp");
    strAux +="\r\n";
  }
  if(server.hasArg("staticMsk"))
  {
    //Serial.printf("staticMsk: %s\n",server.arg("staticMsk").c_str());
    strAux += server.arg("staticMsk");
    strAux +="\r\n";
  }
  if(server.hasArg("staticGtw"))
  {
    //Serial.printf("staticGtw: %s\n",server.arg("staticGtw").c_str());
    strAux += server.arg("staticGtw");
    strAux +="\r\n";
  }
  if(server.hasArg("staticDn"))
  {
    //Serial.printf("staticDn: %s\n",server.arg("staticDn").c_str());
    strAux += server.arg("staticDn");
    strAux +="\r\n";
    salvar = 2;
  }
  
  if(server.hasArg("buttonQuit"))
  {
    //Serial.printf("buttonQuit: %s\n",server.arg("buttonQuit").c_str());
    quitWebServer = true;
  }

  switch(salvar)
  {
    case 1:
      strAux = "";
      strAux += ssid;
      strAux += "\r\n";
      strAux += pass;
      strAux += "\r\n";
    
      atualizaDados = true;
      writeFile(FLASH_WIFICONF_PATH, strAux, "w");
      //Serial.printf("Wifi principal salva: %s\n",strAux.c_str());
      
      salvar = 0;
      break;
    case 2:
      //Serial.printf("\nDHCP:%d\n%s\n",_opdhcp,strAux.c_str());
      if(_opdhcp == 0)        // IP estatico
      {
        DHCP = 0;
        writeFile(FLASH_IPCONFIG_PATH, strAux, "w");
        atualizaDados = true;
        //Serial.print("DHCP = 0");
      }
      else
      {
        //Serial.print("DHCP = 1");
        DHCP = 1;
        writeFile(FLASH_IPCONFIG_PATH, strAux, "w");
        atualizaDados = true;
      }

      salvar = 0;
      strAux = "";
    break;
  }

  strAux = "";

  byte strt, end;
  strt = end = 0;

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  String partWebPage = "";
  
  webPageFirstPart = F("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
  "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
  "<head>\n"
  "<meta http-equiv=\"Content-Type\" content=\"width=device-width, initial-scale=1.0\" charset=utf-8\" />"
  "<title>SISGEN</title>"
  "<script>"
  "window.onload=function() {\n"
  "  // get tab container\n"
  "  var container = document.getElementById(\"tabContainer\");\n"
  "  console.log(\"container\");\n"
  "  console.log(container);\n"
  "    // set current tab\n"
  "    var navitem = container.querySelector(\"#tabs > ul > li\");\n"
  "    //store which tab we are on\n"
  "  console.log(\"navitem\");\n"
  " console.log(navitem);\n"
  "    var ident = navitem.id.split(\"_\")[1];\n"
  " console.log(\"navitem id:\");\n"
  " console.log(ident);\n"
  "    navitem.parentNode.setAttribute(\"data-current\",ident);\n"
  "    //set current tab with class of activetabheader\n"
  "    navitem.setAttribute(\"class\",\"tabActiveHeader\");\n"
  "    //hide two tab contents we don't need\n"
  "    var pages = container.querySelectorAll(\".tabpage\");\n"
  "    for (var i = 1; i < pages.length; i++) {\n"
  "      pages[i].style.display=\"none\";\n"
  "    }\n"
  "    //this adds click event to tabs\n"
  "    var tabs = container.querySelectorAll(\"#tabs ul li\");\n"
  "    for (var i = 0; i < tabs.length; i++) {\n"
  "      tabs[i].onclick =displayPage;\n"
  "   console.log(\"tabs:\");\n"
  "   console.log(tabs[i]);\n"
  "    }\n"
  "}\n");
  server.sendContent(webPageFirstPart);
  
  webPageFirstPart = F("// on click of one of tabs\n"
  "function displayPage()\n"
  "{  var current = this.parentNode.getAttribute(\"data-current\");\n"
  "  //remove class of activetabheader and hide old contents\n"
  "  document.getElementById(\"tabHeader_\" + current).removeAttribute(\"class\");\n"
  "  console.log(\"line 93\");\n"
  "  console.log(document.getElementById(\"tabHeader_\" + current).removeAttribute(\"class\"));\n"
  "  document.getElementById(\"tabpage_\" + current).style.display=\"none\";\n"
  "  var ident = this.id.split(\"_\")[1];\n"
  "  //add class of activetabheader to new active tab and show contents\n"
  "  this.setAttribute(\"class\",\"tabActiveHeader\");\n"
  "  document.getElementById(\"tabpage_\" + ident).style.display=\"block\";\n"
  "  this.parentNode.setAttribute(\"data-current\",ident);\n}\n"
  "</script>\n"
  "<style>"
  "*, *:before, *:after {"
  "-moz-box-sizing: border-box;"
  "-webkit-box-sizing: border-box;"
  "box-sizing: border-box;"
  "}"
  "html {"
  "font-family: Helvetica, Arial, sans-serif;"
  "font-size: 100%;"
  "background: #333;"
  "}"
  "#page-wrapper {"
  "width: auto;"
  "height: auto;"
  "background: #FFF;"
  "padding: 01%;"
  "margin: 1em auto;"
  "border-top: 5px solid #70d0d0;"
  "box-shadow: 0 2px 10px rgba(0,0,0,0.8);"
  "}");
  server.sendContent(webPageFirstPart);
  
  webPageFirstPart = F("#tabContainer {"
  " width:auto;"
  " padding:15px;"
  " background-color:#2e2e2e;"
  " -moz-border-radius: 4px;"
  " border-radius: 4px; "
  "}"
  "#tabs{"
  " height:30px;"
  " overflow:hidden;"
  "}"
  "#tabs > ul{"
  " font: 1em;"
  " list-style:none;"
  "}"
  "#tabs > ul > li{"
  " margin:0 2px 0 0;"
  " padding:7px 10px;"
  " display:block;"
  " float:left;"
  " color:#FFF;"
  " -webkit-user-select: none;"
  " -moz-user-select: none;"
  " user-select: none;"
  " -moz-border-radius-topleft: 4px;"
  " -moz-border-radius-topright: 4px;"
  " -moz-border-radius-bottomright: 0px;"
  " -moz-border-radius-bottomleft: 0px;"
  " border-top-left-radius:4px;"
  " border-top-right-radius: 4px;"
  " border-bottom-right-radius: 0px;"
  " border-bottom-left-radius: 0px; "
  " background: #C9C9C9;"
  " background: -moz-linear-gradient(top, #70d0d0 0%, #409090 100%);"
  " background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#70d0d0), color-stop(100%,#409090));"
  "}");
  server.sendContent(webPageFirstPart);
  
  webPageFirstPart=F("#tabs > ul > li:hover{"
  " background: #FFFFFF; "
  " background: -moz-linear-gradient(top, #FFFFFF 0%, #F3F3F3 10%, #F3F3F3 50%, #FFFFFF 100%); "
  " background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#FFFFFF), color-stop(10%,#F3F3F3), color-stop(50%,#F3F3F3), color-stop(100%,#FFFFFF));"
  " cursor:pointer;"
  " color: #333;"
  "}"
  "#tabs > ul > li.tabActiveHeader{"
  " background: #FFFFFF; "
  " background: -moz-linear-gradient(top, #FFFFFF 0%, #F3F3F3 10%, #F3F3F3 50%, #FFFFFF 100%); "
  " background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#FFFFFF), color-stop(10%,#F3F3F3), color-stop(50%,#F3F3F3), color-stop(100%,#FFFFFF));"
  " cursor:pointer;"
  " color: #333;"
  "}"
  "#tabscontent {"
  " -moz-border-radius-topleft: 0px;"
  " -moz-border-radius-topright: 4px;"
  " -moz-border-radius-bottomright: 4px;"
  " -moz-border-radius-bottomleft: 4px;"
  " border-top-left-radius: 0px;"
  " border-top-right-radius: 4px;"
  " border-bottom-right-radius: 4px;"
  " border-bottom-left-radius: 4px; "
  " padding:10px 10px 25px;"
  " background: #FFFFFF;"
  " background: -moz-linear-gradient(top, #FFFFFF 0%, #FFFFFF 90%, #e4e9ed 100%);"
  " background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#FFFFFF), color-stop(90%,#FFFFFF), color-stop(100%,#e4e9ed));"
  " margin:0;"
  " color:#333;"
  "}");
  server.sendContent(webPageFirstPart);
  
  webPageFirstPart = F("h1 {"
  "margin-top: 0;"
  "}"
  "#status {"
  "font-size: 0.9rem;"
  "margin-bottom: 1rem;"
  "}"
  ".open {"
  "color: green;"
  "}"
  ".closed {"
  "color: red;"
  "}"
  "ul {"
  "list-style: none;"
  "margin: 0;"
  "padding: 0;"
  "font-size: 0.95rem;"
  "}"
  "ul li {"
  "padding: 0.5rem 0.75rem;"
  "border-bottom: 1px solid #EEE;"
  "}"
  "ul li:first-child {"
  "border-top: 1px solid #EEE;"
  "}"
  "ul li span {"
  "display: inline-block;"
  "width: 90px;"
  "font-weight: bold;"
  "color: #999;"
  "font-size: 0.7rem;"
  "text-transform: uppercase;"
  "letter-spacing: 1px;"
  "}");
  server.sendContent(webPageFirstPart);
  
  webPageFirstPart = F("textarea {"
  "width: auto;"
  "padding: 0.5rem;"
  "font-size: 1rem;"
  "border: 1px solid #D9D9D9;"
  "border-radius: 3px;"
  "box-shadow: inset 0 1px 1px rgba(0, 0, 0, 0.1);"
  "min-height: 100px;"
  "margin-bottom: 1rem;"
  "}\n"
  "button {\n"
  "display: inline-block;\n"
  "border-radius: 3px;\n"
  "border: none;\n"
  "font-size: 0.9rem;\n"
  "padding: 0.6rem 1em;\n"
  "color: white;\n"
  "margin: 0 0.25rem;\n"
  "text-align: center;\n"
  "background: #70d0d0;\n"
  "border-bottom: 1px solid #999;\n"
  "}\n"
  "button#buttonIpSettings {\n"
  "display: block;\n"
  "border-radius: 3px;\n"
  "border: none;\n"
  "font-size: 0.9rem;\n"
  "padding: 0.6rem 1em;\n"
  "color: white;\n"
  "margin: 10px 0.25rem;\n"
  "text-align: center;\n"
  "background: #70d0d0; \n"
  "border-bottom: 1px solid #999;\n"
  "}\n"
  "button[type=\"submit\"] {\n"
  "background: #70d0d0;\n"
  "border-bottom: 1px solid #5d7d1f;\n"
  "}\n"
  "button:hover {\n"
  "opacity: 0.75;\n"
  "}\n"
  "ul#menuRoot {"
  "    list-style-type: none;"
  "    margin: 0;"
  "    padding: 0;"
  "    width: 200px;"
  "    background-color: #f1f1f1;"
  "}"
  "ul#menuRoot li a {"
  "    display: block;"
  "    color: #000;"
  "    padding: 8px 0 8px 16px;"
  "    text-decoration: none;"
  "}");
  server.sendContent(webPageFirstPart);
  
  webPageFirstPart = F("ul#menuRoot li a:hover:not(.active) {"
  "    background-color: #70d0d0;"
  "    color: white;"
  "}"
  ".inputPassword {"
  "    width: auto;"
  "    padding: 10px 20px;"
  "    margin: 8px 0;"
  "    box-sizing: border-box;"
  "}"
  ".staticIpSetting form  { display: table;      }"
  ".staticIpSetting p     { display: table-row;  }"
  ".staticIpSetting label { display: table-cell; }"
  ".staticIpSetting   input { display: table-cell; width: 179px; padding: 0.2rem 2px; text-align: center; }"
  "label.telegramBlock {   display: inline-block;  width: 140px;  text-align: right; }"
  "</style>"
  "</head>"
  "<body>\n"
  "<div id=\"page-wrapper\">\n"
  "<h1 align=\"center\" aling=\"left\">Sisgen</h1>\n"
  "<ul id=\"messages\"></ul>\n"
  "<div id=\"tabContainer\">\n"
  "    <div id=\"tabs\">\n"
  "      <ul>\n"
  "        <li id=\"tabHeader_1\">WLAN Config</li>\n"
  "        <li id=\"tabHeader_2\">IP Settings</li>\n"
  "        <li id=\"tabHeader_3\">Quit</li>\n"
  "      </ul>\n"
  "    </div>\n"
  "    <div id=\"tabscontent\">\n"
  "      <div class=\"tabpage\" id=\"tabpage_1\">\n"
  "        <p>\n"
  "        </p>\n"
  "        <form method=\"post\">\n");
  server.sendContent(webPageFirstPart);
  webPageFirstPart = "";
  // "      <div class=\"radio\">\n");
  //   server.sendContent(webPageFirstPart); // ############
  //   webPageFirstPart = "";
  //
  //  for (uint8_t ap_idx = 0; ap_idx < ap_count; ap_idx++)
  //  {
  //    webPage += "     <input type=\"radio\" name=\"wifi\" value=\"";
  //    webPage += String(WiFi.SSID(ap_idx)) + "\" ";
  //    if( ap_idx == 0 ) webPage += "checked";
  //    webPage += ">";
  //    webPage+= String(WiFi.SSID(ap_idx)) +  " (RSSI: " + WiFi.RSSI(ap_idx) + ")";
  //    (WiFi.encryptionType(ap_idx) == ENC_TYPE_NONE) ? webPage += " " : webPage += "*";
  //    webPage += "<br>";
  //  }
  webPage += F("<div >\n"
  /*"<input type=\"radio\" id=\"WifiOp1\" name=\"MainWifiOp\" value=\"0\" checked>\n"
  "<label for=\"WifiOP1\">Main</label>\n"
  "<input type=\"radio\" id=\"WifiOP2\"  name=\"MainWifiOp\" value=\"1\">\n"
  "<label for=\"WifiOP2\">Backup</label><br><br>\n"
  "</div>\n"
  "<div>   \n"*/
  "<label for=\"_wifiName\">SSID</label><br>\n"
  "<input type=\"text\" class=\"inputPassword\" id=\"_wifiName\" name=\"wifiName\" value=\"");
  webPage += ssid;
  webPage += F("\">");
  webPage += F("<p></p><label for=\"wifiPassword\">PASSWORD</label><br>\n"
  "<input type=\"password\" class=\"inputPassword\" id=\"wifiPassword\" name=\"password\">\n"
  "<br><button id=\"buttonWifi\" onclick=\'getElementById(\"buttonWifi\").innerHTML=\"Saved\" \' type=\"submit\">Save settings</button>\n"
  "</div>\n"
  "</form>\n"
  "</div>"
  "<div class=\"tabpage\" id=\"tabpage_2\">\n"
  "<p></p>\n"
  "<h2>IP settings</h2>\n"
  "<p></p>\n"
  "<form method=\"post\">\n <input type=\"radio\" id=\"dhcpOn\" name=\"dhcpOP\" value=\"1\" ");
  
  if (DHCP == 1) partWebPage = "checked ";
  
  partWebPage += "> \n <label for=\"dhcpOn\">DHCP</label>";
  partWebPage += "<input type=\"radio\" id=\"dhcpOff\"  name=\"dhcpOP\" value=\"0\" ";
  if(DHCP == 0) partWebPage += "checked";
  
  webPage += partWebPage;
  partWebPage = "";
  webPage += F(" >\n       <label for=\"dhcpOff\">Static IP </label><br><br>\n"
  "       <div class=\"staticIpSetting\">       \n"
  "       <label class=\"staticIpSetting\" for=\"staticIp\">IP : (eg.: 192.168.1.10)</label>\n"
  "       <input class=\"staticIpSetting\" type=\"text\" id=\"staticIp\" name=\"staticIp\" value=\"");
  webPage += IP[0];
  webPage += F(".");
  webPage += IP[1];
  webPage += F(".");
  webPage += IP[2];
  webPage += F(".");
  webPage += IP[3];
  webPage += "\">";
  webPage +=F("       <label class=\"staticIpSetting\" for=\"staticMask\">Mask : (eg.: 255.255.255.0)</label> \n"
  "         <input class=\"staticIpSetting\" type=\"text\" id=\"staticMask\" name=\"staticMsk\" value=\"");
  webPage += Subnet[0];
  webPage += ".";
  webPage += Subnet[1];
  webPage += ".";
  webPage += Subnet[2];
  webPage += ".";
  webPage += Subnet[3];
  webPage += "\">";
  webPage +=F("       <label class=\"staticIpSetting\" for=\"staticGate\">Gateway : (eg.: 192.168.1.1)</label>\n"
  "       <input class=\"staticIpSetting\" type=\"text\" id=\"staticGate\" name=\"staticGtw\" value=\"");
  webPage += Gateway[0];
  webPage += ".";
  webPage += Gateway[1];
  webPage += ".";
  webPage += Gateway[2];
  webPage += ".";
  webPage += Gateway[3];
  webPage += "\">\n";
  webPage += F("     <label class=\"staticIpSetting\" for=\"staticDns\">DNS : (eg.: 192.168.1.1)</label>\n "
  "       <input class=\"staticIpSetting\" type=\"text\" id=\"staticDns\" name=\"staticDn\" value=\"");
  webPage += DNS[0];
  webPage += ".";
  webPage += DNS[1];
  webPage += ".";
  webPage += DNS[2];
  webPage += ".";
  webPage += DNS[3];
  webPage += F("\">\n </div>\n");
  server.sendContent(webPage);
  partWebPage = "";
  /*partWebPage = F("<div>\n"
  "<input class=\"radio-inline\" type=\"radio\" id=\"ipConfOpt1\" name=\"MainIpConf\" value=\"0\" checked>\n"
  "<label for=\"ipConfOpt1\">Main</label>\n"
  "<input type=\"radio\" id=\"ipConfOpt2\"  name=\"MainIpConf\" value=\"1\">\n"
  "<label for=\"ipConfOpt2\">Backup</label><br>\n"
  "</div>\n");*/
  partWebPage +=F("<br><button id=\"buttonIpSettings\" type=\"submit\">Save settings</button>\n");
  partWebPage += F("</form>\n"
  "       </div>\n"
  "   <div class=\"tabpage\" id=\"tabpage_3\" >\n"
  "       <button onclick=\"location.href = \'/Quit\';\" id=\"buttonTelgram\" name=\"buttonQuit\" type=\"submit\"  value=\"1\">Quit</button>\n"
  "       </div>\n"
  "       </div>"
  "    </div>\n"
  "  </div>\n"
  "</div>\n"
  "</body>\n"
  "</html>\n");
      server.sendContent(partWebPage);
     //server.send(200,"text/html",webPageFirstPart);
     //webPageFirstPart += webPage;
     //Serial.println(webPageFirstPart);
     //Serial.println("\n\n");
     //Serial.println(webPage);
     //server.send(200,"text/html",webPageFirstPart);
     webPage = "";
    partWebPage = "";
}

void quitPage()
{
  String webPage = "";
  if(server.hasArg("buttonQuit"))
  {
    //Serial.printf("[Quit]buttonQuit: %s\n",server.arg("buttonQuit").c_str());
    quitWebServer = true;
  }
  quitWebServer = true;
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  webPage =F("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
  "<head>"
  "<meta http-equiv=\"Content-Type\" content=\"width=device-width, initial-scale=1.0\" charset=\"utf-8\" />"
  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
  "<title>Sensorbox</title>"
  "<style>"
  "*, *:before, *:after {"
  "-moz-box-sizing: border-box;"
  "-webkit-box-sizing: border-box;"
  "box-sizing: border-box;"
  "}"
  "html {"
  "font-family: Helvetica, Arial, sans-serif;"
  "font-size: 100%;"
  "background: #333;"
  "}"
  "#page-wrapper {"
  "width: auto;"
  "height: auto;"
  "background: #FFF;"
  "padding: 01%;"
  "margin: 1em auto;"
  "border-top: 5px solid #70d0d0;"
  "box-shadow: 0 2px 10px rgba(0,0,0,0.8);"
  "align-items: center;"
  "}");
  server.sendContent(webPage);
  webPage = F( ".box{"
  "  display: block;"
  "  align-items: center;"
  " border: 1px solid lightgrey;"
  " border-radius: 3px;"
  " padding: 5px;"
  " width: auto;"
  "  color:#70d0d0;"
  "  text-align: center;"
  "}"
  "</style>"
  "</head>"
  "<body>"
  "  <div  id=\"page-wrapper\">"
  "  <div  class=\"box\">"
  "    <div id=tabscontent>"
  "    <p><h2>Sisgen</h2></p>"
  "    <p> Technology for all.</p>"
  "  </div>"
  "  </div>"
  "  </div>"
  "</body>");
  server.sendContent(webPage);
  webPage="";
}
