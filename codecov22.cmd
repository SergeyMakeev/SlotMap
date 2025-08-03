if exist ./codecov_report/ (rd /S /Q ./codecov_report/)
rem Install OpenCppCoverage from here https://github.com/OpenCppCoverage/OpenCppCoverage/releases
OpenCppCoverage.exe --export_type html:codecov_report --stop_on_assert --sources slot*.* --excluded_sources SlotMapTest02.cpp --excluded_sources *googletest* --modules *.exe -- .\build2022\Debug\SlotMapTest.exe --gtest_filter=-SlotMapTest.*_Slow