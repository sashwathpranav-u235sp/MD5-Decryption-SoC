

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#define LW_SIZE 0x00200000
#define LWHPS2FPGA_BASE 0xff200000
#define NUM_MD5_UNITS 16

/* FIX: md5_control and md5_data have been removed — they no longer exist in
 * the updated hps_0.h (MD5_DATA_0_BASE and MD5_CONTROL_0_BASE are gone).
 * Only the group-level peripherals are present in this system revision.
 */
volatile uint32_t *md5_group_control = NULL;
volatile uint32_t *md5_group_data = NULL;
int success, total;
void *virtual_base;

/*
 * ASCII
 *
 * T-0x54 o-0x6f r-0x72 n-0x6e t-0x74
 * M-0x4d e-0x65 p-0x70 l-0x6c i-0x69, a-0x61
 * U-0x55, v-0x76, s-0x73, y-0x79
 *
 */

// Chunk
uint32_t message[16] = {
		// TorontoM
		0x546f726f, 0x6e746f4d,
		// etropoli
		0x6574726f, 0x706f6c69,
		// tanUnive
		0x74616e55, 0x6e697665,
		// rsityTor
		0x72736974, 0x79546f72,
		// ontoMetr
		0x6f6e746f, 0x4d657454,
		// opolitan
		0x6f706f6c, 0x6974616e,
		// Universi
		0x556e6976, 0x65546f72,
		// tyToront
		0x7476546f, 0x726f6e74
};

uint32_t messagetb[16] = {
		0x01680208,
		0x13ab80bb,
		0xcb8b2c30,
		0xb9657582,
		0xa3793c48,
		0x103f26be,
		0x0b78dac4,
		0x5c433348,
		0x4de99287,
		0xeff0be7c,
		0x00808533,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000150,
		0x00000000
};

uint32_t expected_digest[4] = {
		0x2ad26682,
		0x14ba892c,
		0x61d3eb27,
		0xbaebddf8
};

uint32_t final_digest[4];
uint32_t total_hashes = 1;

void print_message(uint32_t message[16]) {
	int k;

	printf("Hex version:\n");
	for(k = 0; k < 16; k++) {
		printf("0x%08x\n", message[k]);
	}
}

/* FIX: reset_system() previously asserted reset then entered an infinite loop
 * polling for the bit to self-clear. The hardware never auto-clears the reset
 * register — only the CPU can write it back to 0. The second while() loop has
 * been replaced with an explicit write of 0x0 to deassert reset.
 */
void reset_system() {
	printf("Resetting system\n");
	alt_write_word(md5_group_control + 1, 0x1); // assert reset

	usleep(100); // allow reset to propagate

	alt_write_word(md5_group_control + 1, 0x0); // deassert reset
	printf("Reset done.\n");
}

/*
 * MD5_group:
 * - writeaddr(8 downto 5); // Choosing Unit
 * - 0000->1111 = 0-15
 *
 *
 * MRAM-MD5_UNIT:
 * - address_a => wraddress => writeaddr(3 downto 0);
 */

void send_message_to_md5_parallel(uint32_t message[16]){
	int i;

	// wait for wr
	printf("waiting for wr\n");
	alt_write_word(md5_group_data+3, 0x1); // assert wr

	while(!(alt_read_word(md5_group_data+4) & 0x1));

	printf("sending messages\n");
	for (i = 0; i < NUM_MD5_UNITS; i++) {
		alt_write_word(md5_group_data+1, i);
		alt_write_word(md5_group_data, message[i]);
		printf("0x%08x\n", message[i]);
		usleep(100);
	}

	alt_write_word(md5_group_data+3, 0x0); // deassert wr
	usleep(100);
}
	/*
	 *
	 * 	wr							: IN STD_LOGIC;
	 * 	writedata					: IN STD_LOGIC_VECTOR(31 DOWNTO 0);
	 * 	writeaddr					: IN STD_LOGIC_VECTOR(8 DOWNTO 0);
	 * 		- to md5_unit: writeaddr(4 DOWNTO 0);
	 * 		- to decode: writeaddr(8 DOWNTO 5);
	 * 	readaddr					: IN STD_LOGIC_VECTOR(6 DOWNTO 0);
	 *		- digest_sel <= readaddr(6 DOWNTO 2);
	 *		- word_sel <= readaddr(1 DOWNTO 0);
	 */

