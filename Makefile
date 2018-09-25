asm:
	gcc hashtable.c wall-e.c -o asm

eva:
	gcc eva.c vm_ht.c -o vm 
