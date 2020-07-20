//
// Created by Abu Muhammad on 7/10/20.
//

#ifndef KHOBAR_COVID_STARTCOMMAND_H
#define KHOBAR_COVID_STARTCOMMAND_H

#include "command.h"
#include "../handler/messagehandler.h"

class StartCommand: public Command {
public:
    StartCommand(MessageHandler& handler);
    virtual void execute(Type) override {
        m_handler.start();
    }

private:
    MessageHandler& m_handler;
};


#endif //KHOBAR_COVID_STARTCOMMAND_H
