cmake_minimum_required(VERSION 3.16)
idf_component_register(SRCS "src/protocol.c" INCLUDE_DIRS "include" PRIV_REQUIRES cjson)
