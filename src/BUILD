cc_library(
    name = "http",
    srcs = ["HTTP.cpp"],
    hdrs = ["HTTP.hpp"],
    deps = ["@boost//:algorithm", "@boost//:filesystem", "@boost//:optional"], 
)

cc_binary (
    name = "main",
    srcs = ["main.cpp"],
    deps = [":http", "@boost//:algorithm", "@boost//:filesystem", "@boost//:optional"], 
)
