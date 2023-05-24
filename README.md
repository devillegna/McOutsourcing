# Classic McEliece implementation for ARM-Cortex M4

Code for experiments of outsourcing key-generation of Classic McEliece.  
  
Goal:  minimizing stack usage of key-generation by outsourcing some computation to un-trusted remote pcs.

## Authors

Ming-Shing Chen (IIS, Academia Sinica) and Tung Chou (CITI, Academia Sinica)

## Licence

All implementations are in the public domain.

## APIs

These functions are called by the device for out-sourcing key-generation.
```
// pksrv_nXXXX_tXXX.h
uint32_t pksrv_init( parameters );  // return a (communication) token, for generating a new key
uint32_t pksrv_retrive_pk( const uint8_t* pkhash );  // return a token, for retriving generated pk with its hash value

unsigned pksrv_store_prepk( uint32_t token , const uint8_t* prepk_chunk , unsigned idx_prepk );
unsigned pksrv_compute_pk( uint32_t token );
unsigned pksrv_get_pk(uint32_t token, uint8_t * pk_chunk , unsigned idx_pk );
```

## Contents

- host-side/: A (python) pk server executed in the PC side.  
- u32_nXXXX_tXXXX: Classic McEliece cdoe with various parameters for performing the out-sourcing experiments in a M4 device.  

- **pqm4-projects**: Projects for PQM4(https://github.com/mupq/pqm4).  
 [Note] The implementation generates the same KATs as the R3 submisions in https://classic.mceliece.org/ .  

## Running tests and benchmarks with the mps2-an386 simulator

Follow the setup of PQM4 first.

Then obtain the pqm4 library and the submodules:
```
git clone --recursive https://github.com/mupq/pqm4.git
```
Copy files to the PQM4 directory:
```
cp -r u32_nxxxx_txxx/src  /pqm4/crypto_kem/u32_nxxxx_txxx
```
Go to the PQM4:
```
cd pqm4
```
Build projects for mps2-an386 platform:
```
make -j4 PLATFORM=mps2-an386 IMPLEMENTATION_PATH=crypto_kem/u32_nxxxx_txxx/
```
Set up a pk server(see /host-side).

Finally, run tests or benchmarks:
```
qemu-system-arm -M mps2-an386 -nographic -semihosting -serial /dev/pts/x -kernel elf/crypto_kem_u32_nxxxx_txxx__xxxxx.elf
```
## Experiments and Results:

### Experiments:
- Set up a pk server in the PC side(see host-side/host-mc348864keysrv.py), listening to a particular tty device (e.g., /dev/ttyUSB0)  

- The M4 device (STM32F4-discovery) performs key-generation. The steps:  
  1) The M4 device sends some public pre-pk chunks to pk server in the PC side.  
  2) The M4 device will query if the pk server successfully generates the pk. The server side performs a Gaussian elimination and returns its results.  
  3) If successfully generating a pk, the M4 device requests PK chunks from the server side.  
  4) While receiving pk chunks, the M4 device tests its integrity with its sk. If the pk is correct, the device calcuates its hash and stores hashes instead of a real pk.  
   (Note: the M4 can always generate its pk from its sk. It choices to store the pk in the key server to save its local storage space.)  

- The M4 device performs encapsulation with pkhsahes:
  1) With the stored pk hashes, the M4 device ask the server if the server stored its pk.  
  2) If yes, the device requests the pk chunks.  
  3) While receiving pk chunks, the M4 device check its integrity with the stored hashes of the pk chunks.  
  4) The device performs encapsulation chunk by chunk.  

  We hope the encapsulation experiment can show the scenario:  
  We assumed that the key server (host-side/) stored MANY public keysfrom many devices it served.
  Supposing device A wants to generate a shared secret with device B and device A has only the HASH of public key of B (PKb), then device A can request PKb from the key server by the HASH of PKb.
  The server sends PKb to device A chunk by chunk, and device A performs the encapsulation while checking the integrity of PKb  via its own HASH of PKb.  
  In the experiment, it might be misleading since the M4 performs encapsulation with its own pk. The point here is that the M4 device can request any pk from the server as long as the M4 device has the hashes of requested pks.

- Decapsulation: The M4 device decaps the previous encap results for a shared key.  


### Benchmarks:

We report experiment results on a STM32F4-discovery(168MHz) and a pk sever on a x86 PC.  
The connection between M4 and PC is M4 USART port (baud rate 38400 bits/sec) <-> USB dongle <-> PC.  

The API for KEM and execution evniroment and the same with the PQM4.  
(PQM4 version: 2691b4915 Nov/29/2021 and arm-none-eabi-gcc (15:9-2019-q4-0ubuntu2) 9.2.1).  

