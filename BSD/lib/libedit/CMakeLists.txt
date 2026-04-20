add_library(libedit STATIC
	./src/chared.c
	./src/chartype.c
	./src/common.c
	./src/el.c
	./src/eln.c
	./src/emacs.c
	./src/filecomplete.c
	./src/getline.c
	./src/hist.c
	./src/history.c
	./src/historyn.c
	./src/keymacro.c
	./src/literal.c
	./src/map.c
	./src/parse.c
	./src/prompt.c
	./src/read.c
	./src/readline.c
	./src/reallocarr.c
	./src/refresh.c
	./src/search.c
	./src/sig.c
	./src/strlcat.c
	./src/strlcpy.c
	./src/terminal.c
	./src/tokenizer.c
	./src/tokenizern.c
	./src/tty.c
	./src/unvis.c
	./src/vi.c
	./src/vis.c
	./src/wcsdup.c
	${CMAKE_CURRENT_BINARY_DIR}/vi.h
	${CMAKE_CURRENT_BINARY_DIR}/emacs.h
	${CMAKE_CURRENT_BINARY_DIR}/common.h
	${CMAKE_CURRENT_BINARY_DIR}/fcns.h
	${CMAKE_CURRENT_BINARY_DIR}/help.h
	${CMAKE_CURRENT_BINARY_DIR}/func.h
)

set_target_properties(libedit PROPERTIES PREFIX lib OUTPUT_NAME edit)
target_compile_options(libedit PRIVATE
	--sysroot=${RAVYN_SDKROOT}
)

target_include_directories(libedit PRIVATE
	${SYSROOT}/usr/include
	src
	.
	${CMAKE_CURRENT_BINARY_DIR}
	../ncurses/include
)

add_custom_command(OUTPUT vi.h 
	COMMAND AWK=awk sh ${CMAKE_CURRENT_SOURCE_DIR}/src/makelist -h
		${CMAKE_CURRENT_SOURCE_DIR}/src/vi.c > vi.h)

add_custom_command(OUTPUT emacs.h 
	COMMAND AWK=awk sh ${CMAKE_CURRENT_SOURCE_DIR}/src/makelist -h
		${CMAKE_CURRENT_SOURCE_DIR}/src/emacs.c > emacs.h)

add_custom_command(OUTPUT common.h 
	COMMAND AWK=awk sh ${CMAKE_CURRENT_SOURCE_DIR}/src/makelist -h
		${CMAKE_CURRENT_SOURCE_DIR}/src/common.c > common.h)

set(AHDR ${CMAKE_CURRENT_BINARY_DIR}/vi.h 
	 ${CMAKE_CURRENT_BINARY_DIR}/emacs.h
	 ${CMAKE_CURRENT_BINARY_DIR}/common.h)
set(ASRC ${CMAKE_CURRENT_SOURCE_DIR}/src/vi.c
	 ${CMAKE_CURRENT_SOURCE_DIR}/src/emacs.c
	 ${CMAKE_CURRENT_SOURCE_DIR}/src/common.c)

add_custom_command(OUTPUT fcns.h 
	COMMAND AWK=awk sh ${CMAKE_CURRENT_SOURCE_DIR}/src/makelist -fh ${AHDR} > fcns.h
	DEPENDS ${AHDR})

add_custom_command(OUTPUT help.h 
	COMMAND AWK=awk sh ${CMAKE_CURRENT_SOURCE_DIR}/src/makelist -bh ${ASRC} > help.h
	DEPENDS ${ASRC})

add_custom_command(OUTPUT func.h 
	COMMAND AWK=awk sh ${CMAKE_CURRENT_SOURCE_DIR}/src/makelist -fc ${AHDR} > func.h
	DEPENDS ${AHDR})

