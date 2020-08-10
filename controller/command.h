//
// Created by Abu Muhammad on 7/10/20.
//

#ifndef KHOBAR_COVID_COMMAND_H
#define KHOBAR_COVID_COMMAND_H

#include <string_view>

enum class Type {
    Total,
    Start,
    Internasional,
    Country,
    Nasional,
    Hoaks,
    Nasehat,
    Akhbar,
    Ciamis,
    Artikel,
    BNPB,
    CovidGov,
    RS
};

class Command {
public:
    virtual void execute(Type) = 0;
};

#endif //KHOBAR_COVID_COMMAND_H
