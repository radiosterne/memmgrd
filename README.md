memmgrd
=======

English description:
-------
- Term paper on Operating systems.
- Consists of three projects:
- 1) memmgrd: UNIX-daemon, providing client application memory manager 
   functionality. Interprocess comunication is inplemented using
   Unix Domain Socket, authentication algorithm is using
   keccak family of hash functions (a.k.a SHA-3), memory manager is 
   utilizing the "best fitting" distribution algorithm. For every 
   incoming connection a new thread is created (multithreading
   implemented with extensive usage of pthreads library).
   Loads config from file, provides log.
- 2) memmgrtest: Test application allocating one memory block in
   manager's memory.
- 3) memmgrstate: Control and test application capable of sending
   the daemon a quit signal and outputting memory state at a given time.

Описание по-русски:
-------
- Курсовая работа по дисциплине "Операционные системы".
- Состоит из трёх проектов:
- 1) memmgrd: UNIX-демон, предоставляющий клиентским приложениям свои
   услуги в качестве менеджера памяти. Обмен данными происходит
   при помощи Unix Domian Socket, верификация пользователя для
   совершения административных действий использует алгоритм 
   хэширования keccak, менеджер памяти построен на основе алгоритма
   "наиболее подходящий". Для каждого входящего подключения создаёт
   свой поток обработки (использована библиотека pthreads). Подгружает
   настройки из конфигурационного файла, ведёт лог.
- 2) memmgrtest: Тестовое приложение, размещающее в памяти один блок
   заданного размера.
- 3) memmgrstate: Управляющие приложение, имеющее возможность удаленного
   выключения демона и показа состояния его памяти.