void start_unit_parallel() {
	printf("Starting units\n");
	alt_write_word(md5_group_control, 0b00000000000000000000000000000001);
	usleep(100);
	alt_write_word(md5_group_control, 0b00000000000000000000000000000000);

	// Wait for done
	while(!(alt_read_word(md5_group_control+2) & 0b11111111111111111111111111111111));
}

void read_digest_parallel() {
	int i;

	for (i = 0; i < 4; i++) {
		usleep(100);
		alt_write_word(md5_group_data+2, i);
		usleep(100);
		final_digest[i] = alt_read_word(md5_group_data);
		printf("Digest: 0x%08x\n", final_digest[i]);
	}
}

void send_message_to_md5_serial(uint32_t message[16]){
	int i;

	// wait for wr
	printf("waiting for wr\n");
	alt_write_word(md5_group_data+3, 0x1); // assert wr

	while(!(alt_read_word(md5_group_data+4) & 0x1));

	printf("sending messages\n");
	for (i = 0; i < NUM_MD5_UNITS; i++) {
		uint32_t writeaddr = (0 << 5) | i;
		alt_write_word(md5_group_data+1, writeaddr);
		alt_write_word(md5_group_data, message[i]);
		printf("0x%08x\n", message[i]);
		usleep(100);
	}

	alt_write_word(md5_group_data+3, 0x0); // deassert wr
	usleep(100);
}
	/*
	 *
	 * 	wr							: IN STD_LOGIC;
	 * 	writedata					: IN STD_LOGIC_VECTOR(31 DOWNTO 0);
	 * 	writeaddr					: IN STD_LOGIC_VECTOR(8 DOWNTO 0);
	 * 		- to md5_unit: writeaddr(4 DOWNTO 0);
	 * 		- to decode: writeaddr(8 DOWNTO 5);
	 * 	readaddr					: IN STD_LOGIC_VECTOR(6 DOWNTO 0);
	 *		- digest_sel <= readaddr(6 DOWNTO 2);
	 *		- word_sel <= readaddr(1 DOWNTO 0);
	 */

void start_unit_serial() {
	printf("Starting units\n");

	alt_write_word(md5_group_control, 0b00000000000000000000000000000001);
	usleep(100);
	alt_write_word(md5_group_control, 0b00000000000000000000000000000000);

	// Wait for done
	while(!(alt_read_word(md5_group_control+2) & 0x00000001));
}

void read_digest_serial() {
	int i;

	for (i = 0; i < 4; i++) {
		usleep(100);
		alt_write_word(md5_group_data+2, i);
		usleep(100);
		final_digest[i] = alt_read_word(md5_group_data);
		printf("Digest: 0x%08x\n", final_digest[i]);
	}
}

/* FIX: print_addresses() updated to remove md5_control and md5_data, which
 * no longer exist in the updated hps_0.h.
 */
void print_addresses() {
	printf("Printing Addresses:\n");
	printf("LWHPS2FPGA_BASE   = %p\n", (void *)LWHPS2FPGA_BASE);
	printf("virtual_base      = %p\n", virtual_base);
	printf("md5_group_control = %p\n", md5_group_control);
	printf("md5_group_data    = %p\n", md5_group_data);
}

