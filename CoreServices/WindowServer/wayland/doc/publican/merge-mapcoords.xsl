<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:param name="basedir"/>
  <xsl:output method="xml" encoding="utf-8" indent="yes"/>
  <!-- -->
  <!-- Template for the root so we can add a DOCTYPE -->
  <xsl:template match="/">
    <xsl:text disable-output-escaping="yes"><![CDATA[<!DOCTYPE chapter PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" [
<!ENTITY % BOOK_ENTITIES SYSTEM "Wayland.ent">
%BOOK_ENTITIES;
]>
]]></xsl:text>
    <xsl:apply-templates select="@*|node()"/>
  </xsl:template>
  <!-- -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>
  <!-- -->
  <!-- suppress existing image map areas -->
  <xsl:template match="area"/>
  <!-- -->
  <xsl:template match="areaspec[area][name(..)='imageobjectco']">
    <xsl:element name="areaspec">
      <xsl:apply-templates select="@*"/>
      <xsl:text>&#xa;</xsl:text>
      <xsl:variable name="pngfile" select="../imageobject/imagedata/@fileref"/>
      <xsl:variable name="mapfile" select="concat(substring($pngfile, 1, string-length($pngfile)-3), 'map')"/>
      <xsl:variable name="maproot" select="document(concat($basedir, '/', $mapfile))"/>
      <!-- -->
      <!-- now emit the needed areas -->
      <xsl:for-each select="area">
	<xsl:variable name="anchor" select="."/>
	<xsl:variable name="other" select="($maproot)//area[@href=($anchor)/@x_steal]"/>
	<xsl:choose>
	  <xsl:when test="$other">
	    <xsl:text>&#x9;    </xsl:text>
	    <xsl:element name="area">
	      <xsl:attribute name="id">
		<xsl:value-of select="@id"/>
	      </xsl:attribute>
	      <xsl:attribute name="linkends">
		<xsl:value-of select="@linkends"/>
	      </xsl:attribute>
	      <xsl:attribute name="coords">
		<xsl:value-of select="($other)/@coords"/>
	      </xsl:attribute>
	    </xsl:element>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:text>&#x9;    </xsl:text>
	    <xsl:comment>
	      <xsl:value-of select="concat('Warning: unable to locate area tagged ', ($anchor)/@x_steal)"/>
	    </xsl:comment>
	  </xsl:otherwise>
	</xsl:choose>
	<xsl:text>&#xa;</xsl:text>
      </xsl:for-each>
      <!-- -->
      <xsl:text>&#x9;  </xsl:text>
    </xsl:element>
  </xsl:template>
</xsl:stylesheet>
