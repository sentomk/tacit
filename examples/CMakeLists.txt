add_executable(tacit_demo main.cpp)
target_link_libraries(tacit_demo PRIVATE tacit::tacit)
tacit_configure_target(tacit_demo)
