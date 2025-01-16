
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        label_view.cpp
        label_view.h
        mainwindow.ui
        label_abstract_item.h
        label_abstract_item.cpp
)

qt_add_executable(xAnnotationApp
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    resources.qrc
    label_item_rect.h label_item_rect.cpp
    label_control_point.h label_control_point.cpp
    label_item_poly.h label_item_poly.cpp
)


target_link_libraries(xAnnotationApp
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE Qt6::Quick
)

#
target_link_libraries(xAnnotationApp PUBLIC ${FFMPEG_LIBRARIES})


set_target_properties(xAnnotationApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS xAnnotationApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(xAnnotationApp)
endif()
