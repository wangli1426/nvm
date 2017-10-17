//
// Created by Li Wang on 10/17/17.
//

#include <iostream>
#include "accessor/ns_entry.h"
#include "accessor/qpair_context.h"
using namespace std;
using namespace nvm;
int main() {
    cout << "perf test" << endl;
    nvm::initialize_namespace();
    QPair qpair = allocateQPair(8);

}
