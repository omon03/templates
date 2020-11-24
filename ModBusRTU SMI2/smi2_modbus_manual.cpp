/*!
    Файл содержит библиотеку для работы с шиной Modbus
    и пример её использования
*/
#define ADDRESS 16
//!*************************************************************
//!-----------------Библиотека работы с Modbus------------------
//!*************************************************************
    //! Порт подключения шины Modbus
    #define MB_PORT_INDEX 2

    //! Скорость подключения по шине Modbus
    #define MB_PORT_SPEED 19200    

    //! Количество стопбит
    #define MB_PORT_STOP 0  

    //! Размер буфера для обмена по шине Modbus
    #define MB_BUF_SIZE 256

    //! Таймаут ожидания ответа слейва, мс
    #define MB_TIMEOUT 10000

    //! Коды ошибок, возвращаемые функциями обмена по шине
    #define MB_ERR_OK 0      //!< Успешный обмен данными
    #define MB_ERR_CMD 1     //!< Принятый код функции не может быть обработан
    #define MB_ERR_ADDR 2    //!< Адрес данных, указанный в запросе, недоступен
    #define MB_ERR_VALUE 3   //!< Значение, содержащееся в поле данных запроса, является недопустимой величиной
    #define MB_ERR_ERR 4     //!< Невосстанавливаемая ошибка имела место, пока ведомое устройство пыталось выполнить затребованное действие
    #define MB_ERR_WAIT 5    //!< Ведомое устройство приняло запрос и обрабатывает его, но это требует много времени. Этот ответ предохраняет ведущее устройство от генерации ошибки тайм-аута
    #define MB_ERR_BUSY 6    //!< Ведомое устройство занято обработкой команды. Ведущее устройство должно повторить сообщение позже, когда ведомое освободится
    #define MB_ERR_CNTEXEC 7 //!< Ведомое устройство не может выполнить программную функцию, заданную в запросе
    #define MB_ERR_PARITY 8  //!< Ведомое устройство при чтении расширенной памяти обнаружило ошибку паритета
    #define MB_ERR_CRC 9     //!< Ошибка контрольной суммы
    #define MB_ERR_SIZE 10   //!< Получен ответ некорректного размера

    //! Начальная позиция полезных данных при команде "чтение"
    #define MB_READ_START 3

    //! Начальная позиция полезных данных при команде "запись"
    #define MB_WRITE_START 7

    //! Инициализация шины Modbus
    modbusInit()
    {
        PortInit(MB_PORT_INDEX, MB_PORT_SPEED, MB_BUF_SIZE , MB_PORT_STOP);
    }

    //! Запись значения 16-битного регистра в буфер в формате Modbus
    //! \param[in] mbBuf Буфер, куда производится запись
    //! \param[in] offset Смещение указателя записи от начала буфера
    //! \param[in] value Значение записываемого регистра
    setReg(mbBuf{}, offset, value)
    {
        mbBuf{offset} = value >> 8 //Старший байт вперед
        mbBuf{offset + 1} = value //Младший байт
    }

    //! Функция возвращает значения 16-битного регистра Modbus из буфера
    //! \param[in] mbBuf Буфер, откуда производится чтение
    //! \param[in] offset Смещение указателя чтения от начала буфера
    //! \return Конвертированное значение регистра
    getReg(mbBuf{}, offset)
    {
        return mbBuf{offset} * 256 + mbBuf{offset + 1}
    }

    //! Функция возвращает значения 32-битного регистра Modbus из буфера
    //! \param[in] mbBuf Буфер, откуда производится чтение
    //! \param[in] offset Смещение указателя чтения от начала буфера
    //! \return Конвертированное значение регистра
    getReg32(mbBuf{}, offset)
    {
        return (mbBuf{offset} << 24) + (mbBuf{offset + 1} << 16) + (mbBuf{offset + 2} << 8) + (mbBuf{offset + 3})
    }

    //! 0x03 - Функция чтение значений из нескольких регистров хранения (Read Holding Registers)
    //! \param[in] addr Адрес ведомого устройства
    //! \param[out] mbBuf Буфер обмена, будет перезаписан считанными данными
    //! \param[in] fRegAddr Адрес первого элемента, значение которого требуется прочитать
    //! \param[in] count Количество считываемых элементов
    //! \return Код ошибки MB_ERR_*. MB_ERR_OK если транзакция прошла успешно
    readRegs(addr, mbBuf{}, fRegAddr, count)
    {
        mbBuf{0} = addr
        mbBuf{1} = 0x03 //Код функции
        setReg(mbBuf, 2, fRegAddr) //Адрес первого элемента
        setReg(mbBuf, 4, count) //Количество считываемых элементов
        return transact(mbBuf, 6)
    }

    //! 0x04 - Функция чтение значений из нескольких регистров хранения (Read Input Registers)
    //! \param[in] addr Адрес ведомого устройства
    //! \param[out] mbBuf Буфер обмена, будет перезаписан считанными данными
    //! \param[in] fRegAddr Адрес первого элемента, значение которого требуется прочитать
    //! \param[in] count Количество считываемых элементов
    //! \return Код ошибки MB_ERR_*. MB_ERR_OK если транзакция прошла успешно
    readRegsIn(addr, mbBuf{}, fRegAddr, count)
    {
        mbBuf{0} = addr
        mbBuf{1} = 0x04 //Код функции
        setReg(mbBuf, 2, fRegAddr) //Адрес первого элемента
        setReg(mbBuf, 4, count) //Количество считываемых элементов
        return transact(mbBuf, 6)
    }

    //! 0x10 - Функция записи значений в несколько регистров хранения (Preset Multiple Registers)
    //!
    //! Функция заполнит заголовок буфера и отправит его в шину, полезные данные требуется располагать
    //! начиная с позиции \a MB_WRITE_START. Записываемые значения все 16-битные
    //! \param[in] addr Адрес ведомого устройства
    //! \param[in] mbBuf Буфер обмена, который будет отправлен
    //! \param[in] fRegAddr Адрес первого элемента, значение которого требуется записать
    //! \param[in] count Количество записываемых элементов
    //! \return Код ошибки MB_ERR_*. MB_ERR_OK если транзакция прошла успешно
    writeRegs(addr, mbBuf{}, fRegAddr, count)
    {
        mbBuf{0} = addr
        mbBuf{1} = 0x10 //Код функции
        setReg(mbBuf, 2, fRegAddr) //Адрес первого элемента
        setReg(mbBuf, 4, count) //Количество считываемых элементов
        mbBuf{6} = count * 2
        return transact(mbBuf, 7 + count * 2)
    }

    //!----------------- служебные функции MODBUS ------------------
    //! Обмен с устройством данными по шине
    //! \param[inout] mbBuf Буфер обмена, данные из него будут записаны с добавленной CRC, и в него будут считан ответ без CRC
    //! \param[inout] size Размер буфера обмена
    //! \return Код ошибки MB_ERR_*. MB_ERR_OK если транзакция прошла успешно
    transact(mbBuf{}, size)
    {    
        //Добавляем CRC
        new c = CRC16(mbBuf, size)
        mbBuf{size} = c
        mbBuf{size + 1} = c >> 8
        //Пишем в порт
        PortWrite(MB_PORT_INDEX, mbBuf, size + 2)
        //Размер ответа, для записывающих команд размер известен из стандарта
        new ansSize = (mbBuf{1} > 4) ? 8 : MB_BUF_SIZE + 1
        //Слушаем ответ
        new err = 0
        new bytesRead = 0  
        c = 0    
        while (PortRead(MB_PORT_INDEX, c, MB_TIMEOUT))
        {
            mbBuf{bytesRead} = c
            bytesRead++

            if ((bytesRead >= MB_BUF_SIZE) || (bytesRead >= ansSize))
                break

            //Ошибка ли?
            if (bytesRead == 2)
                err = (c & 0x80 == 0x80)

            //Определяем длину сообщения, либо код ошибки
            if (bytesRead == 3)
            {
                if (err)
                {
                    err = c
                    ansSize = 5
                }
                else 
                {
                    if (ansSize == MB_BUF_SIZE + 1) //Считываем размер только для операций чтения
                        ansSize = 3 + c + 2
                }
            }
        }    
        //Сверяем целевой и фактический размеры ответа
        if (ansSize != bytesRead)
        {
            Diagnostics("Mer[%d]", MB_ERR_SIZE)
            Diagnostics("aS:%d != br:%d", ansSize, bytesRead)
            
            return MB_ERR_SIZE
        }

        printData(mbBuf, bytesRead)
        //Проверяем CRC
        c = mbBuf{ansSize - 2} + mbBuf{ansSize - 1} * 256
        if (c != CRC16(mbBuf, ansSize - 2))
            return MB_ERR_CRC
        //Проверки пройдены, возвращаем код результата(ошибки)
        return err
    }

    //! Функция вывода содержимого буфера \a buf длиной \a len
    printData(buf{}, len)
    {
        // вывод по 2 байта в строке
        for(new i = 0; i < len / 2; i++)
        {
            Diagnostics("%02X %02X", buf{i * 2}, buf{i * 2 + 1})
            Delay(10)
        }
        // вывод последнего байта при нечётной длине
        if (len % 2)
            Diagnostics("%02X", buf{len - 1})
    }

    //! Функция записи значения регистра в тег
    readRegWriteTag(mbBuf{}, tag, valueName{}, regAdr)
    {
        Diagnostics(valueName)
        new res = readRegs(ADDRESS, mbBuf, regAdr, 1)
        if (!res)
        {
            res = getReg(mbBuf, MB_READ_START)
            // Diagnostics("value %x: in tag: %d", res, regAdr)
            TagWriteValue(tag, res)
        }
        else
            Diagnostics("Error code: %d in tag: %x", res, regAdr)
    }

    //! Функция записи значения регистров в тег
    readReg32WriteTag(mbBuf{}, tag, valueName{}, regAdr)
    {
        Diagnostics(valueName)
        new res = readRegs(ADDRESS, mbBuf, regAdr, 2)
        if (!res)
        {
            res = getReg32(mbBuf, MB_READ_START)
            // Diagnostics("value %x: in tag: %d", res, regAdr)
            TagWriteValue(tag, res)
        }
        else
            Diagnostics("Error code: %d in tag: %x", res, regAdr)
    }
