import keyboard #type: ignore
import serial #type: ignore
import csv
from datetime import datetime
from pydrive.auth import GoogleAuth #type: ignore
from pydrive.drive import GoogleDrive #type: ignore
import os


g_login = GoogleAuth()
drive = GoogleDrive(g_login)

Temperatura = []
Umidade = []
X = []
Y = []
Z = []
Gx = []
Gy = []
Gz = []
Hora = []

while True:
    try:
        arduino = serial.Serial('COM7', 115200)
        print('Arduino conectado')
        inicio = datetime.now().strftime("%H-%M-%S")
        break
    except:
        pass

def send_data_to_arduino(data):
    arduino.write(data.encode('utf-8'))

def read_data_from_arduino():
    data = arduino.readline().decode('utf-8').strip()
    print(data)
    if data.split(';')[0] == 'TEMPUMID':
        Temperatura.append(data.split(';')[2])
        Umidade.append(data.split(';')[1])
    if data.split(';')[0] == 'ACELGIROS':
        X.append(data.split(';')[1])
        Y.append(data.split(';')[2])
        Z.append(data.split(';')[3])
        Gx.append(data.split(';')[4])
        Gy.append(data.split(';')[5])
        Gz.append(data.split(';')[6])
        t = datetime.now().strftime("%H:%M:%S")
        Hora.append(t)
        

def on_key_event(e):
    if e.event_type == keyboard.KEY_DOWN:
        key = e.name
        send_data_to_arduino(key)
        if key == 'p':
            final()
            arduino.close()
    if e.event_type == keyboard.KEY_UP:
        send_data_to_arduino('z')

keyboard.hook(on_key_event)

def final():
    a = len(X) % 6
    b= 1
    while b < a:
        X.pop
        Y.pop
        Z.pop
        Gx.pop
        Gy.pop
        Gz.pop
        Hora.pop
        b+=1
    f = open('./DadosRobo_'+inicio+'.csv', 'w', newline = "")
    w = csv.writer(f, delimiter=';')
    w.writerow(['Hora','Temperatura', 'Umidade', 'X', 'Y', 'Z', 'Gx', 'Gy', 'Gz'])
    for x in range(len(X)):
        c = 0
        if x%6 == 0:
            w.writerow([Hora[x], Temperatura[c], Umidade[c], X[x], Y[x], Z[x], Gx[x], Gy[x], Gz[x]])
            c+=1
        else:
            w.writerow([Hora[x], '', '', X[x], Y[x], Z[x], Gx[x], Gy[x], Gz[x]])
    f.close()
    file = open('./DadosRobo_'+inicio+'.csv')
    fn = os.path.basename(file.name)
    file_drive = drive.CreateFile({'title': fn })
    file_drive.SetContentString(file.read())
    file_drive.Upload()


    permission = file_drive.InsertPermission({'type': 'anyone','value': 'anyone','role': 'reader'})

try:
    while True:
        read_data_from_arduino()
except KeyboardInterrupt:
    pass
    
