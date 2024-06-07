#FERDIG VERSJON:

#IMPORTS
import os
import time
import random
# KILDE: Redhia, Azka. (21/9/2023) Dijkstra's Algorithm in Python: Finding the shortes path
# https://medium.com/@azkardm/dijkstras-algorithm-in-python-finding-the-shortest-path-bcb3bcd4a4ea Hentet (25/5/2024)

# KILDE: Python Software Foundation. (2023). Python: The os module (Versjon 3.10) [Programvare]. 
# https://www.python.org/ Hentet (2/6/2024)
# KILDE: Python Software Foundation. (2023). Python: The random module (Versjon 3.10) [Programvare]. 
# https://www.python.org/ Hentet (2/6/2024)

import paho.mqtt.client as mqtt

# Inloggingsdetaljer og konfigurasjon
broker_address = "10.25.18.138"
port = 1883
username = "njaal"
password = "3Inshallah4"

# Callback for tilkobling
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Vellykket tilkobling {rc}")
        #client.subscribe("esp32/output")
    else:
        print(f"Mislykket tilkobling {rc}")

# Callback for når beskjed publiseres
def on_publish(client, userdata, mid):
    print("Rute sendt til bil")

# Callback for når en melding blir mottatt fra brokeren
def on_message(client, userdata, msg):
    global global_payload
    # Dekoder meldingen og printer den
    payload = msg.payload.decode('utf-8')
    print(f"Mottok melding: '{payload}' på topic: '{msg.topic}'")

# Lager en instans av mqtt.Client
client = mqtt.Client("Dijkstras")

# Setter passord og brukernavn
client.username_pw_set(username, password)
#Bruker randints for å simulere trafikk:
Val1 = random.randint(1,10)
Val2 = random.randint(3,7)
Val3 = random.randint(1,4)
#Henter den faktiske trafikkdataen fra bomstasjonen:
with open('/var/www/html/trafikk.txt', 'r') as file:
    # Les fil og split
    innhold = file.read()
    splitcontents = innhold.split(',')
#print(f'Trafikk1 {innhold[0]}')
#print(f'Trafikk2 {innhold[2]}')

# Listen med trafikkdata
bom_verdi =  [Val1, 1, Val2, 5, Val3, int(innhold[0]), Val3, 9, int(innhold[2]), Val2, 3, Val1, 3, 7]
# Nodene i kartet
punkter = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H']
ekstremalpunkter = ['G', 'H']

with open('/var/www/html/data.txt', 'r') as file:
    # Les fil og split
    contents = file.read()
    splitcontents = contents.split(',')
    print(f'startpunkt {contents[0]}')
    print(f'sluttpunkt {contents[2]}')

start = contents[0]
stopp = contents[2]

linjer = [
    ['A', 'B', bom_verdi[0]],
    ['A', 'C', bom_verdi[1]],
    ['B', 'A', bom_verdi[2]],
    ['B', 'D', bom_verdi[3]],
    ['B', 'G', 0],
    ['C', 'A', bom_verdi[4]],
    ['C', 'D', bom_verdi[5]],
    ['C', 'E', bom_verdi[6]],
    ['D', 'B', bom_verdi[7]],
    ['D', 'C', bom_verdi[8]],
    ['D', 'F', bom_verdi[9]],
    ['E', 'C', bom_verdi[10]],
    ['E', 'F', bom_verdi[11]],
    ['E', 'H', 0],
    ['F', 'D', bom_verdi[12]],
    ['F', 'E', bom_verdi[13]],
    ['G', 'B', 0],
    ['H', 'E', 0]
]

tabell = [
    ['A', '', float('inf')],
    ['B', '', float('inf')],
    ['C', '', float('inf')],
    ['D', '', float('inf')],
    ['E', '', float('inf')],
    ['F', '', float('inf')],
    ['G', '', float('inf')],
    ['H', '', float('inf')]
]

