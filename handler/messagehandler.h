//
// Created by Abu Muhammad on 7/10/20.
//

#ifndef KHOBAR_COVID_MESSAGEHANDLER_H
#define KHOBAR_COVID_MESSAGEHANDLER_H

#include <tgbot/tgbot.h>
#include <nlohmann/json.hpp>

class MessageHandler {
public:
    MessageHandler(TgBot::Bot& bot, nlohmann::json& data);

    void start();
    void internasional();
    void total();
    void nasional();

private:
    TgBot::Bot& m_bot;
    nlohmann::json m_data;
};


#endif //KHOBAR_COVID_MESSAGEHANDLER_H
