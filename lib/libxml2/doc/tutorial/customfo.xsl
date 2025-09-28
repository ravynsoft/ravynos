<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version='1.0'>

  <xsl:import href="http://docbook.sourceforge.net/release/xsl/1.61.2/fo/docbook.xsl" />

<!-- don't display ulink targets in main text -->
<xsl:param name="ulink.show">0</xsl:param>

<!-- don't use graphics for callouts -->
<xsl:param name="callout.unicode">1</xsl:param>
<xsl:param name="callout.graphics">0</xsl:param>

<!-- tighter margins -->
<xsl:param name="page.margin.inner">2in</xsl:param>
<xsl:param name="page.margin.outer">2in</xsl:param>

<!-- enable pdf bookmarks -->
<xsl:param name="fop.extensions">1</xsl:param>


</xsl:stylesheet>