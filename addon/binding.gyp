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
          },
          "OS=='mac'",
          {
            "libraries": [
              "-L$(INSTALL_DIR)",
              "-llibppp"
            ],
            "ldflags": [
              "-Wl,-rpath,'$$ORIGIN'"
            ],
              'xcode_settings': {
              'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
            }
          }
        ]
      ]
    }
  ]
}