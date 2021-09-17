# Most plugins will need to link against KF5TextEditor to have access to its plugin interface.
find_package(KF5TextEditor ${KF5_DEP_VERSION} QUIET REQUIRED)

ecm_optional_add_subdirectory(backtracebrowser)
ecm_optional_add_subdirectory(close-except-like) # Close all documents except this one (or similar).
ecm_optional_add_subdirectory(colorpicker) # Inline color preview/picker
ecm_optional_add_subdirectory(externaltools)
ecm_optional_add_subdirectory(filebrowser)
ecm_optional_add_subdirectory(filetree)
ecm_optional_add_subdirectory(gdbplugin) 
ecm_optional_add_subdirectory(git-blame) # Inline git-blame viewer
ecm_optional_add_subdirectory(kate-ctags)
ecm_optional_add_subdirectory(katebuild-plugin)
ecm_optional_add_subdirectory(katesql) 
ecm_optional_add_subdirectory(konsole)
ecm_optional_add_subdirectory(lspclient) # Language Server Protocol (LSP) client plugin.
ecm_optional_add_subdirectory(preview) # Live preview of sources in target format.
ecm_optional_add_subdirectory(project) # Small & smart project manager.
ecm_optional_add_subdirectory(replicode)
ecm_optional_add_subdirectory(search)
ecm_optional_add_subdirectory(sessionapplet)
ecm_optional_add_subdirectory(snippets)
ecm_optional_add_subdirectory(symbolviewer) # List symbols and functions in a file.
ecm_optional_add_subdirectory(tabswitcher) # ALT+Tab like tab switcher.
ecm_optional_add_subdirectory(textfilter) # Pipe text through some external command.
ecm_optional_add_subdirectory(xmlcheck) # XML Validation plugin
ecm_optional_add_subdirectory(xmltools) # XML completion
