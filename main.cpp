#include <iostream>

#include <tgbot/tgbot.h>

#include "handler/messagehandler.h"

#include "controller/startcommand.h"
#include "controller/switcher.hpp"
#include "controller/internasionalcommand.h"

#include <nlohmann/json.hpp>

int main() {

    std::string_view token {""};
    std::cout << token << std::endl;

    TgBot::Bot bot {token.data()};

    std::ifstream i("../data/countries.json");
    nlohmann::json jason;
    i >> jason;

    MessageHandler message {bot, jason};
    StartCommand start_cmd {message};
    InternasionalCommand international_cmd {message};
    Switcher sch;

    sch.start(&start_cmd, Type::Start).action();
    sch.total(&international_cmd, Type::Total).action();
    sch.internasional(&international_cmd, Type::Internasional).action();

    try {
        TgBot::TgLongPoll long_pool(bot);
        while (true) {
            long_pool.start();
        }
    } catch (TgBot::TgException& e) {
        std::cout << "Error: \n" << e.what() << std::endl;
    }

    return 0;
}
