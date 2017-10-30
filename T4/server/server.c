#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "math.h"
#define IP "0.0.0.0"
#define PORT 8080

/* Función que inicializa el servidor en el port
con ip */
int initializeServer(char* ip, int port){
	int welcomeSocket;
  int newSockets[5];
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	/*---- Creación del Socket. Se pasan 3 argumentos ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP en este caso) */
	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

	/*---- Configuración de la estructura del servidor ----*/
	/* Address family = Internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number */
	serverAddr.sin_port = htons(port);
	/* Setear IP address como localhost */
	serverAddr.sin_addr.s_addr = inet_addr(ip);
	/* Setear todos los bits del padding en 0 */
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*---- Bindear la struct al socket ----*/
	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	/*---- Listen del socket, con un máximo de 5 conexiones (solo como ejemplo) ----*/
  int count = 0;
	while(listen(welcomeSocket,3)==0) {
		printf("Waiting for the next user to connect...\n");

  	addr_size = sizeof serverStorage;
  	newSockets[count] = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
  	printf("Conected\n");
    count += 1;
    for (int i = 0; i < 3; i++){
      printf("Sockets: %i\n", newSockets[i]);

    }
  }

	return 0;
}

char* recieveMessage(int socket, char* message){
  printf("Waiting message... ♔ \n");
  recv(socket, message, 1024, 0);
  return message;
}

void sendMessage(int socket, char* message){
  send(socket, message, 1024,0);
}

char* int2bin(int n) {
  char *binary = malloc(8);
  binary[0] = '\0';
  int aux;
  for (int i = 7; i >= 0; i--){
    aux = n >> i;
    if (aux & 1){
      strcat(binary,"1");
    }
    else{
      strcat(binary,"0");
    }
  }
  return binary;
}

char* stringToBinary(char* s) {
    if(s == NULL) return 0; /* no input string */
    size_t len = strlen(s);
    char *binary = malloc(len*8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
    binary[0] = '\0';
    for(size_t i = 0; i < len; ++i) {
        char ch = s[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat(binary,"1");
            } else {
                strcat(binary,"0");
            }
        }
    }
    return binary;
}

char* binarytoString(char* s) {
  char *data = "01010110";
  char c = strtol(data, 0, 2);
  char* normal = malloc(strlen(&c)*sizeof(char));
  strcpy(normal, &c);
  printf("%s = %c = %d = 0x%.2X\n", data, c, c, c);
  return normal;
}





char* build_package(int id, int payload_size, char* payload){
  //char* ret = malloc((8+8+payload_size)*sizeof(char));
  char* id_string = int2bin(id);
  char* payload_size_string = int2bin(payload_size);
  char *result = malloc(strlen(id_string)+strlen(payload_size_string)+payload_size+1);//+1 for the null-terminator
  //in real code you would check for errors in malloc here
  printf("id: %s\n", id_string);
  printf("Prueba id: %s\n", stringToBinary("2"));
  printf("id: %s\n", payload_size_string);
  printf("id: %s\n", payload);
  strcpy(result, id_string);
  strcat(result, payload_size_string);
  strcat(result, stringToBinary(payload));
  printf("Result: %s\n", result);
  return result;
}

void unbuild_package(char* message, int id, int payload_size, char* payload){
  //char* ret = malloc((8+8+payload_size)*sizeof(char));
  char* id_string = int2bin(id);
  char* payload_size_string = int2bin(payload_size);
  char *result = malloc(strlen(id_string)+strlen(payload_size_string)+payload_size+1);//+1 for the null-terminator
  //in real code you would check for errors in malloc here
  printf("id: %s\n", id_string);
  printf("Prueba id: %s\n", stringToBinary("2"));
  printf("id: %s\n", payload_size_string);
  printf("id: %s\n", payload);
  strcpy(result, id_string);
  strcat(result, payload_size_string);
  strcat(result, stringToBinary(payload));
  printf("Result: %s\n", result);
  return result;
}

int main (int argc, char *argv[])
{
  build_package(2, 3, "abc");
  int socket;
  printf("Server\n");
  socket = initializeServer(IP, PORT);
  int count = 0;
  while (count<10) {
    char* message = malloc(sizeof(char)*1024);
    char* msg = recieveMessage(socket, message);
    printf(msg, "%s\n");
    printf("\nYour Message: ");
    scanf("%s", msg);
    printf("\n");
    sendMessage(socket, msg);

  }
}
