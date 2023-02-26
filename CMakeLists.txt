cmake_minimum_required(VERSION 3.20)

set (PROJECT_NAME Minichat)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/Source/src)
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp)
	
# Add header files
set(HEADER_DIR ${PROJECT_SOURCE_DIR}/Source/include)
file(GLOB_RECURSE HEADER_FILES ${HEADER_DIR}/*.h)

add_executable(${PROJECT_NAME}
				${SOURCE_FILES}
				${HEADER_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_RELEASE_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set(DEPENDS_DIR ${PROJECT_SOURCE_DIR}/Dependencies)

#add GLAD
target_include_directories(${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/GLAD/include)
target_sources(${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/GLAD/src/glad.cpp)
source_group("GLAD" FILES ${DEPENDS_DIR}/GLAD/src/glad.cpp)
				
#add GLFW
# We need a CMAKE_DIR with some code to find external dependencies
set(GLFW3_ROOT ${DEPENDS_DIR}/GLFW)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMake)
find_package(GLFW3 REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW3_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GLFW3_LIBRARY})

#add Bullet
set(BULLET_ROOT ${DEPENDS_DIR}/Bullet/)
find_package(Bullet REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC 
	debug ${BULLET_COLLISION_LIBRARY_DEBUG}
	debug ${BULLET_LINEAR_MATH_DEBUG}
	debug ${BULLET_DYNAMICS_LIBRARY_DEBUG}
	optimized ${BULLET_COLLISION_LIBRARY}
	optimized ${BULLET_LINEAR_MATH}
	optimized ${BULLET_DYNAMICS_LIBRARY}
)


#add assimp
set(ASSIMP_ROOT ${DEPENDS_DIR}/Assimp/)
find_package(Assimp REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${_assimp_HEADER_SEARCH_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC 
	debug ${ASSIMP_LIBRARY_DEBUG}
	optimized ${ASSIMP_LIBRARY}
)

#add RapidJson
target_include_directories(${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/RapidJson)

#add IrrKlang
target_include_directories(${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/IrrKlang/include)
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/IrrKlang/lib/irrKlang.lib)

#add STB_image
target_include_directories(${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/STB)

#add IM_GUI
target_include_directories(${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/IM_GUI)
file(GLOB_RECURSE IM_GUI_SRC ${PROJECT_NAME} PUBLIC ${DEPENDS_DIR}/IM_GUI/*.cpp)
target_sources(${PROJECT_NAME} PUBLIC ${IM_GUI_SRC})
source_group("ImGui" FILES ${IM_GUI_SRC})

#add lib math
set(MATH_DIR ${PROJECT_SOURCE_DIR}/maths/LibMaths)
file(GLOB_RECURSE MATH_HEADER ${MATH_DIR}/*.cpp ${MATH_DIR}/*.h)
add_library(LibMaths STATIC EXCLUDE_FROM_ALL ${MATH_HEADER})
target_include_directories(LibMaths PRIVATE ${MATH_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${MATH_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${MATH_LIB})

#keep folder structure
macro(GroupFolders curdir)
file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/Source/${curdir} ${PROJECT_SOURCE_DIR}/Source/${curdir}/*)
	foreach(child ${children})
        if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/Source/${curdir}/${child})
			GroupFolders(${curdir}/${child})
		else()
			string(REPLACE "/" "\\" groupname ${curdir})
			string(REPLACE "src" "Source Files" groupname ${groupname})
			source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/Source/${curdir}/${child})
        endif()
    endforeach()
endmacro()

# execute macro
GroupFolders(src)
GroupFolders(include)

set(ASSIMP_DLL ${DEPENDS_DIR}/Assimp/lib/assimp-vc142-mt.dll)

#if (WIN32)
# copy the .dll file to the same folder as the executable
#add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_files
#    $<TARGET_FILE_DIR:${ASSIMP_DLL}>
#    $<TARGET_FILE_DIR:${PROJECT_NAME})
#endif()