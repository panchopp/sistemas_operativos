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

// Agregue estos structs asi uno le puede pasar mas que un solo puntero al thread, por ejemplo a un thread si le pasamos Client,
// podemos en client guardar cosas e incluso modificarlo desde otro thread.
// Un ejemplo de esto podria ser pasar un mensaje de un client a otro

typedef struct Client {
    int socket;
    int heartbeat_count;
    int activo; // Que sea 2 si esta jugando, 1 si esta en espera y 0 si ya no juega
    int unique_id;
    char nickname[256];
    struct Server* server;
} Client;

typedef struct Server {
    Client* clients[2048];
    Client* pos_player1;
    Client* pos_player2;
    int num_clients;
} Server;

Client* client_init(int socket, Server* server){
  Client* ret = malloc(sizeof(Client));
  ret -> socket = socket;
  ret -> heartbeat_count = 0;
  ret -> activo = 1;
  ret -> server = server;
  return ret;
}

Server* server_init(){
  Server* ret = malloc(sizeof(Server));
  // ret -> clients = malloc(sizeof(Client*)*1000);
  ret -> pos_player1 = NULL;
  ret -> pos_player2 = NULL;
  ret -> num_clients = 0;
  return ret;
}

void add_client_to_server(Server* server, Client* client){
  server -> clients[server -> num_clients] = client;
  client -> unique_id = server -> num_clients; // El unique ID es el numero en que el usuario esta en la lista de usuarios.
  server -> num_clients += 1;
}


static char rand_string()
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    int key = rand() % (int) (sizeof charset - 1);
    return charset[key];
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

void receive_heartbeat_payload_server(char* payload, char* random_string, int* timestamp){
  char random_string_binary[9];
  strncpy(random_string_binary, payload, 8);
  binarytoString(random_string_binary, random_string, 1);

  char timestamp_binary[4*8+1];
  strcpy(timestamp_binary, &payload[8]);
  *timestamp = binarytoInt(timestamp_binary);

}
void make_heartbeat_payload_server(char* random_string, char* payload_to_send){
  stringToBinary(random_string, payload_to_send);
}

void* heartbeat_server(void* client_info){
  Client* client = (Client*)client_info;
  int s = client -> socket;

  while(1){
    int id = 1;
    int payload_size = 1;
    char random_string[2];
    random_string[0] = rand_string();
    random_string[1] = '\0';
    char payload_to_send[8];

    make_heartbeat_payload_server(random_string, payload_to_send);

    char message[1024];
    build_package(id, payload_size, payload_to_send, message);

    sendMessage(s, message);
    client -> heartbeat_count += 1;
    sleep(10);
    if (client -> heartbeat_count >= 3){
      // ACA SE MATA AL CLIENT
      return 0;
    }
  }
  return 0;
}

void make_matchmaking_payload_server(int id, char* payload_to_send){
  char* id_binary = int2bin(id, 2);
  strcpy(payload_to_send, id_binary);
}

void make_matchmaking_list_payload_server(Client* client, char* payload_to_send){
  int waiting_clients = 0;
  // Calculando cuantos clientes estan en espera.
  for (int i = 0; i < client -> server -> num_clients; i++){
    if (client -> server -> clients[i] -> activo == 1){
      waiting_clients += 1;
    }
  }
  char* waiting_clients_binary = int2bin(waiting_clients, 4);
  strcpy(payload_to_send, waiting_clients_binary);

  for (int i = 0; i < client -> server -> num_clients; i++){
    if (client -> server -> clients[i] -> activo == 1){
      char* id_binary = int2bin(client -> server ->clients[i] -> unique_id, 2);
      printf("id_binary%s\n", id_binary);
      strcat(payload_to_send, id_binary);
      printf("El nickname del palyer es: --%s--\n", client -> server ->clients[i] -> nickname);
      int num_bytes_nickname = strlen(client -> server ->clients[i] -> nickname);
      char* num_bytes_nickname_binary = int2bin(num_bytes_nickname, 1);

      printf("num_bytes_nickname_binary%s\n", num_bytes_nickname_binary);
      strcat(payload_to_send, num_bytes_nickname_binary);
      char nickname_binary[num_bytes_nickname*8+1];
      stringToBinary(client -> server ->clients[i] -> nickname, nickname_binary);
      nickname_binary[num_bytes_nickname*8] = '\0';
      printf("nickname_binary%s\n", nickname_binary);
      printf("6\n");
      strcat(payload_to_send, nickname_binary);
      printf("7\n");
    }
  }
}

