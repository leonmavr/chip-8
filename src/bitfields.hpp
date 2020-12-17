#ifndef BITFIELDS_HPP
#define BITFIELDS_HPP 

typedef struct bitfields_t {
	/* see http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0 */
	unsigned type;
	unsigned n;
	unsigned y;
	unsigned x;
	unsigned kk;
	unsigned nnn;
} bitfields;

#endif /* BITFIELDS_HPP */