### Execution time:


- Turnaroud time (successfully generating pk with only one attempt):  

| Parameter        | keypair       | encaps       | decaps      |
| :---             |          ---: |       ---:   |        ---: |
| mceliece348864   | 180.843(sec)  |  70.082(sec) | 0.0160(sec) |
| mceliece460896   | 404.980(sec)  | 140.186(sec) | 0.0388(sec) |
| mceliece6688128  | 799.080(sec)  | 279.089(sec) | 0.0439(sec) |
| mceliece6960119  | 782.217(sec)  | 281.028(sec) | 0.0425(sec) |
| mceliece8192128  | 996.62(sec)   | 364.21(sec)  | 0.0442(sec) |



### Communication and other costs
- Commnunication baud rate: 38400 bits/second  

- Key-generation  

| Parameter        | Turnaround time  | Measured Communication time | Expected Communication time  | 
| :---             |          ---:    |                      ---: |                           ---: |
| mceliece348864   | 180.843(sec)     |      161.914(sec)         |           127.066(sec)         |
| mceliece460896   | 404.980(sec)     |      333.427(sec)         |           262.8(sec)           |
| mceliece6688128  | 799.080(sec)     |      650.619(sec)         |           513.093(sec)         |
| mceliece6960119  | 782.217(sec)     |      642.082(sec)         |           506.009(sec)         |
| mceliece8192128  | 996.62(sec)      |                           |           644.693(sec)         | 


| Parameter        | SK-Set up | pre-pk chunks(SYS) -> server  | Gassian(PC) | pre-pk chunks(PK) -> server    | pk chunks <- server | check integrity and hashes |
| :---             |   ---:    |                          ---: |        ---: |                           ---: |                ---: |                       ---: |
| mceliece348864   |           | 16.000000(sec)                |             |  56.666667(sec)                |   54.400000(sec) |                             |
| mceliece460896   |           | 41.600000(sec)                |             | 112.000000(sec)                |  109.200000(sec) |                             |
| mceliece6688128  |           | 73.493333(sec)                |             | 221.893333(sec)                |  217.706667(sec) |                             |
| mceliece6960119  |           | 64.475833(sec)                |             | 222.375833(sec)                |  219.158333(sec) |                             |
| mceliece8192128  |           | 73.493333(sec)                |             | 288.320000(sec)                |  282.880000(sec) |                             |


| Parameter        | chunk size (pre-pk,m4->server) | chunk size (pk,m4<-server)    |  pre-pk chunks(SYS+PK) -> server  |   pk chunks <- server |
| :---             |                        ---:    |                          ---: |                              ---: |                 ---:  |
| mceliece348864   | 3200 x (24+85)                 | 3072 x 85                     |  76800+272000(byte)               |  261120(byte)         |
| mceliece460896   | 5120 x (39+105)                | 4992 x 105                    | 199680+537600(byte)               |  524160(byte)         |
| mceliece6688128  | 6784 x (52+157)                | 6656 x 157                    | 352768+1065088(byte)              | 1044992(byte)         |
| mceliece6960119  | 6316 x (49+169)                | 6188 x 170                    | 309484+1067404(byte)              | 1051960(byte)         |
| mceliece8192128  | 6784 x (52+204)                | 6656 x 204                    | 352768+1383936(byte)              | 1357824(byte)         |





- Encapsulation  

| Parameter        | Turnaround time  | Measured Communication time | Expected Communication time |
| :---             |            ---:  |                        ---: |                      ---:   |
| mceliece348864   |  70.082(sec)     |   69.918(sec)               |              54.400000(sec) |
| mceliece460896   | 140.186(sec)     |  139.860(sec)               |             109.200000(sec) |
| mceliece6688128  | 279.089(sec)     |  278.443(sec)               |             217.706667(sec) |
| mceliece6960119  | 281.028(sec)     |  280.374(sec)               |             219.158333(sec) |
| mceliece8192128  | 364.21(sec)      |                             |             282.880000(sec) |



### Stack usage:

Benchamrked on mps2-an386 with the mupq/crypto_kem/stack.c in the PQM4.  

| Parameter        | keypair | encaps | decaps  |
| :---             |   ---:  |  ---:  |    ---: |
| mceliece348864   | 52620   | 7204   | 18492   |
| mceliece460896   | 96572   | 11308  | 34964   |
| mceliece6688128  | 100920  | 14996  | 35708   |
| mceliece6960119  | 96972   | 14780  | 35756   |
| mceliece8192128  | 100816  | 15188  | 36092   |

These numbers are the measurements of stack usage of related functions. It does not contain the key pair storage provided by the caller functions.  

