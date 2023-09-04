-include Makefile.inc

SHELL := /bin/bash

HAS_PBKDF2 := $(shell openssl enc -pbkdf2 > /dev/null 2>&1 < /dev/null; echo $$?)
ifeq ($(HAS_PBKDF2), 0)
PBKDF2_FLAG := -pbkdf2
endif
AES256_CMD = openssl enc -a -A $(PBKDF2_FLAG) -aes-256-cbc -md sha256

LINK = src/core/libcore.a -lssl -lpthread -lcrypto

all: application

application: core
	$(eval LASTPORT := $(shell cat .last 2>/dev/null|grep 'port='|cut -d '=' -f 2))
	$(eval LASTPORT := $(shell if [ -n "$(LASTPORT)" ]; then echo $(LASTPORT); else echo "65432"; fi))
	$(eval USEPORT := $(shell if [ -z "$(PORT)" ] && [ -z "$(DATA)" ]; then echo $$(read -p "Listen port [$(LASTPORT)]: "; if [ -n "$$REPLY" ]; then echo $$REPLY; else echo $(LASTPORT); fi); else echo $(PORT); fi))
	$(eval LASTHOST := $(shell cat .last 2>/dev/null|grep 'host='|cut -d '=' -f 2))
	$(eval LASTHOST := $(shell if [ -n "$(LASTHOST)" ]; then echo $(LASTHOST); else echo "ftp.example.com:21"; fi))
	$(eval USEHOST := $(shell if [ -z "$(HOST)" ] && [ -z "$(DATA)" ]; then echo $$(read -p "Target host [$(LASTHOST)]: "; if [ -n "$$REPLY" ]; then echo $$REPLY; else echo $(LASTHOST); fi); else echo $(HOST); fi))
	$(eval LASTIDENT := $(shell cat .last 2>/dev/null|grep 'ident='|cut -d '=' -f 2))
	$(eval LASTIDENT := $(shell if [ -z "$(LASTIDENT)" ] || [ "$(LASTIDENT)" != "false" ]; then echo "true"; else echo "false"; fi))
	$(eval USEIDENT := $(shell if [ -z "$(IDENT)" ] && [ -z "$(DATA)" ]; then echo $$(read -p "Ident lookup [$(shell if [ $(LASTIDENT) == "false" ]; then echo "n"; else echo "y"; fi)]: "; if [ -n "$$REPLY" ]; then if [ "$$REPLY" != "n" ]; then echo "true"; else echo "false"; fi; else echo "$(LASTIDENT)"; fi); else echo $(IDENT); fi))
	$(eval LASTBIND := $(shell cat .last 2>/dev/null|grep 'bind='|cut -d '=' -f 2))
	$(eval LASTBIND := $(shell if [ -z "$(LASTBIND)" ] || [ "$(LASTBIND)" != "true" ]; then echo "false"; else echo "true"; fi))
	$(eval USEBIND := $(shell if [ -z "$(BIND)" ] && [ -z "$(DATA)" ]; then echo $$(read -p "Bind to specific IP or interface [$(shell if [ $(LASTBIND) == "false" ]; then echo "n"; else echo "y"; fi)]: "; if [ -n "$$REPLY" ]; then if [ "$$REPLY" != "n" ]; then echo "true"; else echo "false"; fi; else echo "$(LASTBIND)"; fi); else echo $(BIND); fi))
	$(eval LASTIPIF := $(shell cat .last 2>/dev/null|grep 'ipif='|cut -d '=' -f 2))
	$(eval LASTIPIF := $(shell if [ -n "$(LASTIPIF)" ]; then echo $(LASTIPIF); else echo "0.0.0.0"; fi))
	$(eval USEIPIF := $(shell if [ "$(USEBIND)" == "true" ]; then if [ -z "$(IPIF)" ] && [ -z "$(DATA)" ]; then echo $$(read -p "IP or network interface [$(LASTIPIF)]: "; if [ -n "$$REPLY" ]; then echo $$REPLY; else echo $(LASTIPIF); fi); else echo $(IPIF); fi; else echo $(LASTIPIF); fi))
	$(eval ENCRYPT := $(shell if [ -z "$(DATA)" ] && ( [ -z "$(PORT)" ] || [ -z "$(HOST)" ] || [ -z "$(IDENT)" ] ); then echo $$(read -p "Encrypt data [y]: "; if [ -z "$$REPLY" ] || [ "$$REPLY" != "n" ]; then echo "1"; else echo ""; fi); fi))
	$(eval AGGDATA := port=$(USEPORT);host=$(USEHOST);ident=$(USEIDENT);bind=$(USEBIND);ipif=$(USEIPIF))
	$(eval ENCDATA := $(shell if [ -n "$(ENCRYPT)" ]; then echo -n "$(AGGDATA)" | $(AES256_CMD); fi))
	@rm -f .last
	@if [ -z "$(DATA)" ] && ( [ -z "$(ENCRYPT)" ] || [ -z "$(ENCDATA)" ] ); then echo "$(AGGDATA)" | sed s/\;/\\n/g > .last; fi
	@if [ -n "$(ENCRYPT)" ] && [ -z "$(ENCDATA)" ]; then echo -e "WARNING: Proceeding without encryption."; fi
	$(eval DATA := $(shell if [ -z "$(DATA)" ]; then echo $$(if [ -n "$(ENCDATA)" ]; then echo $(ENCDATA); else echo "$(AGGDATA)"; fi); fi))
	g++ $(ALLFLAGS) -o cbbncd -DBNCDATA="\"$(DATA)\"" $(wildcard src/*.cpp src/bnc/*.cpp) $(LINK)

core:
	@+$(MAKE) -C src/core

clean:
	@+$(MAKE) -C src/core clean
	rm -f .last cbbncd
