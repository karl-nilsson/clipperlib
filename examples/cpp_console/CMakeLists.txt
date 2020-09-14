project(clipper_console)

add_executable(${PROJECT_NAME} clipper_console_demo.cpp)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    clipper::clipper
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT ${PROJECT_NAME}_Runtime
)
