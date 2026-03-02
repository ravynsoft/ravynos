
"" A Vim syntax highlighting file for Doc/Zsh/*.yo

" To try this, run:
"     cd Doc/Zsh && vim --cmd "source ./.vimrc" zle.yo
" (This sources the file <Doc/Zsh/.vimrc>.)
"
" To install this permanently:
" 1. Copy this file to ~/.vim/syntax/zyodl.vim
" 2. Create ~/.vim/filetype.vim as explained in ":help new-filetype" case C.
" 3. Add the following command to ~/.vim/filetype.vim:
"      autocmd BufRead,BufNewFile **/Doc/Zsh/*.yo setfiletype zyodl
" Or alternatively:
" 1. Append the contents of Doc/Zsh/.vimrc to your .vimrc, changing «<sfile>» to «%».
"
" You may also wish to set:
"     autocmd FileType zyodl setlocal conceallevel=2
" in order to benefit from the 'conceal' behaviour for LPAR(), RPAR(), and
" friends.

"" Test case:
"   texinode()()()()
"   chapter(foo)
"   vindex(foo) 
"   foo tt(foo) var(foo) bf(foo) em(foo) foo
"   xitem(foo)
"   item(foo)(foo)
"   sitem(foo)(foo foo)
"   COMMENT(foo var(foo) foo)
"   comment(foo)
"   example(print *.c+LPAR()#q:s/#%+LPAR()#b+RPAR()s+LPAR()*+RPAR().c/'S${match[1]}.C'/+RPAR())
"   example(zargs -- **/*(.) -- ls -l)
"   ifzman(zmanref(zshmisc))ifnzman(noderef(Redirection))
"   LPAR()foo 42 foo+RPAR()
"   chapter(foo (foo) foo)
"   chapter(foo (foo (foo) foo) foo) bar
"
"   sitem(foo)(foo (foo) foo)
"   sitem(foo)(foo (foo) foo)
"
"   sitem(foo)(foo tt(foo) foo) # nested underline

if exists("b:current_syntax")
  finish
endif
let s:cpo_save = &cpo
set cpo&vim

"" Syntax groups:
syn clear
syn cluster zyodlInline contains=zyodlTt,zyodlVar,zyodlBold,zyodlEmph,zyodlCond
syn region zyodlTt      start="\<tt("      end=")" contains=zyodlSpecial,zyodlParenthetical
syn region zyodlVar     start="\<var("     end=")" contains=zyodlSpecial,zyodlParenthetical
syn region zyodlBold    start="\<bf("      end=")" contains=zyodlSpecial,zyodlParenthetical
syn region zyodlEmph    start="\<em("      end=")" contains=zyodlSpecial,zyodlParenthetical
syn region zyodlIndex   start="\<.index("  end=")" contains=zyodlSpecial
syn match  zyodlNumber  "\d\+"
syn region zyodlItem    start="\<xitem(" end=")" contains=zyodlSpecial,@zyodlInline
syn region zyodlItem    start="\<item("  end=")" contains=zyodlSpecial,@zyodlInline
syn region zyodlExample start="\<example(" end=")" contains=zyodlSpecial,zyodlParenthetical
syn region zyodlComment start="\<COMMENT(" end=")" contains=zyodlSpecial,@zyodlInline,zyodlParenthetical
" comment that gets output in generated texinfo/roff source
syn region zyodlComment start="\<comment(" end=")"
syn region zyodlTitle   start="\<\(chapter\|subsect\|sect\)(" end=")" contains=zyodlSpecial,@zyodlInline,zyodlParenthetical
syn match  zyodlTitle   "^texinode(.*$"
syn region zyodlParenthetical start="\w\@<!(" end=")" transparent contained contains=zyodlParenthetical

" zyodlCond doesn't contain zyodlParenthetical, since section names (probably) don't have parentheticals.
syn region zyodlCond    start="\<\(ifzman\|ifnzman\)(" end=")" contains=zyodlRef,zyodlSpecial,@zyodlInline
syn region zyodlRef     start="\<\(zmanref\|noderef\)(" end=")"

" zyodlSItemArg2 should use zyodlParenthetical instead of the 'skip='
syn keyword zyodlKeyword sitem nextgroup=zyodlSItemArg1
syn region zyodlSItemArg1 oneline start="(" end=")" contains=zyodlSpecial,@zyodlInline nextgroup=zyodlSItemArg2 contained
syn region zyodlSItemArg2 start="(" end=")" contains=zyodlSpecial,@zyodlInline contained skip="\w\@<!([^)]*)"

" Miscellany
syn match zyodlLineJoiner /\\$/
syn keyword zyodlNote note Note NOTE

syn keyword zyodlBullet  itemiz      conceal cchar=• 
syn match   zyodlSpecial "\<DASH()-" conceal cchar=—
syn match   zyodlSpecial "+\?LPAR()" conceal cchar=(
syn match   zyodlSpecial "+\?RPAR()" conceal cchar=)
syn match   zyodlSpecial "+\?_LPAR_" conceal cchar=(
syn match   zyodlSpecial "+\?_RPAR_" conceal cchar=)
syn match   zyodlSpecial "+\?PLUS()" conceal cchar=+
syn match   zyodlFAQDash "+\?\<emdash()" conceal cchar=—
 
"" Highlight groups:
hi def link zyodlTt Constant
hi def link zyodlVar Identifier
" Not ':hi def link zyodlBold Bold' since there's no such group.
hi def zyodlBold gui=bold cterm=bold
hi def link zyodlEmph Type
hi def link zyodlIndex PreProc
hi def link zyodlSpecial Special
hi def link zyodlNumber Number
hi def link zyodlItem Keyword
hi def link zyodlExample String
hi def link zyodlComment Comment
hi def link zyodlTitle Title
hi def link zyodlCond Conditional
hi def link zyodlRef Include
hi def link zyodlSItemArg1 Macro
hi def link zyodlSItemArg2 Underlined
hi def link zyodlLineJoiner Special
hi def link zyodlNote Todo

"" Derived highlighting groups:
hi def link zyodlFAQDash zyodlSpecial

let b:current_syntax = "zyodl"
let &cpo = s:cpo_save
unlet s:cpo_save