void make_matchrequest_payload_server(int id, char* nickname, char* payload_to_send){
  char* id_binary = int2bin(id, 2);
  char nick_binary[256*8];
  stringToBinary(nickname, nick_binary);
  strcpy(payload_to_send, id_binary);
  strcat(payload_to_send, nick_binary);
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
      int timestamp;
      char random_string[1];
      receive_heartbeat_payload_server(payload, random_string, &timestamp);
      client -> heartbeat_count -= 1;
    }
    else if (id == 2){

      // Como los return de char* dejaban la cagada los mate jajja
      // La mayoria de las funciones uno primero crea los strings con los largos respectivos
      // y se los pasa a la funcion y la funcion los modifica adentro de ella


      char nickname[payload_size+1];
      binarytoString(payload, nickname, payload_size);
      nickname[payload_size] = '\0';
      printf("Nickname: %s\n", nickname);
      strcpy(client -> nickname, nickname);
      char payload_to_send[2*8+1];
      char message[1024];
      make_matchmaking_payload_server(client -> unique_id,payload_to_send);
      build_package(2, 2, payload_to_send, message);
      sendMessage(client -> socket, message);
    }
    else if (id == 3){
      char payload_to_send[1024-16];
      char message[1024];
      printf("ABCDE\n");
      make_matchmaking_list_payload_server(client, payload_to_send);
      printf("Se hace el payload para responder a match list: %s\n", payload_to_send);
      build_package(3, strlen(payload_to_send), payload_to_send, message);
      printf("Se hace el message para mandar en el package: %s\n", message);
      sendMessage(client -> socket, message);
    }
    else if (id == 4){
      //match request

      if (payload_size == 2){
        //peticion de partida de un cliente
        int id_client_requested;
        id_client_requested = binarytoInt(payload);
        int socket_cliente = (client -> server -> clients)[id_client_requested] -> socket;
        char nick_client_requesting[256];
        strcpy(nick_client_requesting, client -> nickname);
        int id_client_requesting;
        id_client_requesting = client -> unique_id;
        int largo_nick = strlen(nick_client_requesting);
        char payload_to_send[16+256*8];
        make_matchrequest_payload_server(id_client_requesting, nick_client_requesting, payload_to_send);
        int payload_size = 2 + largo_nick;
        char package[16 + 256*8];
        build_package(5, payload_size, payload_to_send, package);
        sendMessage(socket_cliente, package);


      }

    }

    else if (id == 5){
      char respuesta_bits[16];
      strcpy(respuesta_bits, payload);
      int respuesta_int = binarytoInt(respuesta_bits);
      char package[100];
      build_package(4,1,payload, package);
      int id_client_requesting = client -> unique_id;
      int socket_cliente = (client -> server -> clients)[id_client_requesting] -> socket;
      sendMessage(socket_cliente, package);
    }


  }
  return 0;
}

void* client_thread(void* client_info){
  pthread_t thread1;
  pthread_t thread2;
  Client* client = (Client*)client_info;
  //int p = client -> socket;
  //int* s = (int*)socket;

  // int heartbeat_count = 0; // Es para llevar la cuenta de cuantos hearbeat he enviado sin respuesta.

  // thread que hace heartbeat
  pthread_create(&thread1, NULL, heartbeat_server, client);

  // thread que escucha por mensajes de este cliente especifico
  pthread_create(&thread2, NULL, listener_thread, client);
  pthread_join(thread2, NULL);
  pthread_join(thread1, NULL);

  return 0;
}

/* Función que inicializa el servidor en el port
con ip */
int initializeServer(char* ip, int port){
  Server* server = server_init();
  pthread_t thread0;
  int welcomeSocket;
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

  /*Escucha nuevos clientes que se conectan*/

  while (1){
    if(listen(welcomeSocket,50)==0)
      printf("Waiting for the second user to connect...\n");
    else
      printf("Error\n");
    addr_size = sizeof serverStorage;

    int newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

    printf("Conected user in socket: %i\n", newSocket);
    Client* new_client = client_init(newSocket, server);
    add_client_to_server(server, new_client); // Aca adentro se asigna ID unico al usuario
    pthread_create(&thread0, NULL, client_thread, new_client); // Se crea nuevo thread para interactuar con nuevo cliente
  }

  pthread_join(thread0, NULL);

  return 0;

}

int main (int argc, char *argv[])
{
  printf("Server\n");
  initializeServer(IP, PORT);

}
