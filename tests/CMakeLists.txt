cmake_minimum_required(VERSION 3.16)
project(QK4FrequencyEntryTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

qt_add_executable(test_frequencyentryparser
    test_frequencyentryparser.cpp
    ../src/ui/frequencyentryparser.cpp
    ../src/ui/frequencyentryparser.h
)
target_include_directories(test_frequencyentryparser PRIVATE ../src)
target_link_libraries(test_frequencyentryparser PRIVATE Qt6::Core Qt6::Test)

enable_testing()
add_test(NAME frequencyentryparser COMMAND test_frequencyentryparser)

qt_add_executable(test_radiostate
    test_radiostate.cpp
    ../src/models/radiostate.cpp
    ../src/models/radiostate.h
)
target_include_directories(test_radiostate PRIVATE ../src)
target_link_libraries(test_radiostate PRIVATE Qt6::Core Qt6::Test)
add_test(NAME radiostate COMMAND test_radiostate)

qt_add_executable(test_modepopupwidget
    test_modepopupwidget.cpp
    ../src/ui/modepopupwidget.cpp
    ../src/ui/modepopupwidget.h
    ../src/ui/k4popupbase.cpp
    ../src/ui/k4popupbase.h
    ../src/ui/k4styles.cpp
    ../src/ui/k4styles.h
)
target_include_directories(test_modepopupwidget PRIVATE ../src)
target_link_libraries(test_modepopupwidget PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test)
add_test(NAME modepopupwidget COMMAND test_modepopupwidget)

qt_add_executable(test_frequencydisplaywidget
    test_frequencydisplaywidget.cpp
    ../src/ui/frequencydisplaywidget.cpp
    ../src/ui/frequencydisplaywidget.h
    ../src/ui/wheelaccumulator.cpp
    ../src/ui/wheelaccumulator.h
    ../src/ui/k4styles.cpp
    ../src/ui/k4styles.h
)
target_include_directories(test_frequencydisplaywidget PRIVATE ../src)
target_link_libraries(test_frequencydisplaywidget PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test)
add_test(NAME frequencydisplaywidget COMMAND test_frequencydisplaywidget)

qt_add_executable(test_sstvencoder
    test_sstvencoder.cpp
    ../src/sstv/sstvmoderegistry.cpp
    ../src/sstv/sstvmoderegistry.h
    ../src/sstv/sstvencoder.cpp
    ../src/sstv/sstvencoder.h
    ../src/sstv/sstvdecoder.cpp
    ../src/sstv/sstvdecoder.h
)
target_include_directories(test_sstvencoder PRIVATE ../src)
target_link_libraries(test_sstvencoder PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
add_test(NAME sstvencoder COMMAND test_sstvencoder)

qt_add_executable(test_sstvcomposer
    test_sstvcomposer.cpp
    ../src/ui/sstvcomposercanvas.cpp
    ../src/ui/sstvcomposercanvas.h
)
target_include_directories(test_sstvcomposer PRIVATE ../src)
target_link_libraries(test_sstvcomposer PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test)
add_test(NAME sstvcomposer COMMAND test_sstvcomposer)

qt_add_executable(test_sstvstorage
    test_sstvstorage.cpp
    ../src/sstv/sstvstorage.cpp
    ../src/sstv/sstvstorage.h
)
target_include_directories(test_sstvstorage PRIVATE ../src)
target_link_libraries(test_sstvstorage PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
add_test(NAME sstvstorage COMMAND test_sstvstorage)

cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH compiler_runtime_dir)
set_tests_properties(frequencyentryparser frequencydisplaywidget radiostate modepopupwidget sstvencoder sstvcomposer sstvstorage PROPERTIES
    ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
        "PATH=path_list_prepend:${compiler_runtime_dir}"
        "QT_QPA_PLATFORM=set:offscreen"
)
