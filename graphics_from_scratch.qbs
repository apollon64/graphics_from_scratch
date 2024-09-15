import qbs

Project {

CppApplication {
    name: "graphics_from_scratch"
    consoleApplication: true
    Depends { name: "PikumaSoft" }
    Depends { name: "settings" }

    files: [
        "Makefile",
        "src/main.c",
        "src/display.c",
        "src/display.h",
        "src/vertex_shading.c",
        "src/vertex_shading.h",
    ]
    cpp.cLanguageVersion: "c99"
    cpp.cFlags: [
        //"-Wall",
        //"-Wextra", "-Wno-double-promotion", "-Wno-sign-compare",
        //"-fsanitize-undefined-trap-on-error", "-fsanitize=undefined", "-fsanitize=bounds"
        //-fsanitize=memory"
    ]

    Properties {
      condition: qbs.targetOS.contains("linux")
      cpp.staticLibraries: ["m", "mingw32"]
    }
    cpp.staticLibraries: ["SDL2main", "SDL2"] //pthread
    cpp.includePaths: ["C:/dev/SDL2-2.30.7/include/"]
    cpp.libraryPaths: ["C:/dev/SDL2-2.30.7/lib/x64"]

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
        //"-Wall",
        //"-Wextra", "-Wno-double-promotion", "-Wno-sign-compare",
        //"-fsanitize-undefined-trap-on-error", "-fsanitize=undefined", "-fsanitize=bounds"
        //-fsanitize=memory"
    ]

    Properties {
      condition: qbs.targetOS.contains("linux")
      cpp.staticLibraries: ["m"]

    }

    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }

}

Product {
    name: "settings"
           Export {
               // See doccumentation for cpp at https://doc.qt.io/qbs/qml-qbsmodules-cpp.html#details
               Depends { name: "cpp" }
               //property bool isDebug: false //qbs.buildVariant == "debug"
               cpp.optimization: qbs.buildVariant == "debug" ? "none" : "fast" // none, fast, small
               cpp.debugInformation: qbs.buildVariant == "debug" ? true : false

               //cpp.staticLibraries: ["SDL2", "m"] //pthread
               //cpp.dynamicLibraries: "dl"
               //cpp.frameworks: "frameworkName"
               //cpp.precompiledHeader: "myheader.pch"
               cpp.treatWarningsAsErrors: false
               //cpp.cxxLanguageVersion: "c++11"
               //cpp.cxxFlags:["-fopenmp"] // GCC
               //cpp.cxxFlags:["/openmp", "/fp:fast"] // MSVC
               //cpp.cxxFlags:["/fp:fast"] // MSVC
               //cpp.enableRtti: false
               //cpp.enableExceptions: false
               // Remember to configure asan with ASAN_OPTIONS env var when using it.
               // -fsanitize=address  -fno-omit-frame-pointer
               //cpp.cxxFlags: ["-Wno-strict-aliasing", "-fno-omit-frame-pointer"]
               //cpp.cxxFlags: ["-Wall", "-Wno-strict-aliasing"]

               Properties {
                condition: qbs.targetOS.contains("windows")
                name: "Windows Linker"
                //cpp.cxxFlags: base.concat(["static", "mwindows"])
                //cpp.staticLibraries: base.concat(["PixelToaster.lib", "gdi32"])
                //cpp.dynamicLibraries: ["opengl32", "gdi32"]
               }

               cpp.warningLevel:"none"
               //cpp.warningLevel: "all" // all or "none", "default" or use cxxFlags

               cpp.includePaths: [
                   // "../glm/"
               ]
               //cpp.defines: ['COLOR_STR="blanched almond"']

               Properties {
                   //condition: qbs.buildVariant != "debug"
                   cpp.defines: outer.concat([qbs.buildVariant == "debug" ? "DEBUG=1" : "DEBUG=0",
                                              "MY_COMPILER=\""
                                              +"\\n compilerName          :"+cpp.compilerName
                                              +"\\n architecture          :"+cpp.architecture
                                              +"\\n version               :"+cpp.compilerVersion
                                              +"\\n commonCompilerFlags   :"+cpp.commonCompilerFlags
                                              +"\\n cFlags                :"+cpp.cFlags
                                              +"\\n cLanguageVersion      :"+cpp.cLanguageVersion
                                              +"\\n cppFlags              :"+cpp.cppFlags
                                              +"\\n cxxFlags              :"+cpp.cxxFlags
                                              +"\\n cxxLanguageVersion    :"+cpp.cxxLanguageVersion
                                              +"\\n cxxStandardLibrary    :"+cpp.cxxStandardLibrary

                                              +"\\n debugInfoSuffix       :"+cpp.debugInfoSuffix
                                              +"\\n driverLinkerFlags     :"+cpp.driverLinkerFlags
                                              +"\\n dynamicLibraries      :"+cpp.dynamicLibraries
                                              +"\\n endianness            :"+cpp.endianness
                                              +"\\n entryPoint            :"+cpp.entryPoint
                                              +"\\n exceptionHandlingModel:"+cpp.exceptionHandlingModel
                                              +"\\n executablePrefix      :"+cpp.executablePrefix

                                              +"\\n linkerFlags           :"+cpp.linkerFlags
                                              +"\\n linkerMode            :"+cpp.linkerMode
                                              +"\\n linkerName            :"+cpp.linkerName
                                              +"\\n linkerVariant         :"+cpp.linkerVariant

                                              +"\\n minimumWindowsVersion :"+cpp.minimumWindowsVersion
                                              +"\\n optimization          :"+cpp.optimization
                                              +"\\n platformDefines       :"+cpp.platformDefines

                                              +"\\n runtimeLibrary   :"+cpp.runtimeLibrary
                                              +"\\n staticLibraries  :"+cpp.staticLibraries

                                              +"\\n warningLevel:"+cpp.warningLevel
                                              +"\\n windowsApiCharacterSet:"+cpp.windowsApiCharacterSet
                                              +"\\n windowsApiFamily :"+cpp.windowsApiFamily


                                              +"\""])
               }


           }


}


StaticLibrary {
    name: "PikumaSoft"
    Depends { name: "cpp" }
    Depends { name: "settings" }

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
        "src/matrix.c",
        "src/matrix.h",
        "src/mesh.c",
        "src/mesh.h",
        "src/misc.c",
        "src/misc.h",
        "src/stretchy_buffer.h",
        "src/texture.c",
        "src/texture.h",
        "src/triangle.h",
        "src/typedefs.h",
        "src/upng.c",
        "src/upng.h",
        "src/vecmath.c",
        "src/vecmath.h",
        "src/render_font/software_bitmapfont.h",
        "src/render_font/software_bitmapfont.c",
    ]
    cpp.cLanguageVersion: "c99"
    cpp.cFlags: [
        //"-Wall",
        //"-Wextra", "-Wno-double-promotion", "-Wno-sign-compare",
        //"-fsanitize-undefined-trap-on-error", "-fsanitize=undefined", "-fsanitize=bounds"
        //-fsanitize=memory"
    ]


    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [product.sourceDirectory, "src/"]
   }
}


}//project
