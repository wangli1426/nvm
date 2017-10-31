////
//// Created by Li Wang on 10/25/17.
////
//#include <stdio.h>
//#include "../blk/file_blk_accessor.h"
//#include "blk_b_plus_tree.h"
//
//class A {
//public:
//    A(): value(1){};
//    void print() {
//        printf("%d\n", get());
//    }
//    virtual int get() {
//        return 1;
//    }
//private:
//    int value;
//};
//
//class B {
//    int get() {
//        return 2;
//    }
//};
//
//class C: public A {
//public:
//     int get() {
//        return 3;
//    }
//};
//
//
//using namespace tree;
//int main() {
//
//
//    A* a = new C();
//    a->print();
//
//
//
//    blk_accessor* accessor = new file_blk_accessor("./temp", 512);
//    accessor->open();
//    BlkBPlusTree<int, int, 32> tree(accessor);
//    tree.insert(1, 1);
//    tree.insert(2, 10);
//    int value;
//    tree.search(1,  value);
//    printf("%d: %d\n", 1, value);
//    tree.search(2, value);
//    printf("%d: %d\n", 2, value);
//}

int main() {

}