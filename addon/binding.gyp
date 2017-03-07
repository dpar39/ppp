{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "source/addon.cpp",
        "source/pppwrapper.cpp"
      ],
      "include_dirs": [
        "$(INSTALL_DIR)",
        "include"
      ],
      "configurations": {
        "Release": {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }
      },
      "conditions": [
        [
          "OS=='win'",
          {
            "libraries": [
              "$(INSTALL_DIR)/libppp.lib"
            ]
          },
          "OS=='linux'",
          {
            "libraries": [
              "-L$(INSTALL_DIR)",
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