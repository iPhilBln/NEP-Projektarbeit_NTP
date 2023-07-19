
# verwendete Bibliotheken 
import tkinter as tk
import datetime
import pytz                         # Das ist am Terminal zu Installieren : pip install pytz
from PIL import ImageTk, Image      # Das ist am Terminal zu Installieren : pip install pillow
import os
import ntplib                       # Das ist am Terminal zu  Installieren : pip install ntplib


# Funktionen für Land-Button-Click-Events

def show_kamerun_time():
    kamerun_tz = pytz.timezone('Africa/Douala')
    kamerun_time = datetime.datetime.now(kamerun_tz).strftime("%H:%M:%S")
    label0.config(text=kamerun_time)

def show_colombia_time():
    colombia_tz = pytz.timezone('America/Bogota')
    colombia_time = datetime.datetime.now(colombia_tz).strftime("%H:%M:%S")
    label2.config(text=colombia_time)

def show_ukraine_time():
    ukraine_tz = pytz.timezone('Europe/Kiev')
    ukraine_time = datetime.datetime.now(ukraine_tz).strftime("%H:%M:%S")
    label3.config(text=ukraine_time)

def show_sydney_time():
    sydney_tz = pytz.timezone('Australia/Sydney')
    sydney_time = datetime.datetime.now(sydney_tz).strftime("%H:%M:%S")
    label4.config(text=sydney_time)

def show_Shanghai_time():
    Shanghai_tz = pytz.timezone('Asia/Shanghai')
    Shanghai_time = datetime.datetime.now(Shanghai_tz).strftime("%H:%M:%S")
    label8.config(text=Shanghai_time)

def show_time_difference():
    format_str = "%H:%M:%S.%f"
    response = ntp_client.request(ntp_server, version=3)
    uhrzeit = datetime.datetime.fromtimestamp(response.tx_time)
    uhrzeit_str = uhrzeit.strftime('%H:%M:%S.%f')
    time1_obj = datetime.datetime.strptime(uhrzeit_str, format_str)
    response2 = ntp_client.request(gps_data, version=3)
    gpszeit = datetime.datetime.fromtimestamp(response2.tx_time)
    gpszeit_str = gpszeit.strftime('%H:%M:%S.%f')
    time2_obj = datetime.datetime.strptime(gpszeit_str, format_str)
    time_diff = time2_obj - time1_obj
    label5.config(text=str(time_diff))

# Variable zum Speichern des Zustands des Uhr-Buttons
uhr_aktiviert = False

# Funktion zum Aktualisieren der Uhrzeit
def uhrzeit_aktualisieren():
    aktuelle_uhrzeit = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
    label1.config(text=aktuelle_uhrzeit)
    if uhr_aktiviert:
        window.after(1000, uhrzeit_aktualisieren)

# Funktion zum Umschalten des Uhr-Status beim Klicken auf den orangenev Uhr-Button
def uhr_toggle():
    global uhr_aktiviert
    if uhr_aktiviert:
        uhr_aktiviert = False
        button1.config(text="Start")
    else:
        uhr_aktiviert = True
        button1.config(text="Stop")
        uhrzeit_aktualisieren()


# Verbinde dich mit dem NTP-Server
ntp_client = ntplib.NTPClient()

# Berliner NTP-Server
ntp_server = 'de.pool.ntp.org' # durch IP-Adresse oder den Hostnamen des NTP-Servers ersetzen

# Adrian GPS-NTP
gps_data = '192.168.137.93'

# ========>  Funktion zum Laden der Uhrzeit vom NTP-Server
def uhrzeit_laden():
    try:
        response = ntp_client.request(ntp_server, version=3)
        uhrzeit = datetime.datetime.fromtimestamp(response.tx_time)
        uhrzeit_str = uhrzeit.strftime('%H:%M:%S.%f')[:-2]
        label9.config(text=uhrzeit_str)
    except Exception as e:
        print("Fehler beim Laden der Uhrzeit:", str(e))

# ========>  Funktion zum Laden der GPSzeit vom Adrians Meinberg
def gpszeit_laden():
    try:
        response = ntp_client.request(gps_data, version=3)
        gpszeit = datetime.datetime.fromtimestamp(response.tx_time)
        gpszeit_str = gpszeit.strftime('%H:%M:%S.%f')[:-2]
        labelA.config(text=gpszeit_str)
    except Exception as e:
        print("Fehler beim Laden der Uhrzeit:", str(e))

