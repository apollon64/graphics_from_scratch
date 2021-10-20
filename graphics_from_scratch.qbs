import qbs

Project {

CppApplication {
    name: "graphics_from_scratch"
    consoleApplication: true
    Depends { name: "PikumaSoft" }

    files: [
        "src/main.c",
        "src/display.c",
        "src/display.h",
    ]
    cpp.cLanguageVersion: "c99"
    cpp.cFlags: [
        "-Wall",
        "-Wextra", "-Wno-double-promotion", "-Wno-sign-compare",
        //"-fsanitize-undefined-trap-on-error", "-fsanitize=undefined", "-fsanitize=bounds"
        //-fsanitize=memory"
    ]

    cpp.staticLibraries: ["m", "mingw32", "SDL2main", "SDL2"] //pthread
    cpp.includePaths: ["C:/dev/SDL2-2.0.14/x86_64-w64-mingw32/include/"]
    cpp.libraryPaths: ["C:/dev/SDL2-2.0.14/x86_64-w64-mingw32/lib/"]

    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }

}

CppApplication {
    name: "mini"
    consoleApplication: true
    Depends { name: "PikumaSoft" }
    Depends { name: "cpp" }

    files: [
        "src/main2.c",
        "src/stb_image.h",
        "src/stb_image_write.h"
    ]
    cpp.cLanguageVersion: "c99"
    cpp.cFlags: [
        "-Wall",
        "-Wextra", "-Wno-double-promotion", "-Wno-sign-compare",
        //"-fsanitize-undefined-trap-on-error", "-fsanitize=undefined", "-fsanitize=bounds"
        //-fsanitize=memory"
    ]

    cpp.staticLibraries: ["m"]

    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }

}

StaticLibrary {
    name: "PikumaSoft"
    Depends { name: "cpp" }

    // MSCV compiler flags:
    // /EHsc /EHc /O2 /W4 /fp:fast /nologo /Ob2 /GL /GA

    // GCC compiler and linker flags:
    //-s -O3 -Wall -ffast-math -ld3d9 -mconsole -mwindows -ld3d9
    files: [
        "src/array.c",
        "src/array.h",
        "src/camera.c",
        "src/camera.h",
        "src/clip.c",
        "src/clip.h",
        "src/draw_triangle_pikuma.c",
        "src/draw_triangle_pikuma.h",
        "src/draw_triangle_torb.c",
        "src/draw_triangle_torb.h",
        "src/func.c",
        "src/func.h",
        "src/light.c",
        "src/light.h",
        //"src/main.c",
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


    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [product.sourceDirectory, "src/"]
   }
}

}//project
