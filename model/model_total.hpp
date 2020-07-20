#ifndef MODEL_TOTAL_HPP
#define MODEL_TOTAL_HPP

#include <nlohmann/json.hpp>

namespace koloboot {
    using nlohmann::json;

    inline json get_untyped(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json & j, std::string property) {
        return get_untyped(j, property.data());
    }

    struct confirmed {
        int64_t value;
        std::string detail;
    };

    struct country_detail {
        std::string pattern;
        std::string example;
    };

    struct TotalModel {
        confirmed confirmed;
        confirmed recovered;
        confirmed deaths;
        std::string daily_summary;
        country_detail daily_time_series;
        std::string image;
        std::string source;
        std::string countries;
        country_detail country_detail;
        std::string last_update;
    };
}

namespace nlohmann {
    void from_json(const json & j, koloboot::confirmed & x);
    void to_json(json & j, const koloboot::confirmed & x);

    void from_json(const json & j, koloboot::country_detail & x);
    void to_json(json & j, const koloboot::country_detail & x);

    void from_json(const json & j, koloboot::TotalModel & x);
    void to_json(json & j, const koloboot::TotalModel & x);

    inline void from_json(const json & j, koloboot::confirmed& x) {
        x.value = j.at("value").get<int64_t>();
        x.detail = j.at("detail").get<std::string>();
    }

    inline void to_json(json & j, const koloboot::confirmed & x) {
        j = json::object();
        j["value"] = x.value;
        j["detail"] = x.detail;
    }

    inline void from_json(const json & j, koloboot::country_detail& x) {
        x.pattern = j.at("pattern").get<std::string>();
        x.example = j.at("example").get<std::string>();
    }

    inline void to_json(json & j, const koloboot::country_detail & x) {
        j = json::object();
        j["pattern"] = x.pattern;
        j["example"] = x.example;
    }

    inline void from_json(const json & j, koloboot::TotalModel& x) {
        x.confirmed = j.at("confirmed").get<koloboot::confirmed>();
        x.recovered = j.at("recovered").get<koloboot::confirmed>();
        x.deaths = j.at("deaths").get<koloboot::confirmed>();
        x.daily_summary = j.at("dailySummary").get<std::string>();
        x.daily_time_series = j.at("dailyTimeSeries").get<koloboot::country_detail>();
        x.image = j.at("image").get<std::string>();
        x.source = j.at("source").get<std::string>();
        x.countries = j.at("countries").get<std::string>();
        x.country_detail = j.at("countryDetail").get<koloboot::country_detail>();
        x.last_update = j.at("lastUpdate").get<std::string>();
    }

    inline void to_json(json & j, const koloboot::TotalModel & x) {
        j = json::object();
        j["confirmed"] = x.confirmed;
        j["recovered"] = x.recovered;
        j["deaths"] = x.deaths;
        j["dailySummary"] = x.daily_summary;
        j["dailyTimeSeries"] = x.daily_time_series;
        j["image"] = x.image;
        j["source"] = x.source;
        j["countries"] = x.countries;
        j["countryDetail"] = x.country_detail;
        j["lastUpdate"] = x.last_update;
    }
}

#endif // MODEL_TOTAL_HPP
