//
// Created by Abu Muhammad on 7/10/20.
//

#ifndef KHOBAR_COVID_MESSAGEHANDLER_H
#define KHOBAR_COVID_MESSAGEHANDLER_H

#include <tgbot/tgbot.h>
#include <nlohmann/json.hpp>
#include <cpp_redis/cpp_redis>

#include <cpprest/asyncrt_utils.h>

class MessageHandler {
public:
    MessageHandler(TgBot::Bot& bot,
                   nlohmann::json& data,
                   cpp_redis::client& client
                   );

    void start();
    void internasional();
    void total();
    void nasional();
    void hoaxs();
    void nasehat();

private:
    pplx::task<std::vector<unsigned char>> get_timestamp();
    pplx::task<std::vector<unsigned char>> get_hoaxs();

private:
    TgBot::Bot& m_bot;
    nlohmann::json& m_data;
    cpp_redis::client& m_client;
};


#endif //KHOBAR_COVID_MESSAGEHANDLER_H
