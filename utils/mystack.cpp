//
// Created by Abu Muhammad on 6/29/20.
//

#include "mystack.h"

void MyStack::push(const std::string &elem) {
    elems.push_back(elem);
}

void MyStack::pop() {
    assert(!elems.empty());
    elems.pop_back();
}

const std::string& MyStack::top() const {
    assert(!elems.empty());
    return elems.back();
}