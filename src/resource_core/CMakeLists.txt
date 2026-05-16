# Библиотека resource_core
add_library(resource_core STATIC
        resource_core.cpp
)

# Подключаем предупреждения (если есть такой таргет в корневом CMake)
target_link_libraries(resource_core PRIVATE project_warnings)

# Подключаем санитайзеры (если есть)
if (TARGET project_sanitizers)
        target_link_libraries(resource_core PRIVATE project_sanitizers)
endif ()

# Указываем пути к заголовочным файлам
target_include_directories(resource_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}  # чтобы можно было писать #include "resource_core.hpp"
)

# Устанавливаем стандарт C++
set_target_properties(resource_core PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)