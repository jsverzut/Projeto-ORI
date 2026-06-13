#!/usr/bin/env python3

import os
import random
import sys
import hashlib


if len(sys.argv) < 2:
    print(f"Uso\n\t{sys.argv[0]} <NUMERO DE AMOSTRAS>")
    sys.exit(1)

s_number = int(sys.argv[1])
size = 10
mal_list = []

# Gera samples
for i in range(s_number):
    path = f"samples/sample{i}"
    is_mal = random.choices([1, 0], weights=[33, 67])[0]
    
    # Gera os bytes na memória primeiro
    file_data = os.urandom(10)
    
    with open(path, "wb") as file:
        file.write(file_data)
        
    if is_mal:
        # Calcula o hash diretamente dos bytes da memória
        hash_obj = hashlib.sha256(file_data)
        mal_list.append(hash_obj.hexdigest())


with open("database.txt", "w", encoding='utf-8') as db:
    for i in mal_list:
        mal_hash = i + "\n"
        db.write(mal_hash)

print("Os arquivos de teste foram gerados com sucesso")

