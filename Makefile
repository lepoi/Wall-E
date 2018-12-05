all:
	-rm asm vm > /dev/null 2>&1
	gcc -g -ggdb hashtable.c wall-e.c -o asm; gcc eva.c vm_ht.c -o vm
	-cp asm ../compa
	-cp vm ../compa