//!*************************************************************
//!-----------Конец библиотеки работы с Modbus------------------
//!*************************************************************

main()
{
    /*! Введение.

        Протокол модбас широко распространен во всем мире, чаще всего используется в умных домах и для обмена данными с периферией 
        в качестве периферии у нас будет 4х символьный дисплей СМИ2, на его примере и будем разбирать принципы работы протокола и 
        нашей библиотеки.

        Особенностями Modbus-RTU, а именно такой протокол используется в нашем случае, это то что он является бинарным, тоесть 
        информация в нем передается байтами, а не символами.
        Тоесть полученное сообщение нельзя сразу прочитать, его приходится расшифровывать, также как и специальным образом 
        компоновать передаваемую информацию.
        Функцию по компановке сообщений выполняет наша библиотека Modbus, которая расположена выше, изменять что-либо в ней можно, 
        но не нужно, в общем случае это и не требуется, просто пользуемся ей и всё должно работать.

        По протоколу модбас информация на периферии хранится в регистрах, например температурный датчик, который может в модбас, 
        измерил температуру воздуха, и записал результат себе в регистр с определенным номером, например №1.
        Наша задача обращаться к этим регистрам с информацией и считывать оттуда данные, или наоборот записывать в регистры данные, 
        чтобы, например, СМИ2 стал отображать на своем дисплее записанное.
        Один регистр содержит два байта информации, если например температура занимает место - 4 байта, то используются два 
        соседних регистра.
        При считывании или записи, мы указываем в аргументах номер первого регистра и следующим аргументом указываем количество 
        регистров которое следует считать/записать разом.
        У каждого устройства периферии есть адрес к которому нам нужно обращаться, он записан как ADDRESS в самом верху библиотеки, 
        его следует указывать исходя из настроек периферии.
    */
    
    // создаем массив(буфер) в который будем записывать данные перед отправкой на устройство
    new mb_command{MB_BUF_SIZE} = 0 
    
    // инициализируем работу порта
    modbusInit()  

    // скомпануем посылаемое сообщение (это 4) в специальной структуры сообщение.
    setReg(mb_command, MB_WRITE_START, 4) 

    //отправим по адресу (ADDRESS) скомпанованную команду(mb_command) в нужный нам регистр ( 17 ), записать 
    // нужно только в один регистр, поэтому ( 1 )
    writeRegs(ADDRESS, mb_command, 17, 1) 

    //отправим по аналогии указанной выше команду на устройство (примени настройки)
    //скомпануем
    setReg(mb_command, MB_WRITE_START, 0x81)
    //отправим на устройство
    writeRegs(ADDRESS, mb_command, 15, 1)

    // Теперь наш СМИ2 переведен в режим "ПОРТРЕТ".
    // Зажгем все 4 верхних сегмента.
    // Так как протокол модбас подразумевает регистры только по два байта то комманду придется формировать дважды.
    // Судя по инструкции за верхний сегмент (А) отвечает бит №7, то есть чтобы горел только верхний сегмент, наш 
    // байт должен выглядеть следующим образом:
    // 1 0 0 0 0 0 0 0 - если перевести в человеческий вид = 127 в десятичном формате, или = 0x80 в шестнадцатеричном(HEX)
    setReg(mb_command, MB_WRITE_START, 0x8080)
    setReg(mb_command, MB_WRITE_START + 2, 0x8080) //+2 потому что выше мы уже записали 2 байта, и тут пишем следующие два.
    // Отправляем комманду на устройство, на этот раз записывать на устройство придется уже в два регистра подряд 
    writeRegs(ADDRESS, mb_command, 33, 2)
    Delay(10000)
    // Теперь включим все верхние, все средние и все нижние сегменты.
    // Через калькулятор выбираем как выглядит код.
    // 1 0 0 1 0 0 1 0 - это =146(DEC) =0x92(HEX) нагляднее пользоваться HEX
    setReg(mb_command, MB_WRITE_START, 0x9292)
    setReg(mb_command, MB_WRITE_START + 2, 0x9292) //+2 потому что выше мы уже записали 2 байта, и тут пишем следующие два.
    // Отправляем комманду на устройство, также 2 регистра по 2 байта
    writeRegs(ADDRESS, mb_command, 33, 2)
    Delay(10000)

    // Обращаю внимание, та информация, что записана в регистре, она там остается пока не отключим питание, поэтому если 
    // например послать команду на запись только одного регистра или даже одного байта в регистре, информация в остальных 
    // останется прежней.
    setReg(mb_command, MB_WRITE_START, 0x92)
    writeRegs(ADDRESS, mb_command, 33, 1)
    Delay(10000)

    // Пропадут сегменты только на крайнем левом 8-сегментном элементе.
    // Очистим второй слева элемент дисплея.
    setReg(mb_command, MB_WRITE_START, 0x9200)
    writeRegs(ADDRESS, mb_command, 33, 1)
    Delay(10000)

    //Исходя из спецификации, минимальное время смены символов - 200 мс.
    //Сейчас покажу фишку над которой поразмышляйте сами.
    setReg(mb_command, MB_WRITE_START, 0x0000)
    setReg(mb_command, MB_WRITE_START + 2, 0x0000)
    writeRegs(ADDRESS, mb_command, 33, 2)

    new j = 0

    while (j < 3)
    {
        new a = 2  //исключим парочку сегментов для красоты представления

        for (new i = 0; i < 6; i++)
        {
            a *= 2
            setReg(mb_command, MB_WRITE_START, a)  // тут значение принимает вид 0x00|_a|
            writeRegs(ADDRESS, mb_command, 33, 1)
            Delay(200)
        }

        j++
    }


    setReg(mb_command, MB_WRITE_START, 0x0000)
    setReg(mb_command, MB_WRITE_START + 2, 0x0000)
    writeRegs(ADDRESS, mb_command, 33, 2)

    j = 0
    while (j < 3)
    {
        new a = 2 //исключим парочку сегментов для красоты представления
        for (new i = 0; i < 6; i++)
        {
            a = a*2
            setReg(mb_command, MB_WRITE_START, a * 256) //256 это 0xFF, тоесть таким способом можно сдвинуть значение на один байт (0x|_a|00)
            writeRegs(ADDRESS, mb_command, 33, 1)
            Delay(200)
        }

        j++
    }

    setReg(mb_command, MB_WRITE_START, 0x0000)
    setReg(mb_command, MB_WRITE_START + 2, 0x0000)
    writeRegs(ADDRESS, mb_command, 33, 2)

    j = 0
    while (j < 3)
    {
        new a = 2 //исключим парочку сегментов для красоты представления
        for (new i = 0; i < 6; i++)
        {
            a = a*2
            setReg(mb_command, MB_WRITE_START, a * 256)
            setReg(mb_command, MB_WRITE_START + 1, a * 256)
            setReg(mb_command, MB_WRITE_START + 2, a * 256)
            setReg(mb_command, MB_WRITE_START + 3, a * 256)
            writeRegs(ADDRESS, mb_command, 33, 1)
            Delay(200)
        }

        j++
    }
}