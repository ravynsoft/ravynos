cmake_minimum_required(VERSION 4.0)

add_executable(sh)
set(sh_src
	bltin/echo.c
	alias.c
	arith_yacc.c
	arith_yylex.c
	cd.c
	error.c
	eval.c
	exec.c
	expand.c
	histedit.c
	input.c
	jobs.c
	mail.c
	main.c
	memalloc.c
	miscbltin.c
	mystring.c
	options.c
	output.c
	parser.c
	redir.c
	show.c
	trap.c
	var.c)
target_sources(sh PRIVATE
	${sh_src}
	${CMAKE_CURRENT_BINARY_DIR}/builtins.c 
	${CMAKE_CURRENT_BINARY_DIR}/nodes.c 
	${CMAKE_CURRENT_BINARY_DIR}/syntax.c)
target_sources(sh PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/builtins.h
	${CMAKE_CURRENT_BINARY_DIR}/nodes.h
	${CMAKE_CURRENT_BINARY_DIR}/syntax.h
	${CMAKE_CURRENT_BINARY_DIR}/token.h)
target_sources(sh PRIVATE shims.c)

target_compile_definitions(sh PRIVATE SHELL)
target_compile_options(sh PRIVATE 
	-include${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/fbsdcompat/_fbsd_compat_.h
	--sysroot=${RAVYN_SDKROOT})

target_include_directories(sh PRIVATE 
	. 
	${CMAKE_CURRENT_BINARY_DIR}
	../../lib/libedit/src)

add_custom_command(OUTPUT builtins.c builtins.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mkbuiltins ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${sh_src})

add_custom_command(OUTPUT syntax.c syntax.h
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mksyntax
	DEPENDS mksyntax)

add_custom_command(OUTPUT nodes.c nodes.h
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mknodes 
		${CMAKE_CURRENT_SOURCE_DIR}/nodetypes
		${CMAKE_CURRENT_SOURCE_DIR}/nodes.c.pat
	DEPENDS mknodes)

add_custom_command(OUTPUT mksyntax
	COMMAND ${HOST_CC}
		-include${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/fbsdcompat/_fbsd_compat_.h
		"-D__unused="
		-o ${CMAKE_CURRENT_BINARY_DIR}/mksyntax
		${CMAKE_CURRENT_SOURCE_DIR}/mksyntax.c)

add_custom_command(OUTPUT mknodes
	COMMAND ${HOST_CC} 
		-include${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c/fbsdcompat/_fbsd_compat_.h
		"-D__unused="
		-o ${CMAKE_CURRENT_BINARY_DIR}/mknodes
		${CMAKE_CURRENT_SOURCE_DIR}/mknodes.c)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/token.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mktokens)

target_link_options(sh PRIVATE
	-mmacos-version-min=${MACOS_VERSION_MIN}
	-L${ROOT_BINARY_DIR}/BSD/lib/ncurses -lncurses)

target_link_libraries(sh PRIVATE
	${ROOT_BINARY_DIR}/BSD/lib/libedit/libedit.a)

add_dependencies(BSD sh)
add_dependencies(sh Darwin)
