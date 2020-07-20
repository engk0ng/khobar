//
// Created by Abu Muhammad on 6/29/20.
//

#ifndef QSBOT_OL_MYSTACK_H
#define QSBOT_OL_MYSTACK_H

#include <deque>
#include <string>
#include <cassert>

class MyStack {
private:
    std::deque<std::string> elems;
public:
    void push(const std::string& elem);
    void pop();
    const std::string& top() const;
    bool empty() const {
        return elems.empty();
    }
};

#endif //QSBOT_OL_MYSTACK_H
