#include <stdio.h>
#include "tree/vanilla_b_plus_tree.h"



using namespace tree;
int main() {
	VanillaBPlusTree<int, int, 4> tree;
	tree.insert(1, 1);
	tree.insert(2, 2);
	tree.insert(3, 3);
	tree.insert(4, 4);
	tree.insert(5, 5);
	tree.insert(6, 6);
	tree.insert(7, 7);
	tree.insert(8, 8);

	tree.delete_key(4);

	printf("%s\n", tree.toString().c_str());

	tree.delete_key(4);
	tree.delete_key(0);
	tree.delete_key(1);
	tree.delete_key(3);
	printf("%s\n", tree.toString().c_str());

	tree.delete_key(5);
	printf("%s\n", tree.toString().c_str());

    tree.delete_key(6);
    printf("%s\n", tree.toString().c_str());

	

}