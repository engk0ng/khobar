cmake_minimum_required(VERSION 3.1)
project(khobar_covid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system fiber REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(fmt REQUIRED)

set(JSON_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/3rdparty/single_include)
set(DATE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/3rdparty/date/include)

add_executable(khobar_covid
        main.cpp
        utils/mystack.cpp
        utils/mystack.h
        utils/money.h
        controller/command.h
        controller/internasionalcommand.cpp
        controller/internasionalcommand.h
        handler/messagehandler.cpp
        handler/messagehandler.h
        controller/startcommand.cpp
        controller/startcommand.h
        controller/switcher.cpp
        controller/switcher.hpp
        model/country_model.hpp
        data/countries.json
        controller/nasionalcommand.cpp
        controller/nasionalcommand.hpp
        model/article_model.hpp
        )

target_include_directories(khobar_covid
        PRIVATE ${Boost_INCLUDE_DIR}
        ${JSON_INCLUDE_DIRS}
        ${DATE_INCLUDE_DIRS}
        )

target_link_libraries(khobar_covid
        /usr/local/lib/libTgBot.a
        /usr/local/lib/libcpp_redis.a
        /usr/local/lib/libtacopie.a
        cpprestsdk::cpprest
        Threads::Threads
        fmt::fmt
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        )
