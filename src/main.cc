#include <stdio.h>
#include "tree/vanilla_b_plus_tree.h"

using namespace tree;
int main() {
	BTree<int, int>* t = new VanillaBPlusTree<int, int, 64>();

	t->insert(1, 1);
	t->insert(2, 2);
	t->insert(-1, -1);
	BTree<int, int>::Iterator *it = t->range_search(-100, 1000);
	int key, value;
	while(it->next(key, value)) {
		printf("key: %d, value: %d\n", key, value);
	}

	key = 2;
	bool found = t->search(key, value);
	if (found) {
		printf("found: %d", value);
	}

	

}