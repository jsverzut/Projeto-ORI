#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>

std::string get_file_sha256(const std::string& filepath) {
    // Abre o arquivo em modo binário e posiciona o cursor no final
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Não foi possível abrir o arquivo: " + filepath);
    }

    // Cria o contexto de hashing do OpenSSL
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr) {
        throw std::runtime_error("Falha ao criar o contexto EVP.");
    }

    // Inicializa o contexto para usar o algoritmo SHA-256
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Falha ao inicializar o digest SHA-256.");
    }

    // Determina o tamanho do arquivo e retorna o cursor para o início
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Buffer para leitura em blocos (64 KB, igual ao seu script Python)
    std::vector<char> buffer(65536); 

    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        if (EVP_DigestUpdate(mdctx, buffer.data(), file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Falha ao atualizar o hash com os dados.");
        }
    }

    // Array para armazenar o resultado final do hash (SHA-256 possui 32 bytes)
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Falha ao finalizar o digest.");
    }

    // Limpa o contexto da memória
    EVP_MD_CTX_free(mdctx);

    // Converte os bytes do hash para uma string hexadecimal
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

int main(int argc, char **argv) {
    try {
	std::ifstream db("database.txt");

	if (!db.is_open()){
		std::cerr << "Erro ao abrir a base de dados\n";
		return 1;
	}
	
	if (argc < 2){
		std::cout << "Uso\n\t" << argv[0] << " <ARQUIVO>\n";
		return 1;
	}

        std::string hash = get_file_sha256(argv[1]);
	std::string mal_hash;
	while(std::getline(db, mal_hash)){
		if (mal_hash == hash){
			std::cout << "ALERTA: O arquivo foi encontrado na base de dados (POSSÍVEL VÍRUS)\n";
			return 0;
		}
	}
	std::cout << "O arquivo nao foi encontrado na base de dados e possivelmente não é malicioso\n";
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
    }
    return 0;
}

