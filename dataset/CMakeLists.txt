include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
set(DATASET_HEADER ${CMAKE_CURRENT_LIST_DIR}/include/dataset.hpp)

add_library(dataset SHARED src/dataset.cpp ${DATASET_HEADER})

if (BUILD_IOS)
set_target_properties(dataset PROPERTIES FRAMEWORK TRUE)
set_target_properties(dataset PROPERTIES
    MACOSX_FRAMEWORK_IDENTIFIER com.hzx.llm.dataset
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PACKAGE_VERSION}
    MACOSX_FRAMEWORK_BUNDLE_VERSION ${PACKAGE_VERSION}
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
)
set_target_properties(MNN PROPERTIES MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/../MNN/project/ios/MNN/Info.plist)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_LIST_DIR}/include/dataset.hpp PROPERTIES MACOSX_PACKAGE_LOCATION Headers/)
endif()