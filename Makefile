all:
	g++ -std=c++17 src/hash_compare.cpp -o hash_compare -lcrypto -lssl
clean:
	rm -rf samples/*
