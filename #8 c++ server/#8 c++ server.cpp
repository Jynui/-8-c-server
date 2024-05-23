#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main() {
    WSADATA wsaData; // Структура для хранения данных о версии Winsock
    ADDRINFO hints; // Структура для хранения информации об адресе
    ADDRINFO* addrResult = nullptr; // Указатель для хранения результата getaddrinfo
    SOCKET ListenSocket = INVALID_SOCKET; // Сокет для прослушивания входящих соединений
    SOCKET ConnectSocket = INVALID_SOCKET; // Сокет для установления соединения
    char recvBuffer[512]; // Буфер для получения данных
    const char* sendBuffer = "Hello from server"; // Сообщение для отправки клиенту

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup не удалось с результатом: " << result << endl;
        return 1;
    }

    // Подготовка структуры hints
    ZeroMemory(&hints, sizeof(hints)); // Обнуление структуры hints
    hints.ai_family = AF_INET;        // Использование IPv4
    hints.ai_socktype = SOCK_STREAM;  // Использование TCP
    hints.ai_protocol = IPPROTO_TCP;  // Протокол TCP
    hints.ai_flags = AI_PASSIVE;      // Заполнение адреса для использования bind

    // Получение адресной информации
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo не удалось с ошибкой: " << result << endl;
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
        WSACleanup(); // Завершение работы с Winsock
        return 1;
    }

    // Создание сокета для прослушивания
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Создание сокета не удалось" << endl;
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
        WSACleanup(); // Завершение работы с Winsock
        return 1;
    }

    // Привязка сокета к адресу
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Привязка не удалась, ошибка: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
        WSACleanup(); // Завершение работы с Winsock
        return 1;
    }

    // Начало прослушивания на сокете
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Прослушивание не удалось, ошибка: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
        WSACleanup(); // Завершение работы с Winsock
        return 1;
    }

    // Принятие входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Принятие соединения не удалось, ошибка: " << WSAGetLastError() << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
        WSACleanup(); // Завершение работы с Winsock
        return 1;
    }

    // Закрытие сокета для прослушивания, так как он больше не нужен
    closesocket(ListenSocket);

    // Цикл для получения и отправки данных
    do {
        ZeroMemory(recvBuffer, 512); // Обнуление буфера для получения данных
        result = recv(ConnectSocket, recvBuffer, 512, 0); // Получение данных от клиента
        if (result > 0) {
            cout << "Получено " << result << " байт" << endl;
            cout << "Полученные данные: " << recvBuffer << endl;

            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0); // Отправка данных клиенту
            if (result == SOCKET_ERROR) {
                cout << "Отправка не удалась, ошибка: " << result << endl;
                closesocket(ConnectSocket); // Закрытие соединения
                freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
                WSACleanup(); // Завершение работы с Winsock
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Закрытие соединения" << endl;
        }
        else {
            cout << "Получение данных не удалось, ошибка: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket); // Закрытие соединения
            freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
            WSACleanup(); // Завершение работы с Winsock
            return 1;
        }
    } while (result > 0);

    // Завершение отправки данных (закрытие отправляющей части сокета)
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Завершение отправки не удалось, ошибка: " << result << endl;
        closesocket(ConnectSocket); // Закрытие соединения
        freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
        WSACleanup(); // Завершение работы с Winsock
        return 1;
    }

    // Закрытие соединения и освобождение ресурсов
    closesocket(ConnectSocket); // Закрытие сокета
    freeaddrinfo(addrResult); // Освобождение памяти, выделенной для addrResult
    WSACleanup(); // Завершение работы с Winsock
    return 0;
}