def datum_anzeigen():
    aktuelles_datum = datetime.datetime.now().strftime('%d.%m.%Y')
    kalenderwoche = datetime.datetime.now().strftime('%V')
    anzeige_text = f"Datum:  {aktuelles_datum}\n KW:  {kalenderwoche}"
    labelB.config(text=anzeige_text)

def logo_laden():
    
    labelE = tk.Label(window, text="Berliner Hochschule für Technik", justify="center", font=("Calibri", 24) )
    labelE.pack()
    labelE.place(x=80, y=530)


# Erstelle das Hauptfenster
window = tk.Tk()
window.geometry("600x700")
window.minsize(width=600, height= 680 )
window.title("Gruppe 5: NTP Projekt")
window.configure(bg="white")

# Erstelle die Buttons
exit_button = tk.Button(window, text="EXIT", width=10, height=1, bg="red",font=("Arial", 13), command=window.quit)
exit_button.place(x=455, y=450)

button0 = tk.Button(window, text="Kamerun Zeit", width=15, height=1, bg="yellow", command=show_kamerun_time)
button0.place(x=400, y=100)

button1 = tk.Button(window, text="Aktuelle Zeit", width=15, height=1, bg="orange",font=("Arial", 14),  command=uhr_toggle)
button1.place(x=100, y=220)

button2 = tk.Button(window, text="Kolumbien Zeit", width=15, height=1, bg="yellow", command=show_colombia_time)
button2.place(x=400, y=160)

button3 = tk.Button(window, text="Ukraine Zeit", width=15, height=1, bg="yellow", command=show_ukraine_time)
button3.place(x=400, y=220)

button4 = tk.Button(window, text="Sydney Zeit", width=15, height=1, bg="yellow", command=show_sydney_time)
button4.place(x=400, y=280)

button6 = tk.Button(window, text="Shanghai Zeit", width=15, height=1, bg="yellow", command=show_Shanghai_time)
button6.place(x=400, y=340)

button5 = tk.Button(window, text="Differenz", width=10, height=1, bg="Pink", command=show_time_difference)
button5.place(x=25, y=440)

button7 = tk.Button(window, text="NTP-Zeit", width=10, height=1, bg="#64FF64", command=uhrzeit_laden)
button7.place(x=25, y=410)

button8 = tk.Button(window, text="GPS-Zeit", width=10, height=1, bg="#64FFFF", command=gpszeit_laden)
button8.place(x=25, y=380)

# Erstelle die Anzeigeflächen / Labels

labeld = tk.Label(window, text=" ",bg="#8484FF")
labeld.pack(side="top", fill="x")

label0 = tk.Label(window, text="", width=15, height=1)
label0.place(x=400, y=80)

label1 = tk.Label(window, text="Downloading time",font=("Arial", 22), fg="green", width=20, height=4)
label1.place(x=20, y=80)

label2 = tk.Label(window, text="", width=15, height=1)
label2.place(x=400, y=140)

label3 = tk.Label(window, text="", width=15, height=1)
label3.place(x=400, y=200)

label4 = tk.Label(window, text="", width=15, height=1)
label4.place(x=400, y=260)

label8 = tk.Label(window, text="", width=15, height=1)
label8.place(x=400, y=320)

label5 = tk.Label(window, text="...", width=30, height=1)
label5.place(x=110, y=445)

# Erstelle ein Label für die Anzeige des Datums und der Kalenderwoche
labelB = tk.Label(window, text="Datum & Kalenderwoche",font=("Arial", 16), fg="#5400A8")
labelB.pack()
labelB.place(x=80, y=300)


# Erstelle ein Label für die Uhrzeit
label9 = tk.Label(window, text="Uhrzeit wird geladen...",width=30, height=1)
label9.pack()
label9.place(x=110, y=415)

# Erstelle ein Label für die GPSzeit
labelA = tk.Label(window, text="GPSzeit wird geladen...",width=30, height=1)
labelA.pack()
labelA.place(x=110, y=385)

# Aktualisiere die Uhrzeit regelmäßig (z.B. alle 1 Sekunde)

window.after(3000, logo_laden)
window.after(5000, gpszeit_laden)
window.after(5000, uhrzeit_laden)
window.after(10000, datum_anzeigen)


# Erstelle das Label mit blauem Text
label7 = tk.Label(window, text="Ein NEP-Projekt von:  Adrian - Diego  -  Kaja  -  Phillip ",  width=60, fg="blue", justify="center", font=("Arial", 14) )
label7.pack()
label7.place(x=10, y=650)

# Starte die Hauptloop des Fensters
window.mainloop()


