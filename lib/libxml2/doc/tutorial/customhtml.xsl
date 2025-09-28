<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version='1.0'>

<xsl:import href="http://docbook.sourceforge.net/release/xsl/1.61.2/html/chunk.xsl" />

<!-- use callout graphics -->
<xsl:param name="callout.graphics">1</xsl:param>

<!-- use admonition graphics -->
<xsl:param name="admon.graphics">1</xsl:param>
<xsl:template match="revhistory"></xsl:template>
<xsl:template match="revision"></xsl:template>
<xsl:template match="revnumber"></xsl:template>

<xsl:template match="revremark"></xsl:template>

</xsl:stylesheet>