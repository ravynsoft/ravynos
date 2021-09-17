kdoctools_create_handbook(
  index.docbook 
  INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en 
  SUBDIR kate
)

kdoctools_create_manpage(
  man-kate.1.docbook 1 
  INSTALL_DESTINATION ${KDE_INSTALL_MANDIR}
)