int main(int argc, char **argv){

	int fd, i;
	uint32_t temp_final, temp_expected;
	struct timespec start, end;
	double exec_time_ms, exec_time_s, hash_rate;

	success = 0; total = 0;

	//map address space of fpga for software to access here
	if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
		printf("ERROR: could not open \"/dev/mem\"...\n");
		return(1);
	}

	virtual_base = mmap(NULL, LW_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LWHPS2FPGA_BASE);

	if (virtual_base == MAP_FAILED) {
		printf("ERROR: mmap() failed...\n");
		close(fd);
		return(1);
	}

	/* FIX: Initialise pointers from the updated hps_0.h base addresses.
	 * In the new header:
	 *   MD5_GROUP_CONTROL_0_BASE = 0x00
	 *   MD5_GROUP_DATA_0_BASE    = 0x40
	 * Previously these were swapped (control=0x40, data=0x00), which meant
	 * every register write was hitting the wrong peripheral entirely.
	 * The md5_control and md5_data initialisations have been removed
	 * entirely because those macros no longer exist in hps_0.h.
	 */
	md5_group_control = virtual_base + ((uint32_t)(MD5_GROUP_CONTROL_0_BASE));
	md5_group_data    = virtual_base + ((uint32_t)(MD5_GROUP_DATA_0_BASE));

	printf("------>Finished initializing HPS/FPGA system<-------\n");

	reset_system();
//	print_message(messagetb);
	printf("------>Parallel Version:<-------\n");
	send_message_to_md5_parallel(messagetb);

	clock_gettime(CLOCK_MONOTONIC, &start);
	start_unit_parallel();
	clock_gettime(CLOCK_MONOTONIC, &end);

	read_digest_parallel();

	// result =? expected
	printf("------>Parallel Results:<-------\n");
	for (i = 0; i < 4; i++) {
		temp_final    = final_digest[i];
		temp_expected = expected_digest[i];
		printf("Comparing 0x%08x with 0x%08x...", temp_final, temp_expected);
		if (temp_final == temp_expected) {
			printf("SUCCESS!\n");
		} else {
			printf("FAILED!\n");
		}
	}

	exec_time_ms  = (end.tv_sec - start.tv_sec) * 1000.0;
	exec_time_ms += (end.tv_nsec - start.tv_nsec) / 1e6;
	exec_time_s   = exec_time_ms / 1000.0;
	hash_rate     = total_hashes / exec_time_s;

	printf("Final digest: 0x%08x%08x%08x%08x\n", final_digest[3], final_digest[2], final_digest[1], final_digest[0]);
	printf("Total execution time: %.3f ms\n", exec_time_ms);
	printf("Total hashes computed: %u\n", total_hashes);
	printf("Hash rate: %.2f\n", hash_rate);

	// Parallel end

	reset_system();
//	print_message(messagetb);
	printf("------>Serial Version:<-------\n");
	send_message_to_md5_serial(messagetb);

	clock_gettime(CLOCK_MONOTONIC, &start);
	start_unit_serial();
	clock_gettime(CLOCK_MONOTONIC, &end);

	read_digest_serial();

	// result =? expected
	printf("------>Serial Results:<-------\n");
	for (i = 0; i < 4; i++) {
		temp_final    = final_digest[i];
		temp_expected = expected_digest[i];
		printf("Comparing 0x%08x with 0x%08x...", temp_final, temp_expected);
		if (temp_final == temp_expected) {
			printf("SUCCESS!\n");
		} else {
			printf("FAILED!\n");
		}
	}

	exec_time_ms  = (end.tv_sec - start.tv_sec) * 1000.0;
	exec_time_ms += (end.tv_nsec - start.tv_nsec) / 1e6;
	exec_time_s   = exec_time_ms / 1000.0;
	hash_rate     = total_hashes / exec_time_s;

	printf("Final digest: 0x%08x%08x%08x%08x\n", final_digest[3], final_digest[2], final_digest[1], final_digest[0]);
	printf("Total execution time: %.3f ms\n", exec_time_ms);
	printf("Total hashes computed: %u\n", total_hashes);
	printf("Hash rate: %.2f\n", hash_rate);

	// clean up our memory mapping and exit
	if (munmap(virtual_base, LW_SIZE) != 0) {
		printf("ERROR: munmap() failed...\n");
		close(fd);
		return(1);
	}

	close(fd);
	return 0;
}
