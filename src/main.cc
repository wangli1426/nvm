#include <stdio.h>
#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "tree/in_disk_b_plus_tree.h"



using namespace tree;

class A {
public:
    void set() {
        a = 100;
    }
    void print() {
        printf("%d\n", a);
    }
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & a;
    }
private:
    int a;
};

int main() {
    A a, b;
    a.set();
    std::ofstream ofs("filename");
    boost::archive::text_oarchive oa(ofs);
    oa << a;
    ofs.close();


    std::ifstream ifs("filename");
    boost::archive::text_iarchive ia(ifs);
    // read class state from archive
    ia >> b;
    b.print();
}