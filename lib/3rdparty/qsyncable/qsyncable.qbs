import qbs

DynamicLibrary {
    targetName: "qsyncable"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["core"] }

    cpp.cxxLanguageVersion: "c++11"
    cpp.visibility: "minimal"
    cpp.defines: [
        "SLATE_LIBRARY"
    ]

    Export {
        Depends { name: "cpp" }
        Depends {
            name: "Qt"
            submodules: ["core"]
        }

        cpp.includePaths: [product.sourceDirectory]
    }

    files: [
        "priv/qsalgotypes_p.h",
        "priv/qsdiffrunneralgo_p.h",
        "priv/qsfastdiffrunneralgo_p.h",
        "priv/qsimmutablewrapper_p.h",
        "priv/qstree.h",
        "priv/qstreenode.h",
        "qsdiffrunner.cpp",
        "qsdiffrunner.h",
        "qsdiffrunneralgo.cpp",
        "qsfastdiffrunner.h",
        "qsjsonlistmodel.cpp",
        "qsjsonlistmodel.h",
        "qslistmodel.cpp",
        "qslistmodel.h",
        "qspatch.cpp",
        "qspatch.h",
        "qspatchable.h",
        "qstree.cpp",
        "qstreenode.cpp",
        "qsuuid.cpp",
        "qsuuid.h",
        "qsyncablefunctions.cpp",
        "qsyncablefunctions.h",
        "qsyncableqmltypes.cpp",
        "qsyncableqmlwrapper.cpp"
        "qsyncableqmlwrapper.h",
    ]
}
