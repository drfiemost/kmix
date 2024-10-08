cmake_minimum_required(VERSION 3.5)
project(kmix)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options("$<$<CONFIG:DEBUG>:-Wall;-Wextra;-pedantic>")
endif()

set(CMAKE_AUTOMOC ON)
set_property(SOURCE controladaptor.h
                    mixeradaptor.h
                    mixsetadaptor.h PROPERTY SKIP_AUTOMOC ON)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})


# Do not yet REQUIRE Phonon. Hint: As long as we do not find_package(), ${KDE4_PHONON_LIBS} will be empty below, but that should not hurt.
#find_package(Phonon REQUIRED)

set(FPHSA_NAME_MISMATCHED TRUE)
set(PA_VER "0.9.16")
macro_optional_find_package(PulseAudio "${PA_VER}")
macro_log_feature(PULSEAUDIO_FOUND "PulseAudio" "PulseAudio Audio Server"
    "http://www.pulseaudio.org/" FALSE "${PA_VER}" "libpulse is needed to let KMix control PulseAudio")

find_package(GLIB2)

macro_optional_find_package(Canberra)
macro_log_feature(CANBERRA_FOUND "libcanberra" "libcanberra audio library"
    "http://0pointer.de/lennart/projects/libcanberra/" FALSE "" "libcanberra is needed for kmix sound feedback")

find_package(Alsa)
if(ALSA_FOUND)
	alsa_configure_file(${CMAKE_BINARY_DIR}/config-alsa.h)
	add_definitions(-DHAVE_LIBASOUND2)
endif(ALSA_FOUND)

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS} )
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=67100)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
if(MSVC)
    include_directories( ${TAGLIB_INCLUDES} )
endif(MSVC)


include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
    #include <memory>
    int main() { std::shared_ptr<int> p; return 0; }
" HAVE_STD_SHARED_PTR)
check_cxx_source_compiles("
    #include <tr1/memory>
    int main() { std::tr1::shared_ptr<int> p; return 0; }
" HAVE_STD_TR1_SHARED_PTR)


configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


include_directories( ${GLIB2_INCLUDE_DIR} /usr/lib/oss/include)

add_subdirectory( doc )
add_subdirectory( pics ) 
add_subdirectory( profiles ) 
#add_subdirectory( tests )

if (PULSEAUDIO_FOUND)
  add_definitions(-DHAVE_PULSE)

  include_directories(${PULSEAUDIO_INCLUDE_DIR})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
  add_definitions(-DHAVE_CANBERRA)

  include_directories(${CANBERRA_INCLUDE_DIRS})
endif (CANBERRA_FOUND)


####################################################################################################
########### definitions: DBUSADAPTOR ###############################################################
####################################################################################################
set(kmix_adaptor_SRCS
	dbus/dbusmixerwrapper.cpp
	dbus/dbusmixsetwrapper.cpp
	dbus/dbuscontrolwrapper.cpp)

qt4_add_dbus_adaptor( kmix_adaptor_SRCS dbus/org.kde.kmix.control.xml
	dbus/dbuscontrolwrapper.h DBusControlWrapper )
qt4_add_dbus_adaptor( kmix_adaptor_SRCS dbus/org.kde.kmix.mixer.xml
	dbus/dbusmixerwrapper.h DBusMixerWrapper )
qt4_add_dbus_adaptor( kmix_adaptor_SRCS dbus/org.kde.kmix.mixset.xml
	dbus/dbusmixsetwrapper.h DBusMixSetWrapper )


####################################################################################################
########### definitions: backends ##################################################################
####################################################################################################
set(kmix_backend_SRCS
   backends/mixer_backend.cpp
   backends/mixer_mpris2.cpp
   )

if (HAVE_LIBASOUND2)
  set(kmix_backend_SRCS ${kmix_backend_SRCS}
      backends/mixer_alsa9.cpp )
endif (HAVE_LIBASOUND2)

if (PULSEAUDIO_FOUND)
  set(kmix_backend_SRCS ${kmix_backend_SRCS}
      backends/mixer_pulse.cpp )
endif (PULSEAUDIO_FOUND)


set(CMAKE_AUTOMOC ON)


####################################################################################################
########### next target : kmix #####################################################################
####################################################################################################

set( kmix_OSD_SRCS gui/osdwidget.cpp )

set(kmix_KDEINIT_SRCS ${kmix_adaptor_SRCS} ${kmix_backend_SRCS}
   apps/main.cpp 
   apps/kmix.cpp 
   apps/KMixApp.cpp
   gui/kmixdockwidget.cpp 
   gui/kmixprefdlg.cpp 
   gui/viewbase.cpp 
   gui/viewdockareapopup.cpp 
   gui/viewsliders.cpp 
   gui/mixdevicewidget.cpp 
   gui/mdwmoveaction.cpp
   gui/mdwslider.cpp 
   gui/mdwenum.cpp 
   gui/kmixerwidget.cpp 
   gui/ksmallslider.cpp
   gui/verticaltext.cpp
   gui/volumeslider.cpp 
   gui/kmixtoolbox.cpp 
   gui/dialogaddview.cpp 
   gui/dialogviewconfiguration.cpp 
   gui/dialogselectmaster.cpp 
   gui/dialogchoosebackends.cpp 
   gui/guiprofile.cpp
   ${kmix_OSD_SRCS}
   core/MediaController.cpp
   core/mixertoolbox.cpp
   core/kmixdevicemanager.cpp
   core/ControlManager.cpp
#   core/ControlPool.cpp
   core/GlobalConfig.cpp
   core/MasterControl.cpp
   core/mixer.cpp
   core/mixset.cpp
   core/mixdevice.cpp
   core/mixdevicecomposite.cpp
   core/volume.cpp
   )

kde4_add_kdeinit_executable( kmix ${kmix_KDEINIT_SRCS})
  target_link_libraries(kdeinit_kmix ${KDE4_SOLID_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${QT_QTXML_LIBRARY} ${KDE4_PHONON_LIBS})


if (HAVE_LIBASOUND2)
    target_link_libraries(kdeinit_kmix ${ASOUND_LIBRARY})
endif (HAVE_LIBASOUND2)

if (PULSEAUDIO_FOUND)
    target_link_libraries(kdeinit_kmix ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY} ${GLIB2_LIBRARIES})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
  target_link_libraries(kdeinit_kmix ${CANBERRA_LIBRARIES})
endif (CANBERRA_FOUND)

install(TARGETS kdeinit_kmix  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmix kdeinit_kmix )
install(TARGETS kmix  ${INSTALL_TARGETS_DEFAULT_ARGS} )




####################################################################################################
########### next target : kmixd ####################################################################
####################################################################################################

 set(kded_kmixd_SRCS ${kmix_adaptor_SRCS} ${kmix_backend_SRCS}
    apps/kmixd.cpp 
    core/ControlManager.cpp
#    core/ControlPool.cpp
    core/GlobalConfig.cpp
    core/MasterControl.cpp
    core/MediaController.cpp
    core/mixer.cpp 
    core/mixset.cpp 
    core/mixdevice.cpp 
    core/volume.cpp
    core/mixertoolbox.cpp 
    core/kmixdevicemanager.cpp
    )

#qt4_add_dbus_adaptor(kded_kmixd_SRCS org.kde.KMixD.xml kmixd.h Mixer) 

kde4_add_plugin(kded_kmixd ${kded_kmixd_SRCS})

target_link_libraries(kded_kmixd ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTXML_LIBRARY} ${KDE4_PHONON_LIBS})


