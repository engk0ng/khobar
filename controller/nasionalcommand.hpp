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
        else if (type == Type::Akhbar)
            m_hanlder.akhbar();
        else if (type == Type::Ciamis)
            m_hanlder.ciamis();
        else if (type == Type::Artikel)
            m_hanlder.asysyariah();
        else if (type == Type::BNPB)
            m_hanlder.bnpb();
        else if (type == Type::CovidGov)
            m_hanlder.covidgov();
    }
private:
    MessageHandler& m_hanlder;
};

#endif // NASIONALCOMMAND_HPP
