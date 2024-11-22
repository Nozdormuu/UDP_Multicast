#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Ws2tcpip.h> 
#include<iostream>
#pragma warning(disable : 4996)

#define MULTICAST_IP "239.0.0.1" // Мультикаст-адрес
#define PORT 12345               // Порт для приема
#define BUFFER_SIZE 1024

#pragma comment (lib, "Ws2_32.lib")

int main() {
   system("chcp 1251");
   WSADATA wsaData;
   SOCKET sock;
   struct sockaddr_in localAddr;
   struct ip_mreq mreq;
   char buffer[BUFFER_SIZE];
   int addrlen = sizeof(localAddr);

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

   // Разрешение использования одного и того же порта несколькими сокетами
   u_int yes = 1;
   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0) {
      printf("Ошибка установки опции SO_REUSEADDR\n");
      closesocket(sock);
      WSACleanup();
      return 1;
   }

   // Привязка сокета к локальному адресу и порту
   memset(&localAddr, 0, sizeof(localAddr));
   localAddr.sin_family = AF_INET;
   localAddr.sin_port = htons(PORT);
   localAddr.sin_addr.s_addr = htonl(INADDR_ANY); 

   if (bind(sock, (struct sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
      printf("Ошибка bind: %d\n", WSAGetLastError());
      closesocket(sock);
      WSACleanup();
      return 1;
   }

   // Присоединение к мультикаст-группе
   mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
   mreq.imr_interface.s_addr = INADDR_ANY; 

   if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
      printf("Ошибка setsockopt IP_ADD_MEMBERSHIP: %d\n", WSAGetLastError());
      closesocket(sock);
      WSACleanup();
      return 1;
   }

   printf("Присоединились к мультикаст-группе %s:%d, ожидание сообщений...\n", MULTICAST_IP, PORT);

   // Цикл приема сообщений
   while (1) {
      memset(buffer, 0, BUFFER_SIZE); 
      int recvLen = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
      if (recvLen == SOCKET_ERROR) {
         printf("Ошибка recvfrom: %d\n", WSAGetLastError());
      }
      else {
         buffer[recvLen-1] = '\0'; 
         std::cout << buffer << std::endl;
      }
   }
   // Отключение от мультикаст-группы и очистка
   if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
      printf("Ошибка setsockopt IP_DROP_MEMBERSHIP: %d\n", WSAGetLastError());
   }

   closesocket(sock);
   WSACleanup();

   return 0;
}
