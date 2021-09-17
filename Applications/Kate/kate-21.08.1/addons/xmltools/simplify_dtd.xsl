<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!--
Remove all parameter entities and other useless stuff from the kdex.dtd.xml DTD,
to make the file smaller.
Daniel Naber, 2001-09-23, last update 2001-09-29
-->

<xsl:output method="xml"
	indent="yes"
	doctype-public="-//Norman Walsh//DTD DTDParse V2.0//EN"
	doctype-system="dtd.dtd"/>

	<xsl:template match="dtd">
		<dtd>
			<xsl:attribute name="version"><xsl:value-of select="@version"/></xsl:attribute>
			<xsl:attribute name="unexpanded"><xsl:value-of select="@unexpanded"/></xsl:attribute>
			<xsl:attribute name="title"><xsl:value-of select="@title"/></xsl:attribute>
			<xsl:attribute name="namecase-general"><xsl:value-of select="@namecase-general"/></xsl:attribute>
			<xsl:attribute name="namecase-entity"><xsl:value-of select="@namecase-entity"/></xsl:attribute>
			<xsl:attribute name="xml"><xsl:value-of select="@xml"/></xsl:attribute>
			<xsl:attribute name="system-id"><xsl:value-of select="@system-id"/></xsl:attribute>
			<xsl:attribute name="public-id"><xsl:value-of select="@public-id"/></xsl:attribute>
			<xsl:attribute name="declaration"><xsl:value-of select="@declaration"/></xsl:attribute>
			<xsl:attribute name="created-by">XSLT Simplifier</xsl:attribute>
			<xsl:attribute name="created-on"><xsl:value-of select="@created-on"/> (original version)</xsl:attribute>
			<xsl:apply-templates />
		</dtd>
	</xsl:template>

	<!-- remove some "entity" elements and other stuff not needed: -->

	<xsl:template match="element">
			<element>
				<xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
				<xsl:copy-of select="content-model-expanded"/>
			</element>
	</xsl:template>

	<xsl:template match="entity">
		<xsl:if test="not(@type='param')">
			<entity>
				<xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
				<xsl:attribute name="type"><xsl:value-of select="@type"/></xsl:attribute>
				<xsl:apply-templates select="text-expanded"/>
			</entity>
		</xsl:if>
	</xsl:template>

	<xsl:template match="text-expanded">
		<xsl:copy-of select="."/>
	</xsl:template>

	<xsl:template match="attlist">
		<attlist>
			<xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
			<xsl:copy-of select="attribute"/>
		</attlist>
	</xsl:template>

	<xsl:template match="notation" />

</xsl:stylesheet>
