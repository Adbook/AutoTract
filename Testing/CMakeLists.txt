# Test checking that the help can be printed even if no input file is given.
add_test(NAME AutoTractHelp COMMAND $<TARGET_FILE:AutoTract>
    --help
  )

# Test checking that the version can be printed even if no input file is given.
add_test(NAME AutoTractVersion COMMAND $<TARGET_FILE:AutoTract>
    --version
  )

set_tests_properties(AutoTractVersion PROPERTIES FAIL_REGULAR_EXPRESSION "Unspecified" )
