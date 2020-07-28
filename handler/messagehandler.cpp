//
// Created by Abu Muhammad on 7/10/20.
//

#include "messagehandler.h"

#include "../utils/money.h"
#include "../model/article_model.hpp"

#include <fmt/format.h>
#include <string_view>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include <cpprest/http_listener.h>

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace web::http::experimental::listener;

static constexpr const char* url_total = "https://covid19.mathdro.id/api";
static constexpr const char* api_article = "https://cryptic-brushlands-41995.herokuapp.com/";
static constexpr const char* country_url = "https://covid19.mathdro.id/api/countries";
static constexpr const char* nasional_url = "https://dekontaminasi.com/api/id/covid19/stats";
static constexpr const char* date_format = "%A, %d %B %Y";
static constexpr const char* timestamp_url = "https://dekontaminasi.com/api/id/covid19/stats.timestamp";
static constexpr const char* hoaxs_url = "https://dekontaminasi.com/api/id/covid19/hoaxes";

namespace koloboot {

static std::string months[12]{"Januari", "Februari", "Maret", "April",
"Mei", "Juni", "Juli", "Agustus", "September", "Oktober",
"November", "Desember"};
static std::string weekdays[7]{"Ahad", "Senin", "Selasa",
"Rabu", "Kamis", "Jum\'at", "Sabtu"};

static const std::string prov_key = "provinsi_key";
static const std::string rs_key = "rs_key";

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
    using time_point = boost::chrono::system_clock::time_point;
    time_point uptime_timepoint{boost::chrono::duration_cast<time_point::duration>(boost::chrono::milliseconds(tm))};
    std::time_t time_t = boost::chrono::system_clock::to_time_t(uptime_timepoint);

    std::tm tm_time = boost::posix_time::to_tm(boost::posix_time::from_time_t(time_t));
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

MessageHandler::MessageHandler(TgBot::Bot &bot, nlohmann::json& data,
                               cpp_redis::client& client)
    :m_bot(bot), m_data{data}, m_client{client} {}

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
                    return response.extract_string();
                })
                .then([this, &tg](std::string str){
                    m_client.set(koloboot::prov_key, str);
                    m_client.sync_commit();

                    auto json_obj = nlohmann::json::parse(str);
                    std::string negara = json_obj.at("name").get<std::string>();
                    uint64_t timestamp = 0;
                    get_timestamp()
                    .then([&timestamp](std::vector<unsigned char> v){
                        std::string ss = std::string(v.begin(), v.end());
                        timestamp = boost::lexical_cast<uint64_t>(ss);
                    })
                    .wait();
                    int64_t terkonfirmasi = json_obj.at("numbers").at("infected").get<uint64_t>();
                    int64_t sembuh = json_obj.at("numbers").at("recovered").get<uint64_t>();
                    int64_t meninggal = json_obj.at("numbers").at("fatal").get<uint64_t>();
                    int64_t aktif = terkonfirmasi - (sembuh + meninggal);

                    std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
                    std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
                    std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
                    std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");

                    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
                    std::vector<TgBot::InlineKeyboardButton::Ptr> row0;
                    TgBot::InlineKeyboardButton::Ptr prov_btn(new TgBot::InlineKeyboardButton);
                    prov_btn->text = "Lihat Per Provinsi";
                    prov_btn->callbackData = "provinsi";
                    row0.push_back(prov_btn);
                    keyboard->inlineKeyboard.push_back(row0);

                    std::string text = fmt::format("<b>Total kasus Covid-19 di {}:</b>\n\n"
                                                   "<pre>Terkonfirmasi: {} orang\n"
                                                   "Positif Aktif: {} orang\n"
                                                   "Sembuh: {} orang\n"
                                                   "Meninggal: {} orang\n</pre>\n\n"
                                                   "Update terakhir:\n<b>{}</b>", negara, konfirm_str, aktif_str, sembuh_str, meninggal_str, koloboot::datetime_from_timestamp(timestamp));
                    m_bot.getApi().sendMessage(tg->chat->id, text, false, 0, keyboard, "HTML", false);
                 });
        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });


    m_bot.getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query){
        if (StringTools::startsWith(query->data, "provinsi")) {
            auto get = m_client.get(koloboot::prov_key);
            m_client.sync_commit();

            nlohmann::json json = nlohmann::json::parse(get.get().as_string());
            auto json_obj = json.at("regions").get<std::vector<nlohmann::json>>();
            std::string list = "<b> Data Covid19 Per Provinsi: </b>\n\n";
            int i = 1;
            for (const auto& item: json_obj) {
                std::string name = item.at("name").get<std::string>();
                int64_t terkonfirmasi = item.at("numbers").at("infected").get<uint64_t>();
                int64_t sembuh = item.at("numbers").at("recovered").get<uint64_t>();
                int64_t meninggal = item.at("numbers").at("fatal").get<uint64_t>();
                int64_t aktif = terkonfirmasi - (sembuh + meninggal);

                std::string konfirm_str = Money::getInstance().toMoneyFormat(std::to_string(terkonfirmasi), ".", "");
                std::string sembuh_str = Money::getInstance().toMoneyFormat(std::to_string(sembuh), ".", "");
                std::string meninggal_str = Money::getInstance().toMoneyFormat(std::to_string(meninggal), ".", "");
                std::string aktif_str = Money::getInstance().toMoneyFormat(std::to_string(aktif), ".", "");

                std::string prov = fmt::format("<b>{}. Provinsi {} </b>:\n<pre>"
                                               "Terkonfirmasi: {} orang\n"
                                               "Positif Aktif: {} orang\n"
                                               "Sembuh: {} orang\n"
                                               "Meninggal: {} orang\n</pre>",
                                               i, name, konfirm_str, aktif_str, sembuh_str, meninggal_str);
                list.append(prov).append("\n");
                ++i;
            }
            m_bot.getApi().sendMessage(query->message->chat->id, list, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });
}

