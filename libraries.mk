
#lib Openssl
OPENSSL_LIB_VERSION		=	1.1
ifeq ($(OS_DETECTED),LINUX)
	OPENSSL_LIB_PATH	=	/usr/
else ifeq ($(shell uname),Darwin)
	OPENSSL_LIB_PATH		=	$(shell brew --prefix openssl@$(OPENSSL_LIB_VERSION))
endif
CPP_LNK_FLAGS		+=	-lssl -lcrypto -L $(OPENSSL_LIB_PATH)/lib/
CPP_INC_FLAGS		+=	-I $(OPENSSL_LIB_PATH)/include
