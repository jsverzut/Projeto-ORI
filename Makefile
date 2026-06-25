all:
	g++ -std=c++17 -I ./aux_func src/hash_compare.cpp -o hash_compare -lcrypto -lssl
debug:
	g++ -g -std=c++17 -I ./aux_func src/hash_compare.cpp -o hash_compare -lcrypto -lssl
clean:
	rm -rf samples/*
# Windows:
# all:
#	g++ -g -std=c++17 -I ./aux src/hash_compare.cpp -o hash_compare.exe -lcrypto -lssl
# clean:
#	rm -rf samples/*
