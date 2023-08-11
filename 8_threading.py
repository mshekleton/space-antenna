from threading import Thread
import time

global cycle
cycle = 0.0

class Hello5Program:
    def __init__(self):
        self._running = True

    def terminate(self):
        self._running = False
    
    def run(self):
        global cycle
        while self._running:
            time.sleep(5)
            cycle = cycle + 1.0
            print("5 second thread cycle+1.0 - ", cycle)

FiveSecond = Hello5Program()
FiveSecondThread = Thread(target=FiveSecond.run)
FiveSecondThread.start()

Exit = False
while Exit==False:
    cycle = cycle + 0.1
    print("Main Program increases cycle+0.1 - ", cycle)
    time.sleep(1)
    if (cycle > 5): Exit = True

FiveSecond.terminate()
print("Goodbye")