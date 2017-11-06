#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "math.h"
#define IP "0.0.0.0"
#define PORT 8080

typedef struct Client {
    int socket;
    char nickname[256];
    int unique_id;
    int ids_list[1024];
    char nicknames_list[1024][256];
    int lock;
} Client;

Client* client_init(int socket){
  Client* ret = malloc(sizeof(Client));
  ret -> socket = socket;
  ret -> unique_id = 0;
  ret -> lock = 0;
  return ret;
}

char* recieveMessage(int socket, char* message){
  recv(socket, message, 1024, 0);
  return message;
}

void sendMessage(int socket, char* message){
  send(socket, message, 1024,0);
}


char* int2bin(int n, int num_bytes) {
  char *binary = malloc(8*num_bytes);
  binary[0] = '\0';
  int aux;
  for (int i = 8*num_bytes-1; i >= 0; i--){
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

void stringToBinary(char* s, char* bin) {
    if(s == NULL) bin = ""; /* no input string */
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
    strcpy(bin, binary);
}

void binarytoString(char* bin, char* normal, int length) { // Length esta en Bytes, no bits porsiaca
  for(int i = 0; i < length; i++){
    char data[9];
    strncpy(data, &bin[8*i], 8);
    data[8] = '\0';
    char c = strtol(data, 0, 2);
    normal[i] = c;
  }
  normal[length] ='\0';
}

int binarytoInt(char* msg){
  return (int)strtol(msg, NULL, 2);
}

// El build_package guarda en la variable result, el mensaje de de id, size y payload listo para mandarlo.
void build_package(int id, int payload_size, char* payload, char* result){
  char* id_bin = int2bin(id, 1);
  char* payload_size_bin = int2bin(payload_size, 1);
  strcpy(result, id_bin);
  strcat(result, payload_size_bin);
  strcat(result, payload);
}

// El unbuild_package separa en id, payload_size y payload el mensaje que recibio por el socket.

void unbuild_package(char* message, int* id, int* payload_size, char* payload){

  int id_aux;
  int payload_size_aux;

  char id_section[9];
  strncpy(id_section, message, 8);
  id_section[8] = '\0';
  id_aux = binarytoInt(id_section);

  char payload_size_section[9];
  strncpy(payload_size_section, &message[8], 8);
  payload_size_section[8] = '\0';
  payload_size_aux = binarytoInt(payload_size_section);

  char payload_section[(payload_size_aux*8)+1];//+1];
  strncpy(payload_section, &message[16], payload_size_aux*8);
  payload_section[payload_size_aux*8] = '\0';
  strcpy(payload, payload_section);

  *id = id_aux;
  *payload_size = payload_size_aux;
}

// Estos make y receive que hay para cada funcionalidad son para que no sea el medio codigo en el listener.
void receive_heartbeat_payload_client(char* payload, char* random_string){
  binarytoString(payload, random_string, 1);
}
void make_heartbeat_payload_client(char* random_string, int timestamp, char* payload_to_send){
  char random_string_binary[9];
  stringToBinary(random_string, random_string_binary);
  strcpy(payload_to_send, random_string_binary);
  char* timestamp_string = int2bin(timestamp, 4);
  strcat(payload_to_send,timestamp_string);
  //printf("Timestamp en BINARIO --%s--\n", timestamp_string);
}

void make_matchrequest_answer_payload_client(int answer, char* payload_to_send){
  char* id_binary = int2bin(answer, 1);
  strcpy(payload_to_send, id_binary);
}

void recieve_matchmaking_list_payload_server(Client* client, char* payload, int* waiting_clients_ret){
  // Calculando cuantos clientes estan en espera.
  int pos_in_payload = 0;
  char waiting_clients_binary[4*8+1];
  strncpy(waiting_clients_binary, &payload[0], 4*8);
  pos_in_payload += 4*8;
  waiting_clients_binary[4*8] = '\0';
  int waiting_clients = binarytoInt(waiting_clients_binary);

  for (int i = 0; i < waiting_clients; i++){

    char id_binary[2*8+1];
    strncpy(id_binary, &payload[pos_in_payload], 2*8);
    pos_in_payload += 2*8;
    id_binary[2*8] = '\0';
    client -> ids_list[i] = binarytoInt(id_binary);

    char num_bytes_nickname_binary[2*8+1];
    strncpy(num_bytes_nickname_binary, &payload[pos_in_payload], 8);
    pos_in_payload += 8;
    num_bytes_nickname_binary[2*8] = '\0';
    int num_bytes_nickname = binarytoInt(num_bytes_nickname_binary);
    printf("num_bytes_nickname %i\n", num_bytes_nickname);

    char nickname_binary[num_bytes_nickname*8+1];
    strncpy(nickname_binary, &payload[pos_in_payload], num_bytes_nickname*8);
    pos_in_payload += num_bytes_nickname*8;
    nickname_binary[num_bytes_nickname*8] = '\0';
    char nickname[num_bytes_nickname+1];

    binarytoString(nickname_binary, nickname, 15);//num_bytes_nickname);
    printf("The nickname is: %s\n", nickname);
    strcpy(client -> nicknames_list[i], nickname);
  }
  *waiting_clients_ret = waiting_clients;
}

void* listener_thread(void* client_info){
  Client* client = (Client*)client_info;
  char* msg = malloc(sizeof(char)*1024);
  while(1){
    char* message = recieveMessage(client -> socket, msg);
    int id;
    int payload_size;
    char payload[1024];
    unbuild_package(message, &id, &payload_size, payload);


    if (id == 1){
      char random_string[2];
      receive_heartbeat_payload_client(payload, random_string);

      char payload_to_send[((4+1))*8+1];
      int timestamp_number = (int)time(NULL);
      make_heartbeat_payload_client(random_string, timestamp_number, payload_to_send); // Aca payload es el payload antiguo
      char new_message[1024];
      build_package(id, 4+1, payload_to_send, new_message);
      sendMessage(client -> socket, new_message);
    }
    else if (id == 2){
      client -> unique_id = binarytoInt(payload);
    }

    else if (id == 3){
      int waiting_clients;
      recieve_matchmaking_list_payload_server(client, payload, &waiting_clients);
      printf("Waiting clients are: %i\n", waiting_clients);
      for (int i = 0; i < waiting_clients; i++){
        printf("id: %i, nickname: %s\n", client -> ids_list[i], client -> nicknames_list[i]);

      }
    }

    else if (id == 4){
      char respuesta_bits[16];
      strcpy(respuesta_bits, payload);
      int respuesta_int = binarytoInt(respuesta_bits);
      if (respuesta_int == 1){
        printf("\n\nYour match request has been accepted! (1)\n");
      }
      else{
        printf("\n\nYour match request has been declined! (0)\n");
      }
    }

    else if (id == 5){
      client -> lock = 1;
      char nick_bits[256*8];
      strcpy(nick_bits, &payload[16]);
      char normal[256];
      binarytoString(nick_bits, normal, strlen(nick_bits)/8);
      char id_bits[17];
      strncpy(id_bits, payload, 16);
      id_bits[16] = '\0';
      int id_int = binarytoInt(id_bits);
      printf("\n\nYou have recieved a match request!\n\n");
      printf("Oponent information:\nNickname:%s\nID:%i\n\n", normal, id_int);
      printf("Do you wish to accept the match? \nIngrese 'n' luego enter y luego su opcion(1 = yes/0 = no): \n");
      while (1){
        char respuesta[50];
        scanf("%s",respuesta);
        int respuesta_int = atoi(respuesta);
        if (respuesta_int == 1 || respuesta_int == 0){
          char payload_to_send[1024];
          make_matchrequest_answer_payload_client(respuesta_int, payload_to_send);
          char package[1024];
          build_package(5, 1, payload_to_send, package);
          sendMessage(client -> socket, package);
          client -> lock = 0;
          break;
        }
        else{
          printf("Invalid answer, try again...\n");
        }
      }
      
    }

  }
  return 0;
}

void make_matchmaking_payload_client(char* nickname, char* payload_to_send){
  stringToBinary(nickname, payload_to_send);
}

void make_matchrequest_payload_client(int id_oponente, char* payload_to_send_ret){
  char* payload_to_send = int2bin(id_oponente, 2);
  strcpy(payload_to_send_ret, payload_to_send);
}

// Matchmaking List

void* matchmaking_list_thread(void* client_info){
  Client* client = (Client*)client_info;
  char payload_to_send[0];
  char message[1024];
  build_package(3, 0, payload_to_send, message);
  sendMessage(client -> socket, message);
  printf("Matchmaking list request enviado a server \n");
  return 0;
}

// Matchmaking
void* matchmaking_thread(void* client_info){
  Client* client = (Client*)client_info;
  char nickname[100];
  printf("Ingrese su nickname: ");
  scanf("%s",nickname);
  strcpy(client -> nickname, nickname);
  char payload_to_send[8*strlen(nickname)+1];
  char message[1024];
  make_matchmaking_payload_client(nickname, payload_to_send);
  build_package(2, strlen(nickname), payload_to_send, message);
  sendMessage(client -> socket, message);
  pthread_exit(0);
  return 0;
}

// Menu
void* menu_thread(void* client_info){
  Client* client = (Client*)client_info;
  while (1){
    if (client -> lock == 0){
      char opcion[50];
      printf("Ingrese una opcion: ");
      scanf("%s",opcion);
      int opcion_int = atoi(opcion);
      if (opcion_int == 2){
        //match request
        char id_oponente_string[50];
        printf("Ingrese ID del oponente que desea: ");
        scanf("%s",id_oponente_string);
        int id_oponente = atoi(id_oponente_string);
        printf("id oponente: %i\n", id_oponente);
        char payload_to_send[16+1];
        make_matchrequest_payload_client(id_oponente, payload_to_send);
        char package[2*8+1+2*8];
        build_package(4, 2, payload_to_send, package);
        
        sendMessage(client -> socket, package);
      }

      else if (opcion_int == 3){
        matchmaking_list_thread(client);
      }
    }

  }
  return 0;
}

/* Función que inicializa el cliente en el port
con ip */
int initializeClient(char* ip, int port){
  pthread_t thread0;
  pthread_t thread1;
  pthread_t thread2;
  int clientSocket;
  //char buffer[1024];
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  /*---- Creación del Socket. Se pasan 3 argumentos ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP en este caso) */
  clientSocket = socket(PF_INET, SOCK_STREAM, 0);

  /*---- Configuración de la estructura del servidor ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number */
  serverAddr.sin_port = htons(port);
  /* Setear IP address como localhost */
  serverAddr.sin_addr.s_addr = inet_addr(ip);
  /* Setear todos los bits del padding en 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*---- Conectar el socket al server ----*/
  addr_size = sizeof serverAddr;
  connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
  printf("Connected to server!\n");

  // crear thread que escuche todos los mensajes que llegan del servidor
  Client* client = client_init(clientSocket);
  pthread_create(&thread0, NULL, listener_thread, client);

  // crear thread que hace el matchmaking todos los mensajes que llegan del servidor
  pthread_create(&thread1, NULL, matchmaking_thread, client);
  pthread_join(thread1, NULL);

  // crear thread que hace menu
  pthread_create(&thread2, NULL, menu_thread, client);
  pthread_join(thread0, NULL);
  pthread_join(thread2, NULL);
  

  return 0;
}

int main (int argc, char *argv[])
//Elemento 1 de argv será 1 si es server o 0 si es client
{
  int socket;

  printf("Client\n");
  socket = initializeClient(IP, PORT);


  // }
}
