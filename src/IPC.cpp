#include<IPC.h>

int createConnection()
{
    HANDLE hPipe = CreateNamedPipeW(L"\\\\.\\pipe\\dataPipe",
        PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 
        1, 1024, 1024, 0, NULL);

    if(hPipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Error making pipe: " << GetLastError();
        return -1;
    }
     
    BOOL fConnected = ConnectNamedPipe(hPipe, NULL);
    if(!fConnected) CloseHandle(hPipe);

    
}