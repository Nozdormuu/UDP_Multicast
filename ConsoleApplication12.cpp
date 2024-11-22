#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Ws2tcpip.h> 
#pragma warning(disable : 4996)

#define MULTICAST_IP "239.0.0.1" // Мультикаст-адрес
#define PORT 12345               // Порт для отправки
#define BUFFER_SIZE 1024

#pragma comment (lib, "Ws2_32.lib")

int main() {
   system("chcp 1251");
   WSADATA wsaData;
   SOCKET sock;
   struct sockaddr_in multicastAddr;
   char buffer[BUFFER_SIZE];
   int ttl = 1; // Время жизни пакета

   // Инициализация Winsock
   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      printf("Ошибка WSAStartup.\n");
      return 1;
   }

   // Создание UDP-сокета
   sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (sock == INVALID_SOCKET) {
      printf("Ошибка создания сокета: %d\n", WSAGetLastError());
      WSACleanup();
      return 1;
   }

   // Установка TTL для мультикаст-пакетов
   if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR) {
      printf("Ошибка setsockopt IP_MULTICAST_TTL: %d\n", WSAGetLastError());
      closesocket(sock);
      WSACleanup();
      return 1;
   }

   // Настройка адреса мультикаст-группы
   memset(&multicastAddr, 0, sizeof(multicastAddr));
   multicastAddr.sin_family = AF_INET;
   multicastAddr.sin_addr.s_addr = inet_addr(MULTICAST_IP);
   multicastAddr.sin_port = htons(PORT);

   printf("Сервер UDP Multicast запущен, отправка на %s:%d...\n", MULTICAST_IP, PORT);
   while (1) {
      // Ввод сообщения для отправки
      printf("Введите сообщение для отправки: ");
      fgets(buffer, BUFFER_SIZE, stdin);

      // Удаление символа перевода строки
      buffer[strcspn(buffer, "\n")] = '0';

      // Проверка на пустое сообщение
      if (strlen(buffer) == 0) {
         printf("Пустое сообщение не будет отправлено.\n");
         continue;
      }

      // Отправка сообщения в мультикаст-группу
      int sendLen = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
      if (sendLen == SOCKET_ERROR) {
         printf("Ошибка sendto: %d\n", WSAGetLastError());
      }
      else {
         printf("Отправлено %d байт в мультикаст-группу %s:%d\n", sendLen, MULTICAST_IP, PORT);
      }
   }

   // Закрытие сокета и очистка
   closesocket(sock);
   WSACleanup();

   return 0;
}
