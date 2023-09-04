SHELL := /bin/bash

AES256_CMD = openssl enc -a -A -aes-256-cbc -md sha256

LINK = src/core/libcore.a -lssl -lpthread -lcrypto

all: application

application: core
	$(eval LASTPORT := $(shell cat .last 2>/dev/null|sed 's/:/ /'|awk '{print $$1}'))
	$(eval LASTPORT := $(shell if [ -n "$(LASTPORT)" ]; then echo $(LASTPORT); else echo "65432"; fi))
	$(eval PORT := $(shell read -p "Listen port [$(LASTPORT)]: "; if [ -n "$$REPLY" ]; then echo $$REPLY; else echo $(LASTPORT); fi))
	$(eval LASTHOST := $(shell cat .last 2>/dev/null|sed 's/:/ /'|awk '{print $$2}'))
	$(eval LASTHOST := $(shell if [ -n "$(LASTHOST)" ]; then echo $(LASTHOST); else echo "ftp.example.com:21"; fi))
	$(eval HOST := $(shell read -p "Target host [$(LASTHOST)]: "; if [ -n "$$REPLY" ]; then echo $$REPLY; else echo $(LASTHOST); fi))
	$(eval ENCRYPT := $(shell read -p "Encrypt data [y]: "; if [ -z "$$REPLY" ] || [ "$$REPLY" != "n" ]; then echo "1"; else echo ""; fi))
	$(eval DATA := $(PORT):$(HOST))
	$(eval ENCDATA := $(shell if [ -n "$(ENCRYPT)" ]; then echo $(DATA) | $(AES256_CMD); fi))
	@rm -f .last
	@if [ -z "$(ENCRYPT)" ] || [ -z "$(ENCDATA)" ]; then echo "$(DATA)" > .last; fi
	@if [ -n "$(ENCRYPT)" ] && [ -z "$(ENCDATA)" ]; then echo -e "WARNING: Continuing without encryption."; fi
	$(eval DATA := $(shell if [ -n "$(ENCDATA)" ]; then echo $(ENCDATA); else echo $(DATA); fi))
	g++ -g -o cbbncd -DDATA="\"$(DATA)\"" $(wildcard src/*.cpp src/bnc/*.cpp) $(LINK)

core:
	@+$(MAKE) -C src/core

clean:
	@+$(MAKE) -C src/core clean
	rm -f .last cbbncd
