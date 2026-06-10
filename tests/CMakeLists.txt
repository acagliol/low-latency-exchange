# catch2 via fetchcontent — no system install needed
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(exchange_tests
  test_messages.cpp
  test_validate.cpp
  test_timestamp.cpp
  test_order.cpp
  test_order_book.cpp
)
target_link_libraries(exchange_tests PRIVATE exchange_lib Catch2::Catch2WithMain)
set_project_warnings(exchange_tests)

include(CTest)
include(Catch)
catch_discover_tests(exchange_tests)
