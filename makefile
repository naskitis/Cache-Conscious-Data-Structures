compile_all:
#standard chain data structures
	gcc -O3 -fomit-frame-pointer -w -o standard-redblack standard-redblack.c common.c
	gcc -O3 -fomit-frame-pointer -w -o standard-splay standard-splay.c common.c
	gcc -O3 -fomit-frame-pointer -w -o standard-bst standard-bst.c common.c
	gcc -O3 -fomit-frame-pointer -w -DMTF_ON -o standard-hash standard-hash.c common.c
	gcc -O3 -fomit-frame-pointer -w -DMTF_ON -o standard-burst-trie standard-burst-trie.c common.c
#array-based data structures
	gcc -O3 -fomit-frame-pointer -w -o nikolas_askitis_array_bst nikolas_askitis_array_bst.c common.c
	gcc -O3 -fomit-frame-pointer -w -DEXACT_FIT -DMTF_ON -o nikolas_askitis_array_hash_exact_mtf nikolas_askitis_array_hash.c common.c
	gcc -O3 -fomit-frame-pointer -w -DEXACT_FIT -o nikolas_askitis_array_hash_exact nikolas_askitis_array_hash.c common.c
	gcc -O3 -fomit-frame-pointer -w -DPAGING -o nikolas_askitis_array_hash_page nikolas_askitis_array_hash.c common.c
	gcc -O3 -fomit-frame-pointer -w -DEXACT_FIT -o nikolas_askitis_array_burst_trie_exact nikolas_askitis_array_burst_trie.c common.c
	gcc -O3 -fomit-frame-pointer -w -DPAGING -o nikolas_askitis_array_burst_trie_page nikolas_askitis_array_burst_trie.c common.c
#quick usage quide
	@echo
	@cat USAGE_POLICY.txt;
	@echo "\nUsage  eg: ./array-hash-exact 65536  2  insert-file01 insert-file02  1  search-file01"
	@echo "           ./standard-bst  1  insert-file01  3  search-file01 search-file02 search-file03";
	@echo "           ./array-burst-trie-exact  256  1 insert-file01 1 search-file01";
	@echo;
	@echo "Output eg: 'Array-BST 24.79 6.17 6.17 1000000 1000000' = data struct, space(MB), insert(sec), search(sec), inserted, found ";
	@echo;
	@echo "Dr. Nikolas Askitis | Copyright @ 2016 | askitisn@gmail.com"
	@echo;
clean:
	rm -rf t*[!.c]
	rm -rf n*[!.c]
	rm -rf s*[!.c]
