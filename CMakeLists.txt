cmake_minimum_required(VERSION 3.10)

include_directories(.)

aux_source_directory(. FS)
add_executable(myfs ${FS})