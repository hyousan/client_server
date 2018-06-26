#include <windows.h>
#include <stdio.h>
#include <string.h>

//#include "h/pipe.h"

void StartPipe();
void ClosePipe();
bool Pipe(void *lpMem, DWORD dwSize, char *string);

/* グローバル変数（どうにかなくす） */
HANDLE fd_read;
HANDLE fd_write;
HANDLE fd_err;

bool Pipe(void *lpMem, DWORD dwSize, char *string){

    // cmdへ文字列書き込み
    DWORD dwLen;

    WriteFile(fd_read, strcat(string, "\x0D\x0A"), strlen(string) + 2, &dwLen, NULL);
    WriteFile(fd_read, "exit\x0D\x0A", 6, &dwLen, NULL);
    FlushFileBuffers(fd_write);
    FlushFileBuffers(fd_read);

    // プロセス終了まで待つ
    //WaitForSingleObject(pi.hProcess, INFINITE);

    ZeroMemory(lpMem, dwSize);
    ZeroMemory(string, strlen(string));
    ReadFile(fd_write, (TCHAR *)lpMem, dwSize - 1, &dwLen, NULL);

    return true;
}


void StartPipe(){
    HANDLE pfd_in[2], pfd_out[2], pfd_err[2];
    short int R = 0, W = 1;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;

    HANDLE hParent = GetCurrentProcess();

    //SECURITY_ATTRIBUTES sa;
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(sa);

    // 標準出力
    CreatePipe(&pfd_out[R], &pfd_out[W], &sa, 0);
    DuplicateHandle(hParent, pfd_out[R], hParent,
        &fd_write, 0, FALSE, DUPLICATE_SAME_ACCESS);
    CloseHandle(pfd_out[R]);

    // 標準エラー出力
    CreatePipe(&pfd_err[R], &pfd_err[W], &sa, 0);
    DuplicateHandle(hParent, pfd_err[R], hParent,
        &fd_err, 0, FALSE, DUPLICATE_SAME_ACCESS);
    CloseHandle(pfd_err[R]);

    // 標準入力
    CreatePipe(&pfd_in[R], &pfd_in[W], &sa, 0);
    DuplicateHandle(hParent, pfd_in[W], hParent,
        &fd_read, 0, FALSE, DUPLICATE_SAME_ACCESS);
    CloseHandle(pfd_in[W]);

    // cmdプロセス作成
    ZeroMemory(&si, sizeof(si));
    si.cb           = sizeof(si);
    si.dwFlags      = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow  = SW_HIDE;
    si.hStdInput    = pfd_in[R];    // 標準入力のパイプ
    si.hStdOutput   = pfd_out[W];   // 標準出力のパイプ
    si.hStdError    = pfd_err[W];   // 標準エラー出力のパイプ

    if(CreateProcess(NULL,/*"powershell.exe"*/"cmd.exe",NULL, NULL, TRUE,
        0, NULL, NULL, &si, &pi) != TRUE){
        //ClosePipe(pi);
        CloseHandle(pi.hProcess);
        CloseHandle(fd_read);
        CloseHandle(fd_write);
        CloseHandle(fd_err);
    }
}

void ClosePipe(){
    CloseHandle(pi.hProcess);
    CloseHandle(fd_read);
    CloseHandle(fd_write);
    CloseHandle(fd_err);
}
