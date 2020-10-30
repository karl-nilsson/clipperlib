project(clipper_cairo)


find_package(Cairo REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${Cairo_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    clipper::clipper
    ${Cairo_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT ${PROJECT_NAME}_Runtime
)
