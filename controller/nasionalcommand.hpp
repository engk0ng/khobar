#ifndef NASIONALCOMMAND_HPP
#define NASIONALCOMMAND_HPP

#include "command.h"
#include "../handler/messagehandler.h"


class NasionalCommand: public Command
{
public:
    NasionalCommand(MessageHandler& hanlder);

    virtual void execute(Type type) override {
        if (type == Type::Nasional)
            m_hanlder.nasional();
        else if (type == Type::Hoaks)
            m_hanlder.hoaxs();
        else if (type == Type::Nasehat)
            m_hanlder.nasehat();
    }
private:
    MessageHandler& m_hanlder;
};

#endif // NASIONALCOMMAND_HPP
