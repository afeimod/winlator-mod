cmake_minimum_required(VERSION 3.31.5)

project(MIDIHandler)

find_package(OpenMP REQUIRED)

add_library(midihandler SHARED
            midihandler.c)

target_include_directories(midihandler PRIVATE fluidsynth/include)

target_link_libraries(midihandler
                      libFLAC
                      libfluidsynth-assetloader
                      libgio-2.0
                      libglib-2.0
                      libgmodule-2.0
                      libgobject-2.0
                      libgthread-2.0
                      libinstpatch-1.0
                      liboboe
                      libogg
                      libopus
                      libpcre
                      libpcreposix
                      libsndfile
                      libvorbis
                      libvorbisenc
                      libvorbisfile
                      libfluidsynth
                      OpenMP::OpenMP_CXX
                      log)