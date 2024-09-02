local projectName = "dslConsole"

add_rules("mode.release", "mode.debug")

add_requires("qt6widgets", "qt6network")

target(projectName)
  set_languages("cxxlatest")
  set_exceptions("cxx")

  add_includedirs("./src")

  add_files("./src/*.cpp")

  add_packages("qt6widgets", "qt6network")

  add_cxxflags("-Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -permissive- -Zc:__cplusplus -Zc:externConstexpr -utf-8 -w34100 -w34189 -w44996 -w44456 -w44457 -w44458")