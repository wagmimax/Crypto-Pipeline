import IPC

IPC.initThreads()
while True:
    

    userInput = input("Select a valid ticker to display: ")

    if userInput not in IPC.candles:
        continue

    IPC.changeChart(userInput)