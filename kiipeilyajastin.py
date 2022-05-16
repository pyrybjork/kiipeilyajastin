import serial
import time
arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)

timer = False
start = 0

best_time = 0

def milli_time():
    return int(time.time() * 1000)

def format_millis(millis):
    minutes = '0' + str(int(millis / (1000 * 60)))
    seconds = '0' + "{:.3f}".format((millis % (1000 * 60)) / 1000, 2)
    return f'{minutes[-2:]}:{seconds[-6:]}   '

print('\n\n\n')

while True:
    if arduino.in_waiting:
        data_in = int(arduino.readline().decode())
        if data_in > 0:
            if data_in < best_time or best_time == 0:
                best_time = data_in
            timer = False
            print(f'\n\n\nParas: {format_millis(best_time)}')
            print(format_millis(data_in), end='\r')
        elif data_in == -2:
            print('00:00.00', end='\r')
        elif data_in == -1:
            timer = True
            start = milli_time()

    if timer == True:
        print(format_millis(milli_time() - start), end='\r')
