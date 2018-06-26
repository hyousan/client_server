#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#include "h/pipe.h"

bool Server_Start();
bool SendFile(char *line, char *destfile);

/*
 main関数
*/
int main(void){
    if( !Server_Start() ){
        printf("main Error\n");
        return 1;
    }

    return 0;
}

/*
 Server_Start関数
*/
bool Server_Start(){
    WSADATA wsaData;
    SOCKET sock0;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    char string[256];
    char buf[1024];
    int len;
    int n;

    // 送るやつ
    char line[256];
    char destfile[256];

    DWORD dwSize = 1024;
    void *Mem = (void *)new TCHAR[dwSize];
    
    SOCKET sock;
    BOOL yes = 1;

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        return 1;
    }

    sock0 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock0 == INVALID_SOCKET) {
        printf("socket : %d\n", WSAGetLastError());
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;

    setsockopt(sock0,
        SOL_SOCKET,
        SO_REUSEADDR,
        (const char *)&yes, sizeof(yes));

    if (bind(sock0, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("bind : %d\n", WSAGetLastError());
        return false;
    }

    if (listen(sock0, 5) != 0) {
        printf("listen : %d\n", WSAGetLastError());
        return false;
    }

    // pipeを作る
    //StartPipe();

    while (1) {
        len = sizeof(client);
        sock = accept(sock0, (struct sockaddr *)&client, &len);
        if (sock == INVALID_SOCKET) {
            printf("accept : %d\n", WSAGetLastError());
            break;
        }
        printf("accept connection from %s, port=%d\n",
            inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        if (n < 1) {
            printf("send : %d\n", WSAGetLastError());
            break;
        }

        // サーバからデータを受信
        memset(buf, 0, sizeof(buf));
        recv(sock, buf, sizeof(buf), 0);
        
        if( strstr(buf, "Owari") != NULL ){         // 終わり
            printf("Bye\n");
            //ClosePipe();
            ZeroMemory(Mem, dwSize);
            ZeroMemory(buf, strlen(buf));
            closesocket(sock);
            break;
        }else if( strstr(buf, "send") != NULL ){    // ファイルの内容をすべて出力
            printf("%s\n", buf);
            /*strtok(buf, "@@");
            strcpy(destfile, strtok(NULL, "@@"));
            printf("ERRORRRRRRRRRRRRRRRRR : %s\n", line);
            strcpy(line, strtok(NULL, "@@"));
            printf("ERRORRRRRRRRRRRRRRRRR : %s\n", destfile);
            */
            //SendFile(line, destfile);
        }

        printf("command : %s\n\n",buf);
        if( !Pipe(Mem, dwSize, buf) ){
            printf("pipe Error\n");
            ZeroMemory(Mem, dwSize);
            ZeroMemory(buf, strlen(buf));
            closesocket(sock);
            exit;
        }
        printf("%s", (TCHAR *)Mem);
        printf("\n--------------------------------------------\n\n");

        send(sock, (TCHAR *)Mem, strlen((TCHAR *)Mem), 0);
        
        // メモリの中身を空にする
        ZeroMemory(Mem, dwSize);
        ZeroMemory(buf, strlen(buf));

        closesocket(sock);
    }

    WSACleanup();

    return true;
}

/*
SendFile関数
*/
bool SendFile(char *line, char* destfile){
    FILE *fp;

    fp = fopen(destfile, "w");
    if( fp == NULL ){
        printf("%sファイルが開けません\n", destfile);
        return true;
    }
    fputs(line, fp);

    fclose(fp);

    return false;
}