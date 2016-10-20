void ControleModulos(byte funcao, byte indice)
{
  Serial.write(0xF0);

  if ( funcao == CLEAR )
  {
    switch (indice)
    {
      case 0:
        Serial.write(0b01011010);
        break;
      case 1:
        Serial.write(0b00000010);
        break;
      case 2:
        Serial.write(0b00001000);
        break;
      case 3:
        Serial.write(0b00010000);
        break;
      case 4:
        Serial.write(0b01000000);
        break;
    }
  }
  else if ( funcao == SET )
  {
    switch (indice)
    {
      case 0:
        Serial.write(0b10100101);
        break;
      case 1:
        Serial.write(0b00000001);
        break;
      case 2:
        Serial.write(0b00000100);
        break;
      case 3:
        Serial.write(0b00100000);
        break;
      case 4:
        Serial.write(0b10000000);
        break;
    }
  }

}

void recebeDadosSerial()
{
  if (Serial.available())
  {
    if (Serial.read() == 0xFF)
    {
      delay(1);
      if (Serial.read() == 0xFF)
      {
        delay(1);
        byte i = 0;
        chegouDadosPic = true;
        
        while (Serial.available() && i < 14)
        {
          MSG_Pic[i] = Serial.read();
          i++;
          delay(1);
        }
      }
    }
  }
}

