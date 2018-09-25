typedef unsigned short addr_t;

struct asm_state {
	FILE *fp_out;
	addr_t data_size;
	addr_t exec_size;
	addr_t count;
	struct hashtable *var_addrs;
	struct hashtable *ins;
	struct hashtable *labels;
	unsigned int line_number;
};
