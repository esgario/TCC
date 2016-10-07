void ControleModulos(byte funcao, byte indice)
{
  Serial.print(0xF0);
  
  if( funcao == CLEAR )
  {
    switch(indice)
    {
      case 0:
        Serial.print(0b01011010);
        break;
      case 1:
        Serial.print(0b00000010);
        break;
      case 2:
        Serial.print(0b00001000);
        break;
      case 3:
        Serial.print(0b00010000);
        break;
      case 4:
        Serial.print(0b01000000);
        break;
    }
  }
  else if( funcao == SET )
  {
    switch(indice)
    {
      case 0:
        Serial.print(0b10100101);
        break;
      case 1:
        Serial.print(0b00000001);
        break;
      case 2:
        Serial.print(0b00000100);
        break;
      case 3:
        Serial.print(0b00100000);
        break;
      case 4:
        Serial.print(0b10000000);
        break;
    }
  }

  delay(10);
  Serial.print(0xF0);
  Serial.print(0x00);
}

