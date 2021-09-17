add_executable(tstestapp "")

target_link_libraries(
  tstestapp 
  PRIVATE KF5::TextEditor
)

target_sources(
  tstestapp 
  PRIVATE 
    tstestapp.cpp 
    ../tabswitcherfilesmodel.cpp
)
