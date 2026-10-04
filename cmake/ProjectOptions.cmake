function(zk_snark_configure_target target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
        if(ZK_SNARK_ENABLE_SANITIZERS)
            target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE
                -fsanitize=address,undefined)
        endif()
    elseif(ZK_SNARK_ENABLE_SANITIZERS)
        message(FATAL_ERROR "Sanitizer preset requires GCC or Clang")
    endif()
endfunction()
