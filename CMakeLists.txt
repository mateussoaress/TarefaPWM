cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(pwm C CXX ASM)

pico_sdk_init()

add_executable(pwm pwm.c)

pico_set_program_name(pwm "pwm")
pico_set_program_version(pwm "0.1")

# Habilita saída via USB para visualizar printf()
pico_enable_stdio_usb(pwm 1)
pico_enable_stdio_uart(pwm 0)

# Inclui bibliotecas padrão e de hardware PWM
target_link_libraries(pwm
        pico_stdlib
        hardware_pwm
)

target_include_directories(pwm PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(pwm)


