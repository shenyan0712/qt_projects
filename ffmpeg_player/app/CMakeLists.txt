
## FFMPEG library
set(FFMPEG_DIR C:/ffmpeg-n7.1-latest-win64-lgpl-shared-7.1)
find_package(FFmpeg REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIRS})
link_directories(${FFMPEG_LIBRARY_DIRS})

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}

    ffmpeg_decoder.h ffmpeg_decoder.cpp
    video_disp_widget.h video_disp_widget.cpp
    ffmpeg_frame.h ffmpeg_frame.cpp
    media_player.h media_player.cpp
    resources.qrc


    abstract_video_decoder.h
    pcm_play_device.h pcm_play_device.cpp
    ffmpeg_decoder_inner.h ffmpeg_decoder_inner.cpp

    ffmpeg_utils.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(${PROJECT_NAME} PUBLIC ${FFMPEG_LIBRARIES} swscale avutil swresample)
target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES})


set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
