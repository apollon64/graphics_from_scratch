import qbs

CppApplication {
    consoleApplication: true
    files: [
        "src/array.c",
        "src/array.h",
        "src/camera.c",
        "src/camera.h",
        "src/clip.c",
        "src/clip.h",
        "src/display.c",
        "src/display.h",
        "src/draw_triangle_pikuma.c",
        "src/draw_triangle_pikuma.h",
        "src/draw_triangle_torb.c",
        "src/draw_triangle_torb.h",
        "src/func.c",
        "src/func.h",
        "src/light.c",
        "src/light.h",
        "src/main.c",
        "src/matrix.c",
        "src/matrix.h",
        "src/mesh.c",
        "src/mesh.h",
        "src/misc.c",
        "src/misc.h",
        //"src/plf_nanotimer.h",
        "src/stretchy_buffer.h",
        "src/texture.c",
        "src/texture.h",
        "src/triangle.h",
        "src/typedefs.h",
        "src/upng.c",
        "src/upng.h",
        "src/vector.c",
        "src/vector.h",
        "src/render_font/software_bitmapfont.h",
        "src/render_font/software_bitmapfont.c",


    ]
    cpp.cLanguageVersion: "c99"
    cpp.cFlags: [
        "-Wall",
        "-Wextra", "-Wno-double-promotion", "-Wno-sign-compare",
        //"-fsanitize-undefined-trap-on-error", "-fsanitize=undefined", "-fsanitize=bounds"
        //-fsanitize=memory"
    ]


    cpp.includePaths: ["C:/dev/SDL2-2.0.14/x86_64-w64-mingw32/include/"]
    cpp.libraryPaths: ["C:/dev/SDL2-2.0.14/x86_64-w64-mingw32/lib/"]
    cpp.staticLibraries: ["m", "mingw32", "SDL2main", "SDL2", "m"] //pthread

    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }

}
