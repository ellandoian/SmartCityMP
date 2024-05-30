
#kilde: https://medium.com/@azkardm/dijkstras-algorithm-in-python-finding-the-shortest-path-bcb3bcd4a4ea

bom_verdi =  [2, 1, 4, 5, 2, 4, 8, 9, 2, 5, 3, 3, 5, 7]
punkter = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H']
ekstremalpunkter = ['G', 'H']

start = 'G'
stopp = 'H'

kart1 = []

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

dirKart.append(intKart[1] - intKart[2])
if(intKart[3] == 1 and intKart[2] != 1):
    dirKart.append(3)
    if(intKart[4] == 3):
        dirKart.append(2)
        dirKart.append(2)
    else:
        dirKart.append(3)
        dirKart.append(1)
        dirKart.append(1)
        dirKart.append(3)

else:
    dirKart.append(intKart[3])
    if(intKart[3] == 2):
        dirKart.append(1)
        dirKart.append(3)
    else:
        dirKart.append(3)
        dirKart.append(2)
        
print(dirKart)
print(dijkstra(punkter, linjer, start, stopp)[1])






