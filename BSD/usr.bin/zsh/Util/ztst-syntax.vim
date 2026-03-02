"" A Vim syntax highlighting file for Test/*.ztst
"
" See ../Util/zyodl.vim for installation instructions.
" Also, it's recommended to 'setlocal conceallevel=3 concealcursor=nc'.
"
" See B01cd.ztst for cases we cover

" TODO: Some zsh syntax isn't highlighted, e.g., «{ cd $0 }» doesn't highlight either 'cd' or '$0'
"   Apparently because the $VIMRUNTIME/syntax/zsh.vim zshBrackets group is defined as 'contains=TOP'?
"   https://bugs.debian.org/947120
" TODO: ZTST_unimplemented ZTST_skip aren't recognized everywhere
"   I haven't found yet a legitimate use where they aren't highlighted, but
"   they aren't highlighted in theoretical cases such as (( ++ZTST_skip )).
"   (This example is theoretical because those variables are string-typed.)

"" Boilerplate:
if exists("b:current_syntax")
  finish
endif
let s:cpo_save = &cpo
set cpo&vim

"" Syntax groups:
syn clear

syn include @zsh                   syntax/zsh.vim

" Note that we don't do /^\s\zs.*/ here.  If we did that, lines that start
" with " #" (a space and a hash sign) would not be highlighted as comments,
" because zshComment's patterns won't match unless the '#' is preceded by
" a space or start-of-line.  See:
"
" https://github.com/chrisbra/vim-zsh/issues/21#issuecomment-577738791
syn match  ztstPayload             /^\s.*/ contains=@zsh

syn match  ztstExitCode            /^\d\+\|^-/                nextgroup=ztstFlags
syn match  ztstFlags               /[.dDqf]*:/      contained nextgroup=ztstTestName contains=ztstColon
syn match  ztstColon               /:.\@=/          contained
syn region ztstTestName            start=// end=/$/ contained 

syn match  ztstInputMarker         /^<.\@=/                   nextgroup=ztstInput
syn region ztstInput               start=// end=/$/ contained

syn match  ztstOutputPattern       /^[*]>/                    nextgroup=ztstOutput   contains=ztstOutputPatternSigil,ztstOutputPatternMarker
syn match  ztstOutputPatternSigil  /[*]/            contained
syn match  ztstOutputPatternMarker /[>].\@=/        contained conceal
syn match  ztstOutputLiteral       /^>.\@=/                   nextgroup=ztstOutput
syn region ztstOutput              start=// end=/$/ contained

syn match  ztstErrputPattern       /^[*][?]/                  nextgroup=ztstErrput   contains=ztstErrputPatternSigil,ztstErrputPatternMarker
syn match  ztstErrputPatternSigil  /[*]/            contained
syn match  ztstErrputPatternMarker /[?].\@=/        contained conceal
syn match  ztstErrputLiteral       /^[?].\@=/                 nextgroup=ztstErrput
syn region ztstErrput              start=// end=/$/ contained

syn match  ztstFrequentExplanationMarker /^F:/                nextgroup=ztstFrequentExplanation
syn region ztstFrequentExplanation start=// end=/$/ contained

syn match  ztstDirective           /^%.*/

syn match  ztstComment             /^#.*/

" Highlight those variables which are /de jure/ or /de facto/ APIs of the test
" harness to the test files.
syn keyword ztstSpecialVariable ZTST_unimplemented ZTST_skip ZTST_testdir ZTST_fd ZTST_srcdir containedin=@zsh 

"" Sync
" The following is sufficient for our modest line-based format, and helps
" sidestep problems resulting from test cases that use syntax constructs
" that confuse us and/or syntax/zsh.vim.  If we outgrow it, we should sync
" on empty lines instead.
"
" If you run into syntax highlighting issues, just scroll the line that throws
" the syntax highlighting off off the top of the screen.
syn sync maxlines=1

"" Highlight groups:
" Note: every group that's defaulted to "Ignore" has a match pattern that ends
" with /.\@=/.  This ensures the Ignore will only be effective if there is an
" immediately following group that _will_ be highlighted.  (That group will be
" one of ztstTestName, ztstInput, ztstOutput, and ztstErrput.)
"
" ### The Ignore would still apply if the rest of the line is all-whitespace.
" ###
" ### If you run into such lines, consider setting the 'list' and 'listchars'
" ### options appropriately.
hi def link ztstExitCode                  Number
hi def link ztstFlags                     Normal
hi def link ztstColon                     Ignore
hi def link ztstTestName                  Title
hi def link ztstInput                     Normal
hi def link ztstInputMarker               Ignore
hi def link ztstOutput                    String
hi def link ztstOutputPatternSigil        Type
hi def link ztstOutputPatternMarker       Ignore
hi def link ztstOutputLiteral             Ignore
hi def link ztstErrput                    Identifier
hi def link ztstErrputPatternSigil        Type
hi def link ztstErrputPatternMarker       Ignore
hi def link ztstErrputLiteral             Ignore
hi def link ztstDirective                 Statement
hi def link ztstComment                   Comment
hi def link ztstFrequentExplanation       PreProc
hi def link ztstFrequentExplanationMarker Ignore
hi def link ztstSpecialVariable           Underlined

"" Boilerplate:
let b:current_syntax = "ztst"
let &cpo = s:cpo_save
unlet s:cpo_save
