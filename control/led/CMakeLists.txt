set(LIB led)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(LED_SIMULATED_DEFAULT OFF)
else()
	set(LED_SIMULATED_DEFAULT ON)
endif()

option(LED_SIMULATED "Print LED changes instead of using GPIO" ${LED_SIMULATED_DEFAULT})

if(LED_SIMULATED)
	add_library(${LIB} led_simulated.cpp)
else()
	add_library(${LIB} led_gpio.cpp)
endif()
