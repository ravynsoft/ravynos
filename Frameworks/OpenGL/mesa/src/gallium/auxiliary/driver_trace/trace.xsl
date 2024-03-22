<?xml version="1.0"?>

<!--

Copyright 2008 VMware, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

!-->

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="html" />

	<xsl:strip-space elements="*" />

	<xsl:template match="/trace">
		<html>
			<head>
				<title>Gallium Trace</title>
			</head>
			<style>
				body {
					font-family: verdana, sans-serif;
					font-size: 11px;
					font-weight: normal;
					text-align : left;
				}

				.fun {
					font-weight: bold;
				}

				.var {
					font-style: italic;
				}

				.typ {
					display: none;
				}

				.lit {
					color: #0000ff;
				}

				.ptr {
					color: #008000;
				}
			</style>
			<body>
				<ol class="calls">
					<xsl:apply-templates/>
				</ol>
			</body>
		</html>
	</xsl:template>

	<xsl:template match="call">
		<li>
			<xsl:attribute name="value">
				<xsl:apply-templates select="@no"/>
			</xsl:attribute>
			<span class="fun">
				<xsl:value-of select="@class"/>
				<xsl:text>::</xsl:text>
				<xsl:value-of select="@method"/>
			</span>
			<xsl:text>(</xsl:text>
			<xsl:apply-templates select="arg"/>
			<xsl:text>)</xsl:text>
			<xsl:apply-templates select="ret"/>
		</li>
	</xsl:template>

	<xsl:template match="arg|member">
			<xsl:apply-templates select="@name"/>
			<xsl:text> = </xsl:text>
			<xsl:apply-templates />
			<xsl:if test="position() != last()">
				<xsl:text>, </xsl:text>
			</xsl:if>
	</xsl:template>

	<xsl:template match="ret">
		<xsl:text> = </xsl:text>
		<xsl:apply-templates />
	</xsl:template>

	<xsl:template match="bool|int|uint|float|enum">
		<span class="lit">
			<xsl:value-of select="text()"/>
		</span>
	</xsl:template>

	<xsl:template match="bytes">
		<span class="lit">
			<xsl:text>...</xsl:text>
		</span>
	</xsl:template>

	<xsl:template match="string">
		<span class="lit">
			<xsl:text>"</xsl:text>
			<xsl:call-template name="break">
				<xsl:with-param name="text" select="text()"/>
			</xsl:call-template>
			<xsl:text>"</xsl:text>
		</span>
	</xsl:template>

	<xsl:template match="array|struct">
		<xsl:text>{</xsl:text>
		<xsl:apply-templates />
		<xsl:text>}</xsl:text>
	</xsl:template>

	<xsl:template match="elem">
		<xsl:apply-templates />
		<xsl:if test="position() != last()">
			<xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template match="null">
		<span class="ptr">
			<xsl:text>NULL</xsl:text>
		</span>
	</xsl:template>

	<xsl:template match="ptr">
		<span class="ptr">
			<xsl:value-of select="text()"/>
		</span>
	</xsl:template>

	<xsl:template match="@name">
		<span class="var">
			<xsl:value-of select="."/>
		</span>
	</xsl:template>
	
	<xsl:template name="break">
		<xsl:param name="text" select="."/>
		<xsl:choose>
			<xsl:when test="contains($text, '&#xa;')">
				<xsl:value-of select="substring-before($text, '&#xa;')"/>
				<br/>
				<xsl:call-template name="break">
					 <xsl:with-param name="text" select="substring-after($text, '&#xa;')"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$text"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="replace">
		<xsl:param name="text"/>
		<xsl:param name="from"/>
		<xsl:param name="to"/>
		<xsl:choose>
			<xsl:when test="contains($text,$from)">
				<xsl:value-of select="concat(substring-before($text,$from),$to)"/>
				<xsl:call-template name="replace">
					<xsl:with-param name="text" select="substring-after($text,$from)"/>
					<xsl:with-param name="from" select="$from"/>
					<xsl:with-param name="to" select="$to"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$text"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

</xsl:transform>
