#ifndef COUNTRY_MODEL_HPP
#define COUNTRY_MODEL_HPP

#include <string>

struct CountryModel {
    CountryModel() {}
    CountryModel(
        const std::string& name,
        const std::string& code1,
        const std::string& code2,
        int64_t confirmed,
        int64_t recovered,
        int64_t deaths,
        int64_t active
            ): m_name{name},
        m_code1{code1},
        m_code2{code2},
        m_confirmed{confirmed},
        m_recovered{recovered},
        m_deaths{deaths},
        m_active{active} {}

    void set_name(const std::string& name) {
        m_name = name;
    }
    void set_code1(const std::string& code1) {
        m_code1 = code1;
    }
    void set_code2(const std::string& code2) {
        m_code2 = code2;
    }
    void set_confirmed(int64_t confirmed) {
        m_confirmed = confirmed;
    }
    void set_recovered(int64_t recovered) {
        m_recovered = recovered;
    }
    void set_deaths(int64_t deaths) {
        m_deaths = deaths;
    }
    void set_active(int64_t active) {
        m_active = active;
    }

    std::string get_name() const {
        return m_name;
    }

    std::string get_code1() const {
        return m_code1;
    }

    std::string get_code2() const {
        return m_code2;
    }

    int64_t get_confirmed() const {
        return m_confirmed;
    }

    int64_t get_recovered() const {
        return m_recovered;
    }

    int64_t get_death() const {
        return m_deaths;
    }

    int64_t get_active() const {
        return m_active;
    }

    bool operator==(const CountryModel& other) {
        if ((m_code1 == other.m_code1) && (m_code2 == other.m_code2)) {
            return true;
        }
        else
            return false;
    }

    CountryModel(const CountryModel& other) {
        m_name = other.m_name;
        m_code1 = other.m_code1;
        m_code2 = other.m_code2;
        m_confirmed = other.m_confirmed;
        m_recovered = other.m_recovered;
        m_deaths = other.m_deaths;
        m_active = other.m_active;
    }

    CountryModel& operator=(const CountryModel& other) {
        m_name = other.m_name;
        m_code1 = other.m_code1;
        m_code2 = other.m_code2;
        m_confirmed = other.m_confirmed;
        m_recovered = other.m_recovered;
        m_deaths = other.m_deaths;
        m_active = other.m_active;
        return *this;
    }

    CountryModel(CountryModel&& other) {
        m_name = std::move(other.m_name);
        m_code1 = std::move(other.m_code1);
        m_code2 = std::move(other.m_code2);
        m_confirmed = std::move(other.m_confirmed);
        m_recovered = std::move(other.m_recovered);
        m_deaths = std::move(other.m_deaths);
        m_active = std::move(other.m_active);
    }

    CountryModel& operator=(CountryModel&& other) {
        m_name = std::move(other.m_name);
        m_code1 = std::move(other.m_code1);
        m_code2 = std::move(other.m_code2);
        m_confirmed = std::move(other.m_confirmed);
        m_recovered = std::move(other.m_recovered);
        m_deaths = std::move(other.m_deaths);
        m_active = std::move(other.m_active);
        return *this;
    }

    CountryModel& operator+(const CountryModel& other) {
        m_confirmed += other.m_confirmed;
        m_recovered += other.m_recovered;
        m_deaths += other.m_deaths;
        m_active += other.m_active;
        return *this;
    }
private:
    std::string m_name;
    std::string m_code1;
    std::string m_code2;
    int64_t m_confirmed;
    int64_t m_recovered;
    int64_t m_deaths;
    int64_t m_active;
};

#endif // COUNTRY_MODEL_HPP
