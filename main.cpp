#include <iostream>

#include <tgbot/tgbot.h>
#include <memory>

#include "handler/messagehandler.h"

#include "controller/startcommand.h"
#include "controller/switcher.hpp"
#include "controller/internasionalcommand.h"
#include "controller/nasionalcommand.hpp"

#include <cpp_redis/cpp_redis>

#include <nlohmann/json.hpp>


int main() {
    std::string key = std::getenv("KEY");
    std::string_view token {key};
    std::cout << token << std::endl;

    TgBot::Bot bot {token.data()};

    std::ifstream i("../data/countries.json");
    nlohmann::json jason;
    i >> jason;

    cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);

    cpp_redis::client client;

    client.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::connect_state status) {
        if (status == cpp_redis::connect_state::dropped) {
          std::cout << "client disconnected from " << host << ":" << port << std::endl;
        }
    });

    MessageHandler message {bot, jason, client};
    StartCommand start_cmd {message};
    InternasionalCommand international_cmd {message};
    NasionalCommand nasional_cmd {message};

    Switcher sch;

    sch.start(&start_cmd, Type::Start).action();
    sch.total(&international_cmd, Type::Total).action();
    sch.internasional(&international_cmd, Type::Internasional).action();
    sch.nasional(&nasional_cmd, Type::Nasional).action();
    sch.nasional(&nasional_cmd, Type::Hoaks).action();
    sch.nasional(&nasional_cmd, Type::Nasehat).action();
    sch.nasional(&nasional_cmd, Type::Akhbar).action();
    sch.nasional(&nasional_cmd, Type::Ciamis).action();
    sch.nasional(&nasional_cmd, Type::Artikel).action();

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
