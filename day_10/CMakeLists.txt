create_aoc_exec(day10)

find_package(Python3 COMPONENTS Interpreter)
set(DAY_10_VENV_PATH "${CMAKE_CURRENT_BINARY_DIR}/.venv")
execute_process(COMMAND "${Python3_EXECUTABLE}" -m venv ${DAY_10_VENV_PATH})
execute_process(COMMAND "${DAY_10_VENV_PATH}/bin/pip" install --upgrade pip ortools)

# Here is the trick
## update the environment with VIRTUAL_ENV variable (mimic the activate script)
set(ENV{VIRTUAL_ENV} ${DAY_10_VENV_PATH})
## change the context of the search
set(Python3_FIND_VIRTUALENV FIRST)
## unset Python3_EXECUTABLE because it is also an input variable (see documentation, Artifacts Specification section)
unset(Python3_EXECUTABLE)
## Launch a new search
find_package(Python3 COMPONENTS Interpreter Development)