def dijkstra(punkter, linjer, start, stopp):
    ikke_utforsket = punkter
    utforsket = []
    kart = []
    dist1 = 0
    remlist = []
    kart1 = []
    for e in range(len(ekstremalpunkter)):
        if ekstremalpunkter[e] != stopp:
            remlist.append(ekstremalpunkter[e])
    ikke_utforsket = [item for item in ikke_utforsket if item not in remlist]
    
    for punkt in tabell:
        if punkt[0] == start:
            punkt[2] = 0

    while len(utforsket) <= len(tabell):
        for vei in linjer:
            if vei[0] == start:
                if vei[1] in ikke_utforsket:
                    for punkt in tabell:
                        if punkt[0] == vei[1] and vei[2] + dist1 < punkt[2]:
                            punkt[1] = vei[0]
                            for p in tabell:
                                if p[0] == vei[0]:
                                    dist = p[2]
                            punkt[2] = vei[2] + dist
                            dist1 = punkt[2]

        utforsket.append(start)
        
        for punk in range(len(ikke_utforsket) - 1):
            if ikke_utforsket[punk] == start:
                del ikke_utforsket[punk]

        lengde = float('inf')
        for punkt in tabell:
            if punkt[0] in ikke_utforsket:
                if lengde > punkt[2]:
                    lengde = punkt[2]
                    start = punkt[0]

    i = True
    while stopp != '':
        for punkt in tabell:
            if punkt[0] == stopp:
                if i == True:
                    total_lengde = punkt[2]
                    i = False
                kart.append(punkt[0])
                stopp = punkt[1]

    kart1 = kart[::-1]
    return kart1, total_lengde

def f():
    intKart = []
    dirKart = []

    for i in dijkstra(punkter, linjer, start, stopp)[0]:
        if(i == 'A' or i == 'F'):
            intKart.append(2)
        if(i == 'B' or i == 'E'):
            intKart.append(3)
        if(i == 'C' or i == 'D'):
            intKart.append(1)
        if(i == 'G' or i == 'H'):
            intKart.append(4)
        
    for i in range(len(intKart)-2):
        if(intKart[i] == 4):
            if(intKart[i+2] == 1):
                dirKart.append(2)
            else:
                dirKart.append(3)
        elif(intKart[i] == 3):
            if(intKart[i+1] == 1):
                if(intKart[i+2] == 1):
                    dirKart.append(3)
                else:
                    dirKart.append(2)
            else:
                dirKart.append(1)
        elif(intKart[i] == 2):
            if(intKart[i+1] == 1):
                if(intKart[i+2] == 1):
                    dirKart.append(1)
                else:
                    dirKart.append(2)
            else:
                if(intKart[i+2] == 4):
                    dirKart.append(1)
                else:
                    dirKart.append(3)
        else:
            if(intKart[i+1] == 1):
                if(intKart[i+2] == 2):
                    dirKart.append(3)          
                else:
                    dirKart.append(1)
            elif(intKart[i+1] == 2):
                dirKart.append(3)
            else:
                if(intKart[i+2] == 2):
                    dirKart.append(1)
                else:
                    dirKart.append(2)
    if(stopp == 'G'):
        dirKart.append(1)
        dirKart.append(5)

    if(stopp == 'H'):
        dirKart.append(6)

    return dirKart

#MQTT FUNKSJONER HER

# Alle callback for ulike situasjoner:
client.on_connect = on_connect
client.on_message = on_message
client.on_publish = on_publish
# Kobler seg til mqtt brokeren
client.connect(broker_address, port, 60)


#Er txt filen oppdatert?
def file_updated(file_path, initial_mod_time):
    current_mod_time = os.path.getmtime(file_path)
    return current_mod_time != initial_mod_time


#Sjekk txt filen om noe nytt er lagt til:
file_path = '/var/www/html/data.txt'
initial_mod_time = os.path.getmtime(file_path)

#All kode som skal loope på kjøre konstant inne i while løkka
while True:
    #mqtt loop
    #client.loop_forever()
    # Skjekker om filen har blitt oppdatert
    if file_updated(file_path, initial_mod_time):
        #client.loop_forever()
        initial_mod_time = os.path.getmtime(file_path)
        #print(f'Instruksjoner: {f()}')
        #publish til topic web2zumo
        print("Filen har blitt oppdatert")
        # Åpner filen og leser innholdet
        with open('/var/www/html/data.txt', 'r') as file:
            # Les fil og split
            contents = file.read()
            # henter innholdet i form av en liste, splitter den, første indeks blir start, andre blir stopp
            splitcontents = contents.split(',')
            print(f'Nytt startpunkt {contents[0]}')
            print(f'Nytt sluttpunkt {contents[2]}')


        start = contents[0]
        stopp = contents[2]
        try:
            # Omformaterer listen som f() returnerer:
            msg_str = str(f())
            msg = ""
            for i in str(f()):
                if i == "[" or i == " " or i == "]" or i == ",":
                    msg = msg
                else:
                    msg += str(i)
                print(msg)
            # Publiserer meldingen på web2Zumo topicen
            pubMsg = client.publish(
            topic = 'web2Zumo',
            payload = msg.encode('utf-8'),
            qos = 0,
            )
            #pubMsg.wait_for_publish
            #if pubMsg.is_published():
            #    print("Instruksjoner er sendt")
        finally:
             print(f'LEVERT')
             # Tømmer msg variabelen
             msg = None



client.loop_forever()
