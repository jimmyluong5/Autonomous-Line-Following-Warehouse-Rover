FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.c)

idf_component_register(
    SRCS ${app_sources}
    EMBED_FILES 
        "frame1.jpg"
        "frame2.jpg"
        "frame3.jpg"
        "frame4.jpg"
        "frame5.jpg"
        "frame6.jpg"
        "frame7.jpg"
        "frame8.jpg"
        "frame9.jpg"
        "frame10.jpg"
        "frame11.jpg"
        "frame12.jpg"
        "frame13.jpg"
        "frame14.jpg"
        "frame15.jpg"
)
