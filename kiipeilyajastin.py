import serial
import time
arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1) #/dev/ttyAMC0

timer = False
start = 0

#nykyinen aika millisekunteina. Riippuu tietokoneesta kuinka tarkka tulee, mutta pitäisi olla tarkempi kuin sekunnit.
#time.time() palauttaa siis sekunteja desimaalilukuna
def milli_time(): 
    return int(time.time() * 1000)

#millisekuntiti muotoon 00:00.000(minuutit:sekuntit)
def format_millis(millis):
    minutes = '0' + str(int(millis / (1000 * 60)))
    seconds = '0' + "{:.3f}".format((millis % (1000 * 60)) / 1000, 2)
    return f'{minutes[-2:]}:{seconds[-6:]}   '

print('\n\n\n')

while True:
    #odotetaan, että seuraava kokonainen rivi on valmis
    if arduino.in_waiting:
        
        data_in = int(arduino.readline().decode())
        
        if data_in > 0: #jos visti on yli nolla niin se on lopullinen aika; lopetetaan ajastin ja tulostetaan vastaanotettu aika
            timer = False
            print(f'\n\n\nParas: {format_millis(best_time)}')
            print(format_millis(data_in), end='\r')
            
        elif data_in == -2: #nollaus
            print('00:00.00', end='\r')
            
        elif data_in == -1: #paikallinen ajanotto käyntiin
            timer = True
            start = milli_time()

    if timer == True:
        print(format_millis(milli_time() - start), end='\r')
