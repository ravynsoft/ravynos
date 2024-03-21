<?xml version="1.0" encoding="iso-8859-15"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml">

<!-- 
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with avahi; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA. 
-->

<xsl:output method="xml" version="1.0" encoding="iso-8859-15" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd" indent="yes"/>

<xsl:template match="/manpage">
  
    <html>

    <head>
      <title><xsl:value-of select="@name"/>(<xsl:value-of select="@section"/>)</title>
      <style type="text/css">
        body { color: black; background-color: white; } 
        a:link, a:visited { color: #900000; }       
        h1 { text-transform:uppercase; font-size: 18pt; } 
        p { margin-left:1cm; margin-right:1cm; } 
        .cmd { font-family:monospace; }
        .file { font-family:monospace; }
        .arg { text-transform:uppercase; font-family:monospace; font-style: italic; }
        .opt { font-family:monospace; font-weight: bold;  }
        .manref { font-family:monospace; }
        .option .optdesc { margin-left:2cm; }
      </style>
    </head>
    <body>
      <h1>Name</h1>
      <p><xsl:value-of select="@name"/>
        <xsl:if test="string-length(@desc) &gt; 0"> - <xsl:value-of select="@desc"/></xsl:if>
      </p>
      <xsl:apply-templates />
    </body>
  </html>
</xsl:template>

<xsl:template match="p">
 <p>
  <xsl:apply-templates/>
 </p>
</xsl:template>

<xsl:template match="cmd">
 <p class="cmd">
  <xsl:apply-templates/>
 </p>
</xsl:template>

<xsl:template match="arg">
  <span class="arg"><xsl:apply-templates/></span>
</xsl:template>

<xsl:template match="opt">
  <span class="opt"><xsl:apply-templates/></span>
</xsl:template>

<xsl:template match="file">
  <span class="file"><xsl:apply-templates/></span>
</xsl:template>

<xsl:template match="optdesc">
  <div class="optdesc">
    <xsl:apply-templates/>
  </div>
</xsl:template>

<xsl:template match="synopsis">
  <h1>Synopsis</h1>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="seealso">
  <h1>Synopsis</h1>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="description">
  <h1>Description</h1>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="options">
  <h1>Options</h1>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="section">
  <h1><xsl:value-of select="@name"/></h1>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="option">
  <div class="option"><xsl:apply-templates/></div>
</xsl:template>

<xsl:template match="manref">
  <xsl:choose>
    <xsl:when test="string-length(@href) &gt; 0">
    <a class="manref"><xsl:attribute name="href"><xsl:value-of select="@href"/></xsl:attribute><xsl:value-of select="@name"/>(<xsl:value-of select="@section"/>)</a>
    </xsl:when>
    <xsl:otherwise>
    <span class="manref"><xsl:value-of select="@name"/>(<xsl:value-of select="@section"/>)</span>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="url">
  <a class="url"><xsl:attribute name="href"><xsl:value-of select="@href"/></xsl:attribute><xsl:value-of select="@href"/></a>
</xsl:template>

</xsl:stylesheet>
