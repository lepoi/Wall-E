#define MAGIC_NUMBER "\x00\x45\x56\x41\x00\x00\x00\x00\x00\x00\x00\x32\x38\x32\x30\x32"
#define HEADER_OFFSET sizeof(MAGIC_NUMBER) - 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int u64;

struct string {
	char *str;
	u8 size;
};
