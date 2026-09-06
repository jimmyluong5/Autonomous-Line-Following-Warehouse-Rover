FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.c)

idf_component_register(
    SRCS ${app_sources}
    EMBED_FILES 
        "images/frame1.jpg"
        "images/frame2.jpg"
        "images/frame3.jpg"
        "images/frame4.jpg"
        "images/frame5.jpg"
        "images/frame6.jpg"
        "images/frame7.jpg"
        "images/frame8.jpg"
        "images/frame9.jpg"
        "images/frame10.jpg"
        "images/frame11.jpg"
        "images/frame12.jpg"
        "images/frame13.jpg"
        "images/frame14.jpg"
        "images/frame15.jpg"
        "images/left.jpg"
        "images/leftright.jpg"
        "images/linkedin.jpg"
        "images/github.jpg"
)
