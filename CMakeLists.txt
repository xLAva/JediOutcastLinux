#
# Build procedure:
#                $ mkdir some_build_dir
#                $ cd some_build_dir
#                $ cmake PATH_TO_JK2_SOURCE_DIR
#                $ make
#
# Make sure some_build_dir is different from PATH_TO_JK2_SOURCE_DIR
#

cmake_minimum_required(VERSION 2.6)

project(jk2)

set(USE_SDL2 1)

set(PLATFORM_WIN32 0)
set(PLATFORM_MACOS 0)
set(PLATFORM_LINUX 0)

message("system name: ${CMAKE_SYSTEM_NAME}")

if(WIN32)
    set(PLATFORM_WIN32 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(PLATFORM_MACOS 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(PLATFORM_LINUX 1)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif()

if(NOT DEFINED EXTERNAL_LIB_PATH)
    set(EXTERNAL_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty")
endif()

message("Detected platform: Win32=${PLATFORM_WIN32} MacOS=${PLATFORM_MACOS} Linux=${PLATFORM_LINUX}")

# msvc: Debug or Release is selected inside VisualStudio, so don't show the CMAKE_BUILD_TYPE
if(NOT MSVC)
    message("Build Type: ${CMAKE_BUILD_TYPE}")
endif()



file(GLOB SRC_CGAME ${PROJECT_SOURCE_DIR}/code/cgame/*.cpp ${PROJECT_SOURCE_DIR}/code/cgame/*.h)
file(GLOB SRC_GAME ${PROJECT_SOURCE_DIR}/code/game/*.cpp ${PROJECT_SOURCE_DIR}/code/game/*.h ${PROJECT_SOURCE_DIR}/code/game/*.def)
list(REMOVE_ITEM SRC_GAME ${PROJECT_SOURCE_DIR}/code/game/bg_lib.cpp)

set(SRC_ICARUS
	code/icarus/BlockStream.cpp
	code/icarus/Instance.cpp
	code/icarus/Sequence.cpp
	code/icarus/Sequencer.cpp
	code/icarus/TaskManager.cpp
	code/icarus/BlockStream.h
	code/icarus/Instance.h
	code/icarus/Sequence.h
	code/icarus/Sequencer.h
	code/icarus/TaskManager.h
)

source_group(cgame FILES ${SRC_CGAME})
source_group(game FILES ${SRC_GAME})
source_group(icarus FILES ${SRC_ICARUS})

set(GAME_SOURCE_FILES
	code/qcommon/tri_coll_test.cpp
	code/ui/gameinfo.cpp
	${SRC_CGAME}
	${SRC_GAME}
	${SRC_ICARUS}
)


file(GLOB SRC_CLIENT ${PROJECT_SOURCE_DIR}/code/client/*.cpp ${PROJECT_SOURCE_DIR}/code/client/*.h)
file(GLOB SRC_ENCRYPTION ${PROJECT_SOURCE_DIR}/code/encryption/*.cpp ${PROJECT_SOURCE_DIR}/code/encryption/*.h)
file(GLOB SRC_GHOUL2 ${PROJECT_SOURCE_DIR}/code/ghoul2/*.cpp ${PROJECT_SOURCE_DIR}/code/ghoul2/*.h)
file(GLOB SRC_JPEG6 ${PROJECT_SOURCE_DIR}/code/jpeg-6/*.cpp ${PROJECT_SOURCE_DIR}/code/jpeg-6/*.h)
file(GLOB SRC_MP3CODE ${PROJECT_SOURCE_DIR}/code/mp3code/*.c ${PROJECT_SOURCE_DIR}/code/mp3code/*.h)
file(GLOB SRC_QCOMMON ${PROJECT_SOURCE_DIR}/code/qcommon/*.cpp ${PROJECT_SOURCE_DIR}/code/qcommon/*.h)
file(GLOB SRC_RENDERER ${PROJECT_SOURCE_DIR}/code/renderer/*.cpp ${PROJECT_SOURCE_DIR}/code/renderer/*.c ${PROJECT_SOURCE_DIR}/code/renderer/*.h)
list(REMOVE_ITEM SRC_RENDERER ${PROJECT_SOURCE_DIR}/code/renderer/tr_flares.cpp)
file(GLOB SRC_SERVER ${PROJECT_SOURCE_DIR}/code/server/*.cpp ${PROJECT_SOURCE_DIR}/code/server/*.h)
file(GLOB SRC_UI ${PROJECT_SOURCE_DIR}/code/ui/*.cpp ${PROJECT_SOURCE_DIR}/code/ui/*.h)
list(REMOVE_ITEM SRC_UI ${PROJECT_SOURCE_DIR}/code/ui/gameinfo.cpp)


source_group(client FILES ${SRC_CLIENT})
source_group(encryption FILES ${SRC_ENCRYPTION})
source_group(ghoul2 FILES ${SRC_GHOUL2})
source_group(jpeg-6 FILES ${SRC_JPEG6})
source_group(mp3code FILES ${SRC_MP3CODE})
source_group(qcommon FILES ${SRC_QCOMMON})
source_group(renderer FILES ${SRC_RENDERER})
source_group(mp3code FILES ${SRC_SERVER})
source_group(server FILES ${SRC_GHOUL2})
source_group(ui FILES ${SRC_UI})



set(STARWARS_SOURCE_FILES
	code/game/genericparser2.cpp
	code/game/q_math.cpp
	code/game/q_shared.cpp
	${SRC_CLIENT}
	${SRC_ENCRYPTION}
	${SRC_GHOUL2}
	${SRC_JPEG6}
	${SRC_MP3CODE}
	${SRC_QCOMMON}
	${SRC_RENDERER}
	${SRC_SERVER}
	${SRC_UI}
)



# now handle platfom specific stuff
if(USE_SDL2)

    # platform independend sdl stuff
    set(SSF_SDL2 ${SSF_SDL2}
        ${PROJECT_SOURCE_DIR}/code/sdl/sdl_local.h
        ${PROJECT_SOURCE_DIR}/code/sdl/sdl_glw.h
        ${PROJECT_SOURCE_DIR}/code/sdl/sdl_gamma.cpp
        ${PROJECT_SOURCE_DIR}/code/sdl/sdl_glimp.cpp
        ${PROJECT_SOURCE_DIR}/code/sdl/sdl_qgl.cpp
        ${PROJECT_SOURCE_DIR}/code/sdl/sdl_snddma.cpp
    )

    source_group(sdl FILES ${SSF_SDL2})
    set(STARWARS_SOURCE_FILES ${STARWARS_SOURCE_FILES} ${SSF_SDL2})

    set(CUSTOM_COMPILE_FLAGS_ALL "${CUSTOM_COMPILE_FLAGS_ALL} -DUSE_SDL2")

    if(PLATFORM_WIN32)

        set(SSF_WIN32 ${SSF_WIN32}
            ${PROJECT_SOURCE_DIR}/code/win32/win_main.cpp
            ${PROJECT_SOURCE_DIR}/code/win32/win_shared.cpp
            ${PROJECT_SOURCE_DIR}/code/win32/win_syscon.cpp
        )

        include_directories("${EXTERNAL_LIB_PATH}/SDL2-2.0.3/include")

        add_library(imp_sdl2 STATIC IMPORTED)
        set_property(TARGET imp_sdl2 PROPERTY IMPORTED_LOCATION "${EXTERNAL_LIB_PATH}/SDL2-2.0.3/lib/x86/SDL2.lib")

        set(EXTRA_LIBS ${EXTRA_LIBS} imp_sdl2)
    elseif(PLATFORM_MACOS)
        # mac sdl

        set(SSF_MACOS ${SSF_MACOS}
            ${PROJECT_SOURCE_DIR}/code/sdl/sdl_console.cpp
            ${PROJECT_SOURCE_DIR}/code/mac/mac_main_sdl.cpp
        )


        include_directories( /Library/Frameworks )
        FIND_LIBRARY(SDL2_LIBRARY SDL2)
        MARK_AS_ADVANCED (SDL2_LIBRARY)
        set(EXTRA_LIBS ${EXTRA_LIBS} ${SDL2_LIBRARY})

    else()
        # linux sdl
        
        set(SSF_LINUX ${SSF_LINUX}
            ${PROJECT_SOURCE_DIR}/code/sdl/sdl_console.cpp
            ${PROJECT_SOURCE_DIR}/code/linux/linux_main.cpp
        )

        set(EXTRA_LIBS ${EXTRA_LIBS} SDL2)
    endif()
else()
    if(PLATFORM_WIN32)
        file(GLOB SSF_WIN32 ${PROJECT_SOURCE_DIR}/code/win32/*.cpp ${PROJECT_SOURCE_DIR}/code/win32/*.h)
    elseif(PLATFORM_LINUX)
        file(GLOB SSF_LINUX ${PROJECT_SOURCE_DIR}/code/linux/*.cpp ${PROJECT_SOURCE_DIR}/code/linux/*.h)
    endif()
endif()


# add platform specific files
source_group(win32 FILES ${SSF_WIN32})
source_group(linux FILES ${SSF_LINUX})
source_group(mac FILES ${SSF_MACOS})
set(STARWARS_SOURCE_FILES ${STARWARS_SOURCE_FILES} ${SSF_WIN32} ${SSF_LINUX} ${SSF_MACOS})

if(MSVC)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CUSTOM_COMPILE_FLAGS_ALL} -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CUSTOM_COMPILE_FLAGS_ALL} -DNDEBUG")

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CUSTOM_COMPILE_FLAGS_ALL} -DEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CUSTOM_COMPILE_FLAGS_ALL} -DEBUG")

    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_CXX_FLAGS_RELEASE}")

else()
    set(DISABLE_WARNINGS_C "-Wno-write-strings -Wno-pointer-arith -Wno-multichar -Wno-int-to-pointer-cast -Wno-enum-compare")
    set(DISABLE_WARNINGS_CXX "-Wno-write-strings -Wno-pointer-arith -Wno-multichar -Wno-int-to-pointer-cast -Wno-enum-compare -Wno-conversion-null")

    set(COMPILER_FLAGS_C "-m32 -fexceptions -ffast-math")
    set(COMPILER_FLAGS_CXX "-m32 -fexceptions -ffast-math -fno-operator-names")

    if(PLATFORM_LINUX)
        #set NDEBUG even on debug builds - asserts are killing us otherwise
        set(CUSTOM_COMPILE_FLAGS_ALL "${CUSTOM_COMPILE_FLAGS_ALL} -DLINUX -DNDEBUG")

        set(DISABLE_WARNINGS_C "${DISABLE_WARNINGS_C} -Wno-cpp")
        set(DISABLE_WARNINGS_CXX "${DISABLE_WARNINGS_CXX} -Wno-cpp")

    endif()

	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CUSTOM_COMPILE_FLAGS_ALL} ${DISABLE_WARNINGS_C} ${COMPILER_FLAGS_C}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CUSTOM_COMPILE_FLAGS_ALL} ${DISABLE_WARNINGS_CXX} ${COMPILER_FLAGS_CXX}")

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g ${CUSTOM_COMPILE_FLAGS_ALL} ${DISABLE_WARNINGS_C} ${COMPILER_FLAGS_C}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g ${CUSTOM_COMPILE_FLAGS_ALL} ${DISABLE_WARNINGS_CXX} ${COMPILER_FLAGS_CXX}")
endif()

add_library(jk2gamex86 SHARED ${GAME_SOURCE_FILES})
set_target_properties(jk2gamex86 PROPERTIES PREFIX "")
if (PLATFORM_WIN32)
	target_link_libraries (jk2gamex86 winmm.lib)
endif()

if(PLATFORM_WIN32)
	set (EXECUTABLE_FLAG WIN32)
	set (EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_CURRENT_SOURCE_DIR}/code/OpenAL32.lib)
	set (EXTRA_LIBS ${EXTRA_LIBS} winmm.lib)
	set (EXTRA_LIBS ${EXTRA_LIBS} wsock32.lib)
	set (EXTRA_LIBS ${EXTRA_LIBS} opengl32.lib)
endif()

if(PLATFORM_LINUX)
	set (EXTRA_LIBS ${EXTRA_LIBS} dl)
	set (EXTRA_LIBS ${EXTRA_LIBS} openal)
	set (EXTRA_LIBS ${EXTRA_LIBS} GL)
	set (EXTRA_LIBS ${EXTRA_LIBS} X11)
	set (EXTRA_LIBS ${EXTRA_LIBS} Xxf86vm)
	set (EXTRA_LIBS ${EXTRA_LIBS} Xrandr)
	set (EXTRA_LIBS ${EXTRA_LIBS} rt)
	set (EXTRA_LIBS ${EXTRA_LIBS} pthread)
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-m32")
endif()

add_executable(jk2sp ${EXECUTABLE_FLAG} ${STARWARS_SOURCE_FILES})
target_link_libraries (jk2sp  ${EXTRA_LIBS})
set_target_properties (jk2sp PROPERTIES COMPILE_DEFINITIONS "_JK2EXE;_M_IX86")

if(PLATFORM_WIN32)
	add_custom_command(TARGET jk2gamex86 POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:jk2gamex86>/jk2gamex86.dll" "${PROJECT_SOURCE_DIR}/binary/win32/"
		COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:jk2gamex86>/jk2gamex86.pdb" "${PROJECT_SOURCE_DIR}/binary/win32/"
	)

	set_property(TARGET jk2sp APPEND PROPERTY LINK_FLAGS /stack:0x800000)
	add_custom_command(TARGET jk2sp POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:jk2sp>/jk2sp.exe" "${PROJECT_SOURCE_DIR}/binary/win32/"
		COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:jk2sp>/jk2sp.pdb" "${PROJECT_SOURCE_DIR}/binary/win32/"
	)

	set_target_properties(jk2sp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/binary/win32/")

endif()

