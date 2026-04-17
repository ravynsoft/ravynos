cmake_minimum_required(VERSION 4.0)

add_executable(sh 
	alias.c arith_yacc.c arith_yylex.c cd.c bltin/echo.c error.c eval.c
	exec.c expand.c histedit.c input.c jobs.c kill.c mail.c main.c
	memalloc.c miscbltin.c mystring.c options.c output.c parser.c
	printf.c redir.c show.c test.c trap.c var.c)
target_sources(sh PRIVATE builtins.c nodes.c syntax.c)
target_sources(sh INTERFACE builtins.h nodes.h syntax.h token.h)

target_compile_definitions(sh PRIVATE SHELL)
target_include_directories(sh PRIVATE .)

add_custom_command(OUTPUT builtins.c builtins.h
	COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/mkbuiltins ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(OUTPUT syntax.c syntax.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mksyntax)

add_custom_command(OUTPUT nodes.c nodes.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mknodes 
		${CMAKE_CURRENT_SOURCE_DIR}/nodetypes
		${CMAKE_CURRENT_SOURCE_DIR}/nodes.c.pat)

	#.PATH:	${.CURDIR}/bltin \
	#${.CURDIR:H}/kill \
	#${.CURDIR:H}/test \
	#${SRCTOP}/usr.bin/printf

