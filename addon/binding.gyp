{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "source/addon.cpp",
        "source/pppwrapper.cpp"
      ],
      "include_dirs": [
        "../install_release_x86",
        "include"
      ],
      
      "conditions": [
        [
          "OS=='win'",
          {
            "libraries": [
              "..\\..\\install_release_x86\\libppp.lib"
            ]
          },
          "OS=='linux'",
          {
            "libraries": [
              "-L../../install_release_x64/",
              "-llibppp"
            ],
            "ldflags": [
              "-Wl,-rpath,'$$ORIGIN'"
            ],
            "cflags_cc": [
              "-fexceptions",
              "-fPIC",
              "-Wno-unknown-pragmas"
            ]
          }
        ]
      ]
    }
  ]
}