pplx::task<std::vector<unsigned char>> MessageHandler::get_timestamp() {

    http_client client(timestamp_url);
    return client.request(methods::GET).then([](http_response response){
        return response.extract_vector();
    });
}

pplx::task<std::vector<unsigned char>> MessageHandler::get_hoaxs() {
    http_client client(hoaxs_url);
    return client.request(methods::GET).then([](http_response response){
        return response.extract_vector();
    });
}

void MessageHandler::hoaxs() {
    m_bot.getEvents().onCommand("hoaks", [this](TgBot::Message::Ptr tg){
        get_hoaxs()
        .then([this, &tg](std::vector<unsigned char> res){
            std::string json_str = std::string(res.begin(), res.end());
            auto json_obj = nlohmann::json::parse(std::move(json_str));
            std::string result = "";
            BOOST_FOREACH(nlohmann::json& item, json_obj) {
                result.append("- ");
                result.append("<a href=\"");
                result.append(item.at("url").get<std::string>());
                result.append("\">");
                result.append(item.at("title"));
                result.append("</a>");
                result.append("\n");
            }

            std::string hoaxs = fmt::format("<b>Awas berita hoaks</b>\n\n"
                                            "<pre>Klik judulnya lihat faktanya</pre>\n\n "
                                            "{}", result);
            m_bot.getApi().sendMessage(tg->chat->id, hoaxs, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        })
        .wait();
    });
}

void MessageHandler::nasehat() {
    m_bot.getEvents().onCommand("nasehat", [this](TgBot::Message::Ptr tg){
        auto request = http_client(U(api_article))
                .request(methods::GET, U("nasehat"))
                .then([this, &tg](http_response response){
                    if (response.status_code() != 200) {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Terjadi kesalahan</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    return response.extract_json();
                })
                .then([this, &tg](json::value json_obj){
                    std::string isOk = json_obj.at("status").as_string();
                    if (isOk == "Ok") {
                        auto data = json_obj.at("data").as_array();
                        std::string result = "<b>Berikut adalah judul nasehat yang bisa anda ambil faedahnya: </b>\n\n"
                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
                        BOOST_FOREACH(const auto& item, data) {
                            result.append("- ");
                            result.append("<a href=\"");
                            result.append(item.at("url").as_string());
                            result.append("\">");
                            result.append(item.at("judul").as_string());
                            result.append("</a>");
                            result.append("\n");
                        }
                        m_bot.getApi().sendMessage(tg->chat->id, result, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);

                    }
                    else {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                });
        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });
}

void MessageHandler::akhbar() {
    m_bot.getEvents().onCommand("akhbar", [this](TgBot::Message::Ptr tg){
        auto request = http_client(U(api_article))
                .request(methods::GET, U("akhbar"))
                .then([this, &tg](http_response response){
                    if (response.status_code() != 200) {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Terjadi kesalahan</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    return response.extract_json();
                 })
                .then([this, &tg](json::value json_obj){
                    std::string isOK = json_obj.at("status").as_string();
                    if (isOK == "Ok") {
                        auto data = json_obj.at("data").as_array();
                        std::string result = "<b>Berikut adalah judul akhbar yang bisa anda ambil faedahnya: </b>\n\n"
                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
                        BOOST_FOREACH(const auto& item, data) {
                            result.append("- ");
                            result.append("<a href=\"");
                            result.append(item.at("url").as_string());
                            result.append("\">");
                            result.append(item.at("judul").as_string());
                            result.append("</a>");
                            result.append("\n");
                        }
                        m_bot.getApi().sendMessage(tg->chat->id, result, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    else {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                });
        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });
}

void MessageHandler::ciamis() {
    m_bot.getEvents().onCommand("ciamis", [this](TgBot::Message::Ptr tg){
        auto request = http_client(U(api_article))
                .request(methods::GET, U("ciamis"))
                .then([this, &tg](http_response response){
                    if (response.status_code() != 200) {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Terjadi kesalahan</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    return response.extract_json();
                })
                .then([this, &tg](json::value json_obj){
                    std::string isOK = json_obj.at("status").as_string();
                    if (isOK == "Ok") {
                        auto data = json_obj.at("data").as_array();
                        std::string header = data[0].as_string();
                        std::string tgl = data[1].as_string();
                        std::string konfirmasi = fmt::format("{}\n{} {}\n{}\n{}",
                                                             data[3].as_string(),
                                data[4].as_string(),
                                data[5].as_string(),
                                data[6].as_string(),
                                data[7].as_string());
                        std::string opp = fmt::format("{}\n{}\n{}\n{} {}", data[8].as_string(),
                                data[9].as_string(), data[10].as_string(), data[11].as_string(), data[12].as_string());
                        std::string odp = fmt::format("{}\n{} {}\n{}\n{}",
                                                      data[13].as_string(),
                                                        data[14].as_string(),
                                                        data[15].as_string(),
                                                        data[16].as_string(),
                                                        data[17].as_string());
                        std::string pdp = fmt::format("{}\n{}\n{}\n{}",
                                                      data[18].as_string(),
                                                        data[19].as_string(),
                                                        data[20].as_string(),
                                                        data[23].as_string());
                        std::string result = fmt::format("<b>{}</b>\n\n<i>{}</i>\n\n{}\n\n{}\n\n{}\n\n{}",
                                                         header,
                                                         tgl,
                                                         konfirmasi,
                                                         opp,
                                                         odp,
                                                         pdp);
                        m_bot.getApi().sendMessage(tg->chat->id, result, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    else {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                });
        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });
}

void MessageHandler::asysyariah() {
    m_bot.getEvents().onCommand("artikel", [this](TgBot::Message::Ptr tg){
        auto request = http_client(U(api_article))
                .request(methods::GET, U("asysyariah"))
                .then([this, &tg](http_response response){
                    if (response.status_code() != 200) {
                        m_bot.getApi().sendMessage(tg->chat->id, "<pre> Terjadi kesalahan</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    return response.extract_json();
                })
                .then([this, &tg](json::value json_obj){
                    std::string isOK = json_obj.at("status").as_string();
                    if (isOK == "Ok") {
                        auto data = json_obj.at("data").as_array();
                        std::vector<Article> articles;
                        articles.reserve(data.size());
                        std::string result = "<b>Berikut adalah judul artikel islami yang bisa anda ambil faedahnya: </b>\n\n"
                                             "<pre>Klik judulnya ambil faedahnya</pre>\n\n";
                        BOOST_FOREACH(json::value item, data) {
                            Article art {item.at("judul").as_string(), item.at("url").as_string()};
                            auto fn = std::find(articles.begin(), articles.end(), art);
                            if (fn == std::end(articles)) {
                                articles.push_back(art);
                                result.append("- ");
                                result.append("<a href=\"");
                                result.append(item.at("url").as_string());
                                result.append("\">");
                                result.append(item.at("judul").as_string());
                                result.append("</a>");
                                result.append("\n");
                            }
                        }
                        articles.erase(articles.begin(), articles.end());
                        articles.shrink_to_fit();
                        m_bot.getApi().sendMessage(tg->chat->id, result, false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                    else {
                         m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
                    }
                });
        try {
            request.wait();
        } catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
             m_bot.getApi().sendMessage(tg->chat->id, "<pre> Tidak dapat menampilkan data</pre>", false, 0, std::make_shared< TgBot::GenericReply >(), "HTML", false);
        }
    });
}
