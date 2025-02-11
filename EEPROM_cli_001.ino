#include <EEPROM.h>

const int EEPROM_SIZE = 1024;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) 
  {
    // ждем подключения
  }
  Serial.println("EEPROM CLI. Enter your commands:");
}

void loop() {
  if (Serial.available()) 
  {
    //чтение и обработка входных данных, выделение команд
    String input_data = Serial.readStringUntil('\n');
		input_data.trim(); 

		if (input_data.length() == 0) 
		{
			return; 
		}
		
		const int BUFFER_SIZE = 100;
		char commandBuffer[BUFFER_SIZE];
		input_data.toCharArray(commandBuffer, BUFFER_SIZE);

		const int MAX_TOKENS = 10;
		char *tokens[MAX_TOKENS];
		int tokenCount = 0;

		char *token = strtok(commandBuffer, " ");
		while (token != NULL && tokenCount < MAX_TOKENS) 
		{
		  tokens[tokenCount++] = token;
		  token = strtok(NULL, " ");
		}

		if (tokenCount == 0) return;

		if (String(tokens[0]) != "eeprom") 
		{						
		  Serial.println("Ошибка: Команда должна начинаться с 'eeprom'");	
		  return;
		}

    bool write = false;
		bool read  = false;
		bool erase = false;
		bool dump  = false;

    //анализ команд/ключевых символов и их обработка
		int addr = -1;
		int value = -1;

		for (int i = 1; i < tokenCount; i++) 
		{
			String currentToken = tokens[i];
			if (currentToken == "-w") 
      {
				write = true;
			}
			else if (currentToken == "-r") 
      {
				read = true;
			}
			else if (currentToken == "-e") 
      {
				erase = true;
			}
			else if (currentToken == "-d") 
      {
				dump = true;
			}
			else if (currentToken == "-a") 
      {
				// следующий токен должен быть адресом
				if (i + 1 < tokenCount) 
        {
				  addr = atoi(tokens[i + 1]);
				  i++; // пропускаем значение адреса
				}
				else {
					Serial.println("Ошибка: Не указан адрес после -a");
					return;
				}
			}
			else if (currentToken == "-v") 
      {
				// следующий токен должен быть значением
				if (i + 1 < tokenCount) 
        {
					value = atoi(tokens[i + 1]); // преобразование из строки к числу (DEC)
					i++; // пропускаем значение
				}
				else 
        {
					Serial.println("Ошибка: Не указано значение после -v");
					return;
				}
			}
			else 
      {
				Serial.print("Внимание: неизвестный параметр '");
				Serial.print(currentToken);
				Serial.println("'");
			}
		}
    //ошибки
    //только одна операция должна быть (-w, -r, -e или -d) выбрана
		int operationCount = (int)write + (int)read + (int)erase + (int)dump;
		if (operationCount != 1) 
		{
			Serial.println("Ошибка: укажите ровно одну операцию: -w, -r, -e или -d");
			return;
		}

    //описание команд
    if (write) 
		{
			// команда записи: eeprom -w -a <address> -v <value>
			if (addr < 0) 
			{
				Serial.println("Ошибка: не указан корректный адрес для записи (-a <address>)");
				return;
			}
			if (value < 0) 
			{
				Serial.println("Ошибка: не указано корректное значение для записи (-v <value>)");
				return;
			}
			// проверка диапазона адреса и значения
			if (addr >= EEPROM_SIZE) 
			{
				Serial.println("Ошибка: адрес выходит за пределы памяти EEPROM");
				return;
			}
			if (value < 0 || value > 255) 
			{
				Serial.println("Ошибка: значение должно быть в диапазоне 0-255");
				return;
			}
			EEPROM.write(addr, value);
			Serial.print("Запись выполнена успешно!");
		}
		else if (read) 
		{
			// команда чтения: eeprom -r -a <address>
			if (addr < 0) 
			{
				Serial.println("Ошибка: не указан корректный адрес для чтения (-a <address>)");
				return;
			}
			if (addr >= EEPROM_SIZE) 
			{
				Serial.println("Ошибка: адрес выходит за пределы памяти EEPROM");
				return;
			}
			byte readValue = EEPROM.read(addr);
			Serial.print("Чтение выполнено успешно!");
		}
		else if (erase) 
		{
			// команда стирания: eeprom -e -a <address>
			// считаем, что стирание означает запись 0 в заданную ячейку.
			if (addr < 0) {
				Serial.println("Ошибка: не указан корректный адрес для стирания (-a <address>)");
				return;
			}
			if (addr >= EEPROM_SIZE) {
				Serial.println("Ошибка: адрес выходит за пределы памяти EEPROM");
				return;
			}
			EEPROM.write(addr, 0);
			Serial.print("Стирание выполнено успешно!");
		}
		else if (dump) 
		{
			// команда дампа: eeprom -d
			Serial.println("Дамп EEPROM:");
			char buffer[50];
			// выводим по 8 ячеек в строке
			for (int i = 0; i < EEPROM_SIZE; i += 8) 
			{
				// вывод адреса в виде 4-х значного HEX числа
				sprintf(buffer, "%04X : ", i);
				Serial.print(buffer);
				// вывод 8 байт
				for (int j = 0; j < 8; j++) 
				{
					int index = i + j;
					if (index < EEPROM_SIZE) 
					{
						byte cell = EEPROM.read(index);
						sprintf(buffer, "%02X ", cell);
						Serial.print(buffer);
					}
				}
				Serial.println();
			}
			Serial.println("Дамп выполнен успешно!");
		}
    //статусное сообщение
  }
}
