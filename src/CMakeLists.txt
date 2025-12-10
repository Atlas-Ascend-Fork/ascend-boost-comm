file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/*/*.cpp")

# Add memset kernel to libmki
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/utils/memset/kernel)

set_source_files_properties(${BINARY_SRC_LIST} PROPERTIES GENERATED TRUE)
add_library(mki_static STATIC ${SOURCE_FILES} ${BINARY_SRC_LIST})
add_dependencies(mki_static ${BINARY_TARGET_LIST})
set(COMMON_LIBS dl pthread c_sec mmpa runtime ascendcl nnopbase)
target_link_libraries(mki_static PUBLIC dl pthread c_sec mmpa runtime ascendcl nnopbase)
add_library(mki SHARED ${SOURCE_FILES} ${BINARY_SRC_LIST})
add_dependencies(mki ${BINARY_TARGET_LIST})
target_link_libraries(mki PUBLIC dl pthread c_sec mmpa profapi runtime error_manager ascendcl nnopbase)
if(USE_ASCENDC_DUMP)
    target_link_libraries(mki_static PRIVATE ascend_dump)
    target_link_libraries(mki PRIVATE ascend_dump)
endif()
add_executable(sym_check ${CMAKE_CURRENT_LIST_DIR}/sym_check.cpp)
target_link_libraries(sym_check PRIVATE mki)
include_directories(${ASCEND_HOME_PATH}/include)

# install libs and includes
install(TARGETS mki_static DESTINATION lib)
install(TARGETS mki DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include DESTINATION .)
