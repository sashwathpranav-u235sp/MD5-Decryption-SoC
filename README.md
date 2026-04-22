# MD5-Decryption-SoC

This is a design for an SoC that uses 32 MD5 decryption engines to turn a 512-bit message into a 128-bit digest. The SoC is designed using VHDL and Qsys on Quartus 16.0. 

The SoC is composed of two separate parts:
1. The MD5 Decryption engines

2. The Hard Processor System (HPS)

The HPS accessess eacha of the 32 MD5 engines using the control wrapper and sends data to them using the data wrapper. These wrappers communicate with the HPS using the Avalon Master-Slave interface. This setup is done using Qsys and upon completion, C header files are created that contain the base addresses for the MD5 data and control wrappers. These files are then used to create a C program that uses the MD5 engines to hash a 512-bit message stored in the program in both serial and parallel execution modes.

Serial execution mode: This process requires only one MD5 engine and it hashes the message in chunks.

Parallel execution mode: This process uses all the 32 MD5 engines to hash chunks of the message
