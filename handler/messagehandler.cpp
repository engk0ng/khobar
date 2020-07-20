//
// Created by Abu Muhammad on 7/10/20.
//

#include "messagehandler.h"

#include "../utils/money.h"
#include "../model/country_model.hpp"

#include <fmt/format.h>
#include <string_view>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include <cpprest/asyncrt_utils.h>

#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>


#include <algorithm>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

static constexpr const char* url_total = "https://covid19.mathdro.id/api";
static constexpr const char* api_article = "https://cryptic-brushlands-41995.herokuapp.com/";
static constexpr const char* country_url = "https://covid19.mathdro.id/api/countries";
static constexpr const char* nasional_url = "https://dekontaminasi.com/api/id/covid19/stats";
static constexpr const char* date_format = "%A, %d %B %Y";

namespace koloboot {

static std::string months[12]{"Januari", "Februari", "Maret", "April",
"Mei", "Juni", "Juli", "Agustus", "September", "Oktober",
"November", "Desember"};
static std::string weekdays[7]{"Ahad", "Senin", "Selasa",
"Rabu", "Kamis", "Juma\'at", "Sabtu"};

std::string date_day_format(const boost::gregorian::date& date) {
    boost::gregorian::date d{date.year(), date.month(), date.day()};
    boost::gregorian::date_facet *df = new boost::gregorian::date_facet{date_format};
    df->long_month_names(std::vector<std::string>{months, months + 12});
    df->long_weekday_names(std::vector<std::string>{weekdays,
    weekdays + 7});
    std::cout.imbue(std::locale{std::cout.getloc(), df});
    std::ostringstream os;
    os.imbue(std::locale{std::cout.getloc(), df});
    os << d;
    return os.str();
}

std::string datetime_from_timestamp(uint64_t tm)
{
    std::stringstream str;
    boost::posix_time::time_facet *facet = new boost::posix_time::time_facet(date_format);
    facet->long_month_names(std::vector<std::string>{months, months + 12});
    facet->long_weekday_names(std::vector<std::string>{weekdays,
    weekdays + 7});
    std::cout.imbue(std::locale{std::cout.getloc(), facet});
    str.imbue(std::locale(str.getloc(), facet));
    str << tm; //your time point goes here
    return str.str();
}

std::string to_date_valid(std::string&& str) {
    boost::algorithm::replace_all(str, "T", " ");
    boost::algorithm::replace_all(str, ".000Z", "");
    std::tm tm_time = boost::posix_time::to_tm(boost::posix_time::time_from_string(str));
    boost::local_time::tz_database tz_db;
    try {
          tz_db.load_from_file("../data/date_time_zonespec.csv");
        }catch(boost::local_time::data_not_accessible dna) {
          std::cerr << "Error with time zone data file: " << dna.what() << std::endl;
          exit(EXIT_FAILURE);
        }catch(boost::local_time::bad_field_count bfc) {
          std::cerr << "Error with time zone data file: " << bfc.what() << std::endl;
          exit(EXIT_FAILURE);
        }
    boost::local_time::time_zone_ptr jkt = tz_db.time_zone_from_region("Asia/Jakarta");
    boost::gregorian::date in_date(tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday);
    boost::posix_time::time_duration td(tm_time.tm_hour + 7, tm_time.tm_min, tm_time.tm_sec);
    boost::local_time::local_date_time jkt_time(in_date, td, jkt, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    std::string res = jkt_time.to_string();
    boost::algorithm::replace_all(res, "WIT", "WIB");
    std::vector<std::string> seq;
    boost::algorithm::split(seq, res, boost::is_any_of(" "));
    std::string result = fmt::format("{}, {} {}", date_day_format(jkt_time.date()), seq.at(1), seq.at(2));
    return result;
}

pplx::task<bool> checking_code_coutry(const nlohmann::json& jsn, std::string_view str) {
    bool status = false;
    for (const auto& item: jsn) {
        if (str == item.at("alpha-3").get<std::string_view>()) {
            status = true;
            break;
        }
    }
    return pplx::task_from_result<bool>(status);
}

pplx::task<std::string> get_country_name(const nlohmann::json& jsn, std::string_view code) {
    std::string name = "";
    for (const auto& item: jsn) {
        if (code == item.at("alpha-3").get<std::string_view>()) {
            name = item.at("name").get<std::string>();
            break;
        }
    }
    return pplx::task_from_result<std::string>(name);
}

}

MessageHandler::MessageHandler(TgBot::Bot &bot, nlohmann::json& data):m_bot(bot), m_data{data} {}

void MessageHandler::start() {
    m_bot.getEvents().onCommand("start", [this](TgBot::Message::Ptr tg){
        std::string nama = fmt::format("{} {}", tg->chat->firstName, tg->chat->lastName);
        std::string message = fmt::format("بسم الله الرحمن الرحيم\n\nAhlan <b>{}</b>\n"
                                          "Ini adalah bot yang menyediakan data dan informasi lainnya tentang Covid-19.\n"
                                          "Semoga anda dapat memperoleh manfaat dari bot ini.\n\n"
                                          "Semoga Allah segera mengangkat wabah ini dari negara kita dan negara kaum muslimin lainnya.\n\n"
                                          "Data di bot ini diambil dari berbagai sumber diantaranya:\n"
                                          "1. <a href=\"https://covid19.mathdro.id/api\"> [https://covid19.mathdro.id/api] Muhammad Mustadi </a>\n"
                                          "\n\n"
                                          "<u>@_abumuhammad_</u>", nama);
        m_bot.getApi().sendMessage(tg->chat->id, message, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
    });
}

void MessageHandler::internasional() {
    m_bot.getEvents().onCommand("internasional", [this](TgBot::Message::Ptr tg){
        auto request = http_client(U(country_url))
                .request(methods::GET, U(""))
                .then([&tg, this](http_response response){
                    if (response.status_code() != 200) {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Terjadi kesalahan</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    return response.extract_json();
                })
                .then([&tg, this](json::value json_obj){
                    auto array = json_obj.at("countries").as_array();
                    std::string country_str = "<b>Daftar Negara</b>\n\n"
                                              "Click link yang berada di sebelah <strong>nama negara</strong> untuk menampilkan data.\n\n";
                    int i = 1;
                    for (const auto& item: array) {
                        if (item.size() >= 3) {
                            country_str.append(std::to_string(i));
                            country_str.append(". ");
                            country_str.append(item.at("name").as_string());
                            country_str.append(" ");
                            country_str.append("/");
                            country_str.append(item.at("iso3").as_string());
                            country_str.append("\n");
                        }
                        ++i;
                    }
                    m_bot.getApi().sendMessage(tg->chat->id, country_str, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                });
        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });

    m_bot.getEvents().onAnyMessage([this](TgBot::Message::Ptr message) {
        if (message->text.length() < 4) {
            return;
        }
        std::string code = message->text;
        boost::algorithm::replace_all(code, "/", "");
        koloboot::checking_code_coutry(m_data, code)
        .then([this, &message, &code](bool status) {
            if (status == true) {
                auto request_json = http_client(U(country_url))
                        .request(methods::GET, uri_builder(U("")).append_path(U(message->text)).to_string())
                        .then([](http_response response){
                                if (response.status_code() != 200) {
                                    throw std::runtime_error("Returned " + std::to_string(response.status_code()));
                                }
                                return response.extract_json();
                        })
                        .then([this, &message, &code](json::value json_obj) {
                            int64_t terkonfirmasi = json_obj.at("confirmed").as_object().at("value").as_number().to_int64();
                            int64_t sembuh = json_obj.at("recovered").as_object().at("value").as_number().to_int64();
                            int64_t meninggal = json_obj.at("deaths").as_object().at("value").as_number().to_int64();
                            int64_t aktif = terkonfirmasi - (sembuh + meninggal);
                            std::string update = json_obj.at("lastUpdate").as_string();

                            std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
                            std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
                            std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
                            std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");
                            std::string name = "";
                            koloboot::get_country_name(m_data, code)
                                    .then([&name](std::string res){
                                name = res;
                            })
                            .wait();
                            std::string text = fmt::format("<b>Total kasus Covid-19 di {}:</b>\n\n"
                                                           "<pre>Terkonfirmasi: {} orang\n"
                                                           "Positif Aktif: {} orang\n"
                                                           "Sembuh: {} orang\n"
                                                           "Meninggal: {} orang\n</pre>\n\n"
                                                           "Update terakhir:\n<b>{}</b>", name, konfirm_str, aktif_str, sembuh_str, meninggal_str, koloboot::to_date_valid(std::move(update)));
                            m_bot.getApi().sendMessage(message->chat->id, text, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                        });
                try {
                    request_json.wait();
                } catch (const std::exception &e) {
                    printf("Error exception:%s\n", e.what());
                     m_bot.getApi().sendMessage(message->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                }
            }
            else {
                return;
            }
        })
        .wait();
    });
}

void MessageHandler::total() {
    m_bot.getEvents().onCommand("total", [this](TgBot::Message::Ptr tg){
        auto request = http_client(U(url_total))
                .request(methods::GET, U(""))
                .then([&tg, this](http_response response){
                    if (response.status_code() != 200) {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Terjadi kesalahan</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    return response.extract_json();
                 })
                .then([&tg, this](json::value json_obj){
                    int64_t terkonfirmasi = json_obj.at("confirmed").as_object().at("value").as_number().to_int64();
                    int64_t sembuh = json_obj.at("recovered").as_object().at("value").as_number().to_int64();
                    int64_t meninggal = json_obj.at("deaths").as_object().at("value").as_number().to_int64();
                    int64_t aktif = terkonfirmasi - (sembuh + meninggal);
                    std::string update = json_obj.at("lastUpdate").as_string();

                    std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
                    std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
                    std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
                    std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");

                    std::string text = fmt::format("<b>Total kasus Covid-19 di seluruh dunia:</b>\n\n"
                                                   "<pre>Terkonfirmasi: {} orang\n"
                                                   "Positif Aktif: {} orang\n"
                                                   "Sembuh: {} orang\n"
                                                   "Meninggal: {} orang\n</pre>\n\n"
                                                   "Update terakhir:\n<b>{}</b>", konfirm_str, aktif_str, sembuh_str, meninggal_str, koloboot::to_date_valid(std::move(update)));
                    m_bot.getApi().sendMessage(tg->chat->id, text, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                });

        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });
}

void MessageHandler::nasional() {
    m_bot.getEvents().onCommand("nasional", [this](TgBot::Message::Ptr tg){
        auto request = http_client(U(nasional_url))
                .request(methods::GET, U(""))
                .then([&tg, this](http_response response){
                    if (response.status_code() != 200) {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Terjadi kesalahan</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    return response.extract_json();
                })
                .then([&tg, this](json::value json_obj){
                    std::string negara = json_obj.at("name").as_string();
                    uint64_t timestamp = json_obj.at("timestamp").as_number().to_uint64();

                 });
        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });
}
