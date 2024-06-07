#Importerer bibliotek
#KILDE: SQLite. (23/5/2024) What is SQLite? (Versjon 3.46.0)[Programvare] https://sqlite.org/ Hentet (2/6/2024)
#Eclipse. (29/4/2024) Paho.mqtt.python (Versjon 2.1.0 ) [Programvare] https://github.com/eclipse/paho.mqtt.python Hentet (18/5/2024)

import sqlite3
import paho.mqtt.client as mqtt

# Konfigurerer innloggingsinformasjon og broker adresse for MQTT
broker_address = "10.25.18.138"
port = 1883
username = "njaal"
password = "3Inshallah4"

# Global variabel for å lagre motatt beskjed fra MQTT
global_payload = None

# Callback for når klienten kobler seg til broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Tilkobling var vellykket {rc}")
        # Abonnerer på topic "esp32/output"
        client.subscribe("esp32/output")
    else:
        print(f"Mislykket tilkobling {rc}")

# Callback for når en melding er motatt fra brokeren
def on_message(client, userdata, msg):
    global global_payload
    # Dekoder meldingen og bruker split funksjonen for å gjøre meldingen om til en liste
    payload = msg.payload.decode('utf-8')
    print(f"Motatt melding: '{payload}' på topic: '{msg.topic}'")
    global_payload = payload.split(',')
    global_payload = payload.split(',')
    print(global_payload)
    # Caller på check_values funksjonen avhengig av den nye meldingen som har blitt motatt
    check_values(global_payload)

# Callback for når beskjed publiseres
def on_publish(client, userdata, mid):
    print(f'Følgende melding ble publisert:{mid}')

# Lager en instans av mqtt.Client
client = mqtt.Client("Python_faktura")

# Setter passord og brukernavn
client.username_pw_set(username, password)

# Tildeler de ulike callback-funksjonene
client.on_connect = on_connect
client.on_message = on_message
client.on_publish = on_publish

# Kobler seg til broker
client.connect(broker_address, port, 60)

# Dette er hovedfunksjonen til programmet, calles kun når en melding har blitt motatt
def check_values(payload):
    # Kobler seg på databasen
    conn = sqlite3.connect('smartcity.db')
    cursor = conn.cursor()
    
    # Query som henter alle radene i tabellen:
    cursor.execute('SELECT * FROM bilskilt2')
    match_found = False
    #Legger til payload i trafikk.txt fil slik at denne informasjonen kan anvendes av andre program
    fil_plassering = '/var/www/html/trafikk.txt'
    with open(fil_plassering, 'w') as file:
        file.write(f"{payload[3]},{payload[3]}\n")
    # Itererer gjennom alle radene i den første tabellen for å prøve å finne en match
    for row in cursor:
        id, num1, num2, num3, classification = row
        if num1 == payload[0] and num2 == payload[1] and num3 == payload[2]:
            print(f'Bilen er av type: {classification}')
            match_found = True
            # Hvis det finnes en match i tabellen henter programmet alle radene i den neste tabellen
            if match_found == True:
                cursor.execute('SELECT * FROM bompenger')
                # Itererer gjennom radene i den neste tabellen
                for row in cursor:
                    id, pris, prisklasse = row
                    # Dersom det finnes en match i denne tabellen brukes denne
                    # informasjonen til å  regne ut et fakturabeløp
                    if prisklasse == classification:
                        fakturerings_belop = pris * float(payload[3]) + 20
                        print(f'Totalt faktureringsbeløp på: {fakturerings_belop}kr')
                        # Faktureringsbeløpet sendes til topicen pytophp
                        try:
                            msg = str(fakturerings_belop)
                            pubMsg = client.publish(
                            topic = 'pytophp',
                            payload = msg.encode('utf-8'),
                            qos = 0,
                            )
                            if pubMsg.is_published():
                                print("Velykket fakturering:")
                        finally:
                            print(f'Faktureringsbeløp sendt til eier av {classification}')
            break

    if not match_found:
        print('Ingen match i database')

    conn.close()

# Setter opp tabell dersom den ikke allerede eksisterer
if __name__ == "__main__":
    conn = sqlite3.connect('smartcity.db')
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS bilskilt (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            num1 TEXT,
            num2 TEXT,
            num3 TEXT,
            classification TEXT
        )
    ''')

    # Verdier som kan legges til i tabellen
    # cursor.execute('INSERT INTO bilskilt (num1, num2, num3, classification) VALUES (?, ?, ?, ?)', ('1', '0', '0', 'A'))
    # cursor.execute('INSERT INTO bilskilt (num1, num2, num3, classification) VALUES (?, ?, ?, ?)', ('4', '5', '6', 'B'))
    # cursor.execute('INSERT INTO bilskilt (num1, num2, num3, classification) VALUES (?, ?, ?, ?)', ('7', '8', '9', 'C'))
    # cursor.execute('INSERT INTO bilskilt (num1, num2, num3, classification) VALUES (?, ?, ?, ?)', ('3', '8', '9', 'D'))
    conn.commit()
    conn.close()

# Starter MQTT loopen for å kunne prosessere callback-funksjonene
client.loop_forever()
