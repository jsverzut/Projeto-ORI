#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>

#include "btree.cpp"
#include "hash_function.cpp"

using namespace std;

// enum para representar cada campo da base de dados
enum ColunasCSV {
    FIRST_SEEN_UTC = 0,
    SHA256_HASH,    // 1
    MD5_HASH,       // 2
    SHA1_HASH,      // 3
    REPORTER,       // 4
    FILE_NAME,      // 5
    FILE_TYPE_GUESS,// 6
    MIME_TYPE,      // 7
    SIGNATURE,      // 8
    CLAMAV,         // 9
    VTPERCENT,      // 10
    IMPHASH,        // 11
    SSDEEP,         // 12
    TLSH            // 13
};

// Fun├º├úo auxiliar para remover aspas extras das strings extra├¡das do CSV
std::string limpar_aspas(std::string str) {
    if (str.empty()) return str;
    // Remove espa├ºos em branco nas pontas
    while (!str.empty() && isspace(str.front())) 
        str.erase(0, 1);
    while (!str.empty() && isspace(str.back())) 
        str.pop_back();
    // Remove aspas se existirem
    if (str.front() == '"' && str.back() == '"') {
        str = str.substr(1, str.length() - 2);
    }
    return str;
}

int main(int argc, char **argv) {
    try {
        BTree database;
        std::ifstream fileDB("database.txt");

        if (!fileDB.is_open()){
            std::cerr << "Erro ao abrir a base de dados (database.txt)\n";
            return 1;
        }
        
        if (argc < 2){
            std::cout << "Uso:\n\t" << argv[0] << " <ARQUIVO_PARA_TESTAR>\n";
            return 1;
        }

        std::string linha;
        
        // Pula o cabe├ºalho do CSV (o cabe├ºalho)

	std::getline(fileDB, linha);

        // 1. CARREGA O CSV NA B-TREE
        while (true) {
            // Guarda posi├º├úo onde a pr├│xima linha come├ºa
            std::streampos offset_atual = fileDB.tellg(); 

            if (!std::getline(fileDB, linha)) {
                break; 
            }

            if (linha.empty()) {
                continue;
            }

            std::stringstream ss(linha);
            std::string campo;
            std::string mal_hash;

            std::getline(ss, campo, ','); // Descarta a coluna 0
            if (std::getline(ss, mal_hash, ',')) {
                mal_hash = limpar_aspas(mal_hash);
                
                database.insert(mal_hash, offset_atual);
            }
        }


        // 2. CALCULA O HASH DO ARQUIVO ALVO
        std::string hash = get_file_sha256(argv[1]);

        // 3. BUSCA O HASH PURO NA B-TREE
        std::streampos offset = database.search(hash);

        // 4. VERIFICA O RESULTADO
        if (offset != std::streampos(-1)){
            // Vai direto para o byte correto da linha no banco de dados
            fileDB.clear();

            fileDB.seekg(offset);

            std::string contents;
            std::getline(fileDB, contents);
            
            std::stringstream aux(contents);
            std::vector<std::string> campos;
            std::string campo;

            // Divide a linha inteira por v├¡rgulas para preencher o vetor
            while (std::getline(aux, campo, ',')) {
                campos.push_back(limpar_aspas(campo));
            }
            
            // Exibe os dados baseando-se no enum
            std::cout << "\n[ALERTA DE SEGURAN├çA] ARQUIVO MALICIOSO DETECTADO!\n";
            std::cout << "========================================================\n";
            if (campos.size() > SIGNATURE) {
                std::cout << "-> Identifica├º├úo/Fam├¡lia: " << campos[SIGNATURE-1] << "\n";
            }
            if (campos.size() > FILE_NAME) {
                std::cout << "-> Nome do Arquivo Original: " << campos[FILE_NAME] << "\n";
            }
            if (campos.size() > MIME_TYPE) {
                std::cout << "-> Tipo de Arquivo: " << campos[MIME_TYPE] << "\n";
            }
            std::cout << "-> Hash SHA256 Detectado: " << hash << "\n";
            std::cout << "========================================================\n";
        }   
        else {
            std::cout << "\n[SEGURO] O arquivo analisado n├úo consta no banco de dados.\n";
        }

    } catch (const std::exception& erro) {
        std::cerr << "[ERRO]: " << erro.what() << std::endl;
    }
    return 0;
}
