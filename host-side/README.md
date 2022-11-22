# Classic McEliece public key storage server

## Licence

All implementations are in the public domain.

## Usage

- Build a C library for computing inverse matrix.
```
make
```

- Build a serial port channel for simulating communication channel between McEliece code and (python) key server.  
The step is skipped if the McEliece code is executed in the stm32f4 device.
```
socat -d -d pty,rawer pty,rawer
```

- Run the Classic McEliece PK storage server:  
For communication between an M4 and /dev/ttyUSB0 port:
```
python3 ./host-mc348864keysrv.py /dev/ttyUSB0 38400
```
For communication with a simulated /dev/ttys009 serial port:
```
python3 ./host-mc348864keysrv.py /dev/ttys009
```

## Contents

-  gauss_elim.c :  C program for calculating inverse matrix and matrix multiplication.  
-  mc348864keysrv.py :  Main classes of Classic McEliece public keys.  
-  host-mc348864keysrv.py :  Code of storage server.  

