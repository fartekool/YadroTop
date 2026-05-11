cmake_minimum_required(VERSION 3.22)
project(YadroTopMonitor)

# Стандарт C++17 обязателен для Crow
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем, где лежат твои хедеры
include_directories(include)

# Поиск потоков (pthread в Linux)
find_package(Threads REQUIRED)

# ВАЖНО: флаги для Crow и Asio
# ASIO_STANDALONE — используем Asio без всего Boost
add_definitions(-DASIO_STANDALONE)

# Собираем исполняемый файл из двух исходников
add_executable(yadro_monitor 
    src/main.cpp 
    src/monitor.cpp
)

# Линкуем потоки
target_link_libraries(yadro_monitor PRIVATE Threads::Threads)