#include <stdio.h>
#include <winsock2.h>

#define MAX_SIZE 256
#define PWSTRING "powershell -NoProfile -ExecutionPolicy Unrestricted "

bool Client_Start(char *deststr, char* string);
bool SendFile(char *sourceFile, char *dstFile, char *deststr);

/*
 main関数
*/
int main(int argc, char *argv[]){
    char string[MAX_SIZE] = {0};    // 命令文格納配列
    char temp[MAX_SIZE] = {0};      // １時作業配列
    char *deststr;                  // ipアドレス格納配列
    bool flag = true;               // フラグ

    // 入力エラー
    if( argc != 2 ){
        printf("Usage : %s IP\n", argv[0]);
        return 1;
    }
    deststr = argv[1];  // ipアドレスを格納

    printf("============================================\n");
    printf("File listen :\n more *.txt\n\n");
    printf("File execution :\n %s\n\n", strcat(PWSTRING, "./*.ps1" ));
    printf("File send :\n send sourcefile destfile\n\n");
    printf("--------------------------------------------\n\n");

    while( flag ){
        printf("INPut : ");
        fgets(string, sizeof(string), stdin);
        
        if( strstr(string, "powershell") != NULL ){         // 最初の文字がpowershellなら実行させるように変える
            strcpy(temp, PWSTRING);
            strtok(string, " ");
            strcat(strcat(temp, "./"), strtok(NULL, " "));
            strcpy(string, temp);
        }else if( strstr(string, "send") != NULL ){         // ファイルの中身を送る
            strtok(string, " ");                // send
            char *source = strtok(NULL, " ");   // sourcefile
            char *dest = strtok(NULL, " ");     // destfile
            
            SendFile(source, dest, deststr);
        }

        fflush(stdin);  // fgets内のあふれた文字を捨てる

        // 終わる
        if( !strcmp(string, "exit") || !strcmp(string, "\\q") || !strcmp(string, "Owari") ){
            break;
        }

        // Client_Start
        Client_Start(deststr, string);
    }

    return 0;
}

/*
 Clientを始める
*/
bool Client_Start(char *deststr, char* string){
    WSADATA wsaData;
    struct sockaddr_in server;
    SOCKET sock;
    char buf[1024];
    unsigned int **addrptr;
    int n;

    // winsock2の初期化
    WSAStartup(MAKEWORD(2,0), &wsaData);

    // ソケットの作成
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // 接続先指定用構造体の準備
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    server.sin_addr.S_un.S_addr = inet_addr(deststr);
    if(server.sin_addr.S_un.S_addr == 0xffffffff){
        struct hostent *host;

        host = gethostbyname(deststr);
        if (host == NULL) {
            return false;
        }

        addrptr = (unsigned int **)host->h_addr_list;

        while(*addrptr != NULL){
            server.sin_addr.S_un.S_addr = *(*addrptr);

            // connect()が成功したらloopを抜けます
            if (connect(sock,
                    (struct sockaddr *)&server,
                    sizeof(server)) == 0) {
                break;
            }

            addrptr++;
        }
        server.sin_addr.S_un.S_addr =
            *(unsigned int *)host->h_addr_list[0];
    }else{
        // inet_addr()が成功したとき
        if (connect(sock,
            (struct sockaddr *)&server,
            sizeof(server)) != 0) {
            printf("connect : %d\n", WSAGetLastError());
            return false;
        }
    }

    // サーバに接続
    connect(sock,
        (struct sockaddr *)&server,
        sizeof(server));

    n = send(sock, string, strlen(string), 0);

    if (n < 1) {
        printf("send : %d\n", WSAGetLastError());
        return false;
    }

    // サーバからデータを受信
    memset(buf, 0, sizeof(buf));
    recv(sock, buf, sizeof(buf), 0);
    printf("%s\n", buf);
    printf("============================================\n\n");
    

    // winsock2の終了処理
    WSACleanup();

    return true;
}


bool SendFile(char *sourcefile, char *dstfile, char *deststr){
    FILE *fp;
    char temp[256] = {0};
    char tem[1000] = {"send@@"};
    strcat(tem, strcat(dstfile, "@@"));

    if( (fp = fopen(sourcefile, "r")) == NULL ){
        printf("File Open Error\n");
        return true;
    }
    
    while( fgets(temp, sizeof(temp), fp) != NULL ){
        /*char *tem;
        strcpy(tem, "send ");
        strcat(tem, temp);
        strcat(tem, " ");
        strcat(tem, dstfile);*/
        //printf("%s", tem);
        //Client_Start(deststr, tem);
        strcat(tem, temp);
    }
    Client_Start(deststr, tem);

    fclose(fp);

    return false;
}