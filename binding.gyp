{
    "targets": [
        {
            "target_name": "winmem",
            "sources": [
                "winmem.cpp"
            ],
            "libraries" : ["Psapi.lib"],
            "include_dirs": [
                "include",
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            "dependencies": [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
            "cflags!": [ "-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "msvs_settings": {
                "VCCLCompilerTool": {
                    "ExceptionHandling": 1
                }
            }
        }
    ]
}
