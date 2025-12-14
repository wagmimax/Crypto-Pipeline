#include<IPC.h>
#include<sstream>

int NamedPipe::createConnection()
{
    hPipe = CreateNamedPipeW(L"\\\\.\\pipe\\dataPipe",
        PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 
        1, 1024, 1024, 0, NULL);

    if(hPipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Error making pipe: " << GetLastError();
        return -1;
    }

    //start up other process, which will be client
    std::system("start python ../../../src/ControlPanel.py");
    Connected = ConnectNamedPipe(hPipe, NULL);
    

    if(!Connected) 
    {
        CloseHandle(hPipe);
        std::cout << "Pipe connection failed\n";
        return -1;
    }
    else
    {
        std::cout << "Pipe connected\n";
    }
    
    return 1;
}

void NamedPipe::sendData()
{
    CandleData currentCandle;
    std::stringstream message;
    DWORD bytesWritten;

    int success = createConnection();
    while(true)
    {
        currentCandle = candleData.popValue();
        message << currentCandle.ticker << " " 
                << currentCandle.open << " "
                << currentCandle.high << " " 
                << currentCandle.low << " "
                << currentCandle.close << " "
                << currentCandle.volume << " "
                << currentCandle.timestamp << " "
                << currentCandle.minute << "\n";
        std::string messageOut = message.str();

        WriteFile(hPipe, messageOut.c_str(), messageOut.size(), &bytesWritten, NULL); 

        message.str("");
        message.clear();
    }
    
}