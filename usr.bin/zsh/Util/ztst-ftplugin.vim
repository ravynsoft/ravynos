" ztst filetype plugin

" Only do this when not done yet for this buffer
if exists("b:did_ftplugin")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

" Inherit zsh.vim
runtime! ftplugin/zsh.vim

let b:undo_ftplugin .= "| setl fo< comments< commentstring<"

" Set 'formatoptions' to break comment lines but not other lines,
" and insert the comment leader when hitting <CR> or using "o".
setlocal fo-=t fo+=croql

" Set 'comments' to format expected output/errput lines
setlocal comments+=:*>,:>,:*?,:?,:F:

" Format comments to be up to 78 characters long
if &textwidth == 0
  setlocal textwidth=78
endif

let &cpo = s:cpo_save
unlet s:cpo_save
