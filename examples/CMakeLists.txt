add_executable(zk_snark_demo main.cpp)
target_link_libraries(zk_snark_demo PRIVATE zk_snark::zk_snark)
zk_snark_configure_target(zk_snark_demo)
