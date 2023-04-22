# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# This module defines a list of warnings called TODDS_CPP_WARNING_FLAGS. They can be enabled by using:
# target_compile_options(${TARGET_NAME} PRIVATE ${TODDS_CPP_WARNING_FLAGS})

include_guard(GLOBAL)

option(TODDS_WARNINGS_AS_ERRORS "Treat TODDS_CPP_WARNING_FLAGS as errors." OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	option(TODDS_CLANG_ALL_WARNINGS
		"Include most Clang warnings. This may trigger unexpected positives when using newer Clang versions." OFF)
endif ()

message(STATUS "Defining TODDS_CPP_WARNING_FLAGS")

set(TODDS_CPP_WARNING_FLAGS)

if (TODDS_CLANG_ALL_WARNINGS AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	list(APPEND TODDS_CPP_WARNING_FLAGS
		-Weverything                          # Enable every Clang warning except for the following exceptions.
		-Wno-c++98-compat                     # This project is not compatible with C++98.
		-Wno-c++98-compat-pedantic            # This project is not compatible with C++98.
		-Wno-c++20-compat                     # This project is compatible with C++20 only.
		-Wno-padded                           # Allow the compiler to add any padding it needs.
		-Wno-exit-time-destructors            # Define global variables required by the SIGINT handler.
		-Wno-global-constructors              # Define global variables required by the SIGINT handler.
		-Wno-disabled-macro-expansion         # SIGINT handler on UNIX systems.
		)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# Warnings present in all supported versions of GCC and Clang.
	list(APPEND TODDS_CPP_WARNING_FLAGS
		-Wall                # Enables most warnings.
		-Wextra              # Enables an extra set of warnings.
		-pedantic            # Strict compliance to the standard is not met.
		-Wcast-align         # Pointer casts which increase alignment.
		-Wcast-qual          # A pointer is cast to remove a type qualifier, or add an unsafe one.
		-Wconversion         # Implicit type conversions that may change a value.
		-Wdate-time          # Macros that might prevent bit-wise-identical compilations are encountered.
		-Wdouble-promotion   # Implicit conversions from "float" to "double".
		-Wextra-semi         # Redundant semicolons after in-class function definitions.
		-Wformat=2           # printf/scanf/strftime/strfmon format string anomalies.
		-Wnon-virtual-dtor   # Non-virtual destructors are found.
		-Wnull-dereference   # Dereferencing a pointer may lead to erroneous or undefined behavior.
		-Wold-style-cast     # C-style cast is used in a program.
		-Woverloaded-virtual # Overloaded virtual function names.
		-Wsign-conversion    # Implicit conversions between signed and unsigned integers.
		-Wshadow             # One variable shadows another.
		-Wswitch-enum        # A switch statement has an index of enumerated type and lacks a case.
		-Wundef              # An undefined identifier is evaluated in an #if directive.
		-Wunused             # Enable all -Wunused- warnings.
		)
	# Enable additional warnings depending on the compiler type and version in use.
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		list(APPEND TODDS_CPP_WARNING_FLAGS
			-Walloca                    # This option warns on all uses of alloca in the source.
			-Walloc-zero                # Calls to allocation functions with attribute alloc_size with zero bytes.
			-Warith-conversion          # Implicit conversions when it cannot change operand values.
			-Wdisabled-optimization     # A requested optimization pass is disabled because GCC is not able to do it.
			-Wdouble-promotion          # Warn about implicit conversions from "float" to "double".
			-Wduplicated-branches       # Duplicated branches in if-else statements.
			-Wduplicated-cond           # Duplicated conditions in an if-else-if chain.
			-Weffc++                    # Warnings related to guidelines from Scott Meyers’ Effective C++ books.
			-Wextra-semi                # Redundant semicolons after in-class function definitions.
			-Wlogical-op                # A logical operator is always evaluating to true or false.
			-Wmisleading-indentation    # Warn when indentation does not reflect the block structure.
			-Wmultiple-inheritance      # Do not allow multiple inheritance.
			-Wnull-dereference          # Dereferencing a pointer may lead to undefined behavior.
			-Wredundant-decls           # Something is declared more than once in the same scope.
			-Wredundant-tags            # Redundant class-key and enum-key where it can be eliminated.
			-Wsign-promo                # Overload resolution chooses a promotion from unsigned to a signed type.
			-Wsuggest-final-methods     # Virtual methods that could be declared as final or in an anonymous namespace.
			-Wsuggest-final-types       # Types with virtual methods that could be declared as final or in an anonymous namespace.
			-Wsuggest-override          # Overriding virtual functions that are not marked with the override keyword.
			-Wunsafe-loop-optimizations # The loop cannot be optimized because the compiler cannot assume anything.
			-Wuseless-cast              # Useless casts.
			)
	endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	list(APPEND TODDS_CPP_WARNING_FLAGS
		/permissive- # Specify standards conformance mode to the compiler.
		/W4          # Enable level 4 warnings.
		/w14062      # Enumerator 'identifier' in a switch of enum 'enumeration' is not handled.
		/w14242      # The types are different, possible loss of data. The compiler makes the conversion.
		/w14254      # A larger bit field was assigned to a smaller bit field, possible loss of data.
		/w14263      # Member function does not override any base class virtual member function.
		/w14265      # 'class': class has virtual functions, but destructor is not virtual.
		/w14287      # 'operator': unsigned/negative constant mismatch.
		/w14289      # Loop control variable is used outside the for-loop scope.
		/w14296      # 'operator': expression is always false.
		/w14311      # 'variable' : pointer truncation from 'type' to 'type'.
		/w14545      # Expression before comma evaluates to a function which is missing an argument list.
		/w14546      # Function call before comma missing argument list.
		/w14547      # Operator before comma has no effect; expected operator with side-effect.
		/w14549      # Operator before comma has no effect; did you intend 'operator2'?
		/w14555      # Expression has no effect; expected expression with side-effect.
		/w14640      # 'instance': construction of local static object is not thread-safe.
		/w14826      # Conversion from 'type1' to 'type2' is sign-extended.
		/w14905      # Wide string literal cast to 'LPSTR'.
		/w14906      # String literal cast to 'LPWSTR'.
		/w14928      # Illegal copy-initialization; applied more than one user-defined conversion.
		/analyze     # Code analysis.
		/wd4530      # Allow code that may raise exceptions when exceptions are disabled.
		/wd6011      # Triggered by OpenCV.
		/wd6201      # Triggered by OpenCV.
		/wd6239      # Triggered by code inside of fmt.
		/wd6294      # Triggered by OpenCV.
		/wd6297      # Triggered by code inside of rgbcx.
		/wd6385      # Triggered by OneAPI TBB on debug mode.
		/wd28182     # Triggered by OneAPI TBB on debug mode.
		/wd28301     # Triggered by the Windows API.
		)
endif ()

# Enable warnings as errors.
if (TODDS_WARNINGS_AS_ERRORS)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		list(APPEND TODDS_CPP_WARNING_FLAGS -Werror)
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		list(APPEND TODDS_CPP_WARNING_FLAGS /WX)
	endif ()
endif ()
