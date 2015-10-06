cmake_minimum_required(VERSION 3.1)

set(MCU atmega2560)
#set(MCU atmega328p)
set(F_CPU 16000000UL)

set(TARGET Example03)

set(CPP_FLAGS 
	-DF_CPU=${F_CPU} 
	-DBAUD=9600
)
string(REPLACE ";" " " CPP_FLAGS "${CPP_FLAGS}")

set(C_FLAGS 
    -pipe
	-gdwarf-2 -g2
	-mmcu=${MCU}
	-fshort-enums
	-fno-inline-small-functions
	-fpack-struct
	-Wall
	-fno-strict-aliasing
	-funsigned-char
	-funsigned-bitfields
	-ffunction-sections
	-mrelax
	-fno-jump-tables
	-x c
	-Os
	-std=gnu99
	-Wstrict-prototypes
	-MMD -MP
)
string(REPLACE ";" " " C_FLAGS "${C_FLAGS}")

set(AVRLIB $ENV{AVR_COMMON}/avrlib)
set(FreeRTOS $ENV{AVR_COMMON}/FreeRTOSv8.2.2)
include_directories(${CMAKE_SOURCE_DIR} ${AVRLIB} ${FreeRTOS})

set(LD_FLAGS 
	-lm
	-Wl,-Map,${TARGET}.map,--cref 
	-Wl,--gc-sections
	-Wl,--relax
	-mmcu=${MCU} 
)
string(REPLACE ";" " " LD_FLAGS "${LD_FLAGS}")

set(SRCS 
	${TARGET}.c 
	${AVRLIB}/uartio.c
	${FreeRTOS}/tasks.c
	${FreeRTOS}/queue.c
	${FreeRTOS}/list.c
	${FreeRTOS}/croutine.c
	${FreeRTOS}/heap_1.c
	${FreeRTOS}/port.c
)
set_source_files_properties(${SRCS} PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ${C_FLAGS}")

add_executable(${TARGET}.elf ${SRCS})
set(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS}")

add_custom_target(${TARGET} ALL
	COMMAND ${OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${TARGET}.elf ${TARGET}.hex
	COMMAND ${OBJCOPY} -O ihex -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings ${TARGET}.elf ${TARGET}.eep
	COMMAND ${OBJCOPY} -O binary -R .eeprom -R .fuse -R .lock -R .signature ${TARGET}.elf ${TARGET}.bin
	COMMAND ${OBJDUMP} -h -d -S -z ${TARGET}.elf > ${TARGET}.lss
	COMMAND ${NM} -n ${TARGET}.elf > ${TARGET}.sym
	COMMAND ${SIZE} -C --mcu=${MCU} ${TARGET}.elf
	DEPENDS ${TARGET}.elf
)

set(PROG_TYPE wiring)
set(PROG_ARGS -P /dev/ttyACM0 -b 115200)
#set(PROG_TYPE arduino)
#set(PROG_ARGS -P /dev/ttyUSB0 -b 115200)

add_custom_target(flash
	 COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} ${PROG_ARGS} -D -U flash:w:${TARGET}.hex
	 DEPENDS ${TARGET}
)
