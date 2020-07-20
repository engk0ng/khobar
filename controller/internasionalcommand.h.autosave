//
// Created by Abu Muhammad on 7/10/20.
//

#ifndef KHOBAR_COVID_INTERNASIONALCOMMAND_H
#define KHOBAR_COVID_INTERNASIONALCOMMAND_H

#include "command.h"
#include "../handler/messagehandler.h"

class InternasionalCommand: public Command {
public:
    InternasionalCommand(MessageHandler& handler);

    virtual void execute(Type type) override {
        if (type == Type::Total) {
            m_handler.total();
        }
        else if (type == Type::Internasional) {
            m_handler.internasional();
        }
    }

private:
    MessageHandler& m_handler;
};


#endif //KHOBAR_COVID_INTERNASIONALCOMMAND_H
