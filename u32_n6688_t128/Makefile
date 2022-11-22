

CC      = gcc
CFLAGS  = -O3 -Wall -Wextra -fno-omit-frame-pointer #-g -pg -fsanitize=address
INCLUDE = -I./src -I./extrafiles  -I$(DATA) -I$(SUPERCOP)
LD	= gcc
LDFLAGS := $(LDFLAGS)
LIBS    = -lssl -lcrypto #-lasan #-lkeccak -lsupercop



ARMCC = arm-none-eabi-gcc
ARMARCH = -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARMCFLAG = -O3 -I./extrafiles -I./src -Wall -Wextra -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -Wundef -Wshadow


#############################

SRC:= $(wildcard src/*.c )
SOBJS = $(SRC:.c=.o)
OBJS = $(SOBJS:src/%=%)


#############################

OS := $(shell uname -s)
ifeq  ($(OS), Darwin)
CFLAGS    +=  -D_MAC_OS_
CXXFLAGS  +=  -D_MAC_OS_
endif


ifdef KAT
CFLAGS  += -D_NIST_KAT_
OBJS    += rng.o
endif


#############################

.INTERMEDIATE:  $(OBJS)

#############################

all: test.run


speed.run: speed
	./speed

speed: speed.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

test.run: test
	./test

test: test.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)


kat_kem: $(OBJS) kat_kem.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

#%-test: unit-test/%-test.c $(OBJS)
#	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ -lssl  $(LDFLAGS)

%-test: %-test.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

#%-speed: unit-test/%-speed.c $(OBJS)
#	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ -lssl  $(LDFLAGS)

%-speed: %-speed.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: src/%.c 
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

%.o: unit-test/%.c 
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ -c $< 

%.o: extrafiles/%.c 
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

%.o: %.c 
	$(CC) $(CFLAGS) $(INCLUDE) -c $< 

%.o: $(SUPERCOP)/%.c 
	$(CC) $(CFLAGS) $(INCLUDE) -c $< 

%.o: stm32l4flashtool/%.c 
	$(CC) $(CFLAGS) $(INCLUDE) -c $< 

%.S: src/%.c
	$(ARMCC) $(ARMARCH) $(ARMCFLAG) -S -c $< -o $@


#############################

clean:
	-rm *.o 
	-rm *.q
	-rm test
	-rm speed
	-rm *-test
	-rm *-speed
	-rm kat_kem kat_kem.*
	#-rm *.s 