if (HAVE_LIBASOUND2)
    target_link_libraries(kded_kmixd ${ASOUND_LIBRARY})
endif (HAVE_LIBASOUND2)

if (PULSEAUDIO_FOUND)
    target_link_libraries(kded_kmixd ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY} ${GLIB2_LIBRARIES})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
  target_link_libraries(kded_kmixd ${CANBERRA_LIBRARIES})
endif (CANBERRA_FOUND)

install(TARGETS kded_kmixd DESTINATION ${PLUGIN_INSTALL_DIR})

#target_link_libraries( kmixd kded_kmixd )
#install(TARGETS kmixd DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kmixd.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded )



####################################################################################################
########### next target : kmixctrl #################################################################
####################################################################################################

set(kmixctrl_KDEINIT_SRCS ${kmix_adaptor_SRCS} ${kmix_backend_SRCS}
   apps/kmixctrl.cpp 
   core/ControlManager.cpp
#   core/ControlPool.cpp
   core/GlobalConfig.cpp
   core/MasterControl.cpp
   core/MediaController.cpp
   core/mixer.cpp 
   core/mixset.cpp 
   core/mixdevice.cpp 
   core/volume.cpp 
   core/mixertoolbox.cpp 
   # There is no actual need for kmixdevicemanager.cpp (hotplug makes no sense in kmixctrl), but
   # the dependency comes in via mixertoolbox.cpp
   core/kmixdevicemanager.cpp
   )

# gui/guiprofile.cpp


kde4_add_kdeinit_executable( kmixctrl ${kmixctrl_KDEINIT_SRCS})
target_link_libraries(kdeinit_kmixctrl ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTXML_LIBRARY} ${KDE4_PHONON_LIBS})

target_link_libraries(kdeinit_kmixctrl ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTXML_LIBRARY} ${KDE4_PHONON_LIBS})

if (HAVE_LIBASOUND2)
    target_link_libraries(kdeinit_kmixctrl ${ASOUND_LIBRARY})
endif (HAVE_LIBASOUND2)

if (PULSEAUDIO_FOUND)
    target_link_libraries(kdeinit_kmixctrl ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY} ${GLIB2_LIBRARIES})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
  target_link_libraries(kdeinit_kmixctrl ${CANBERRA_LIBRARIES})
endif (CANBERRA_FOUND)

########### next target ###############
add_subdirectory( plasma )



####################################################################################################
########### INSTALL RULES ##########################################################################
####################################################################################################
install( TARGETS kdeinit_kmixctrl  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kmixctrl kdeinit_kmixctrl )
install( TARGETS kmixctrl ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( PROGRAMS kmix.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( PROGRAMS apps/kmixremote DESTINATION ${BIN_INSTALL_DIR} )
install( FILES restore_kmix_volumes.desktop  DESTINATION ${AUTOSTART_INSTALL_DIR})
install( FILES kmix_autostart.desktop  DESTINATION ${AUTOSTART_INSTALL_DIR})
install( FILES kmixui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmix )
install( FILES kmixctrl_restore.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES dbus/org.kde.kmix.control.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
install( FILES dbus/org.kde.kmix.mixer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
install( FILES dbus/org.kde.kmix.mixset.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

macro_display_feature_log()
