cmake_minimum_required(VERSION 3.9)

project(it_irlm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    it_irlm.cpp
    irlm.h
)


#target_include_directories(${PROJECT_NAME} PRIVATE /home/yurielnf/opt/ITensor)
#target_link_libraries(${PROJECT_NAME} PRIVATE /home/yurielnf/opt/ITensor/lib/libitensor.a)

add_library(itensor STATIC IMPORTED) # or STATIC instead of SHARED
set_target_properties(itensor PROPERTIES
  IMPORTED_LOCATION_RELEASE "/home/yurielnf/opt/ITensor/lib/libitensor.a"
  IMPORTED_LOCATION_DEBUG "/home/yurielnf/opt/ITensor/lib/libitensor-g.a"
  INTERFACE_INCLUDE_DIRECTORIES "/home/yurielnf/opt/ITensor"
)

find_package(OpenMP REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE itensor openblas OpenMP::OpenMP_CXX)

find_package(Armadillo REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ARMADILLO_LIBRARIES})

