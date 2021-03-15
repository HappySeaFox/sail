# Intended to be included by SAIL.
#
function(sail_check_c11_thread_local)
	cmake_push_check_state(RESET)
		set(CMAKE_REQUIRED_INCLUDES ${PROJECT_BINARY_DIR}/include)

		check_c_source_compiles(
			"
			#include \"config.h\"

			int main(int argc, char *argv[]) {
				SAIL_THREAD_LOCAL static int i = 0;
				return 0;
			}
		"
		SAIL_HAVE_THREAD_LOCAL
		)
	cmake_pop_check_state()

	if (NOT SAIL_HAVE_THREAD_LOCAL)
		message(FATAL_ERROR "Currently selected ${CMAKE_C_COMPILER_ID} compiler doesn't support C11 thread local variables.")
	endif()
endfunction()
