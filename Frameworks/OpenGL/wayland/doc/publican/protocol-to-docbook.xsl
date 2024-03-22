<?xml version="1.0" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" encoding="UTF-8" indent="yes" />

<xsl:template match="/">
  <!-- insert docbook's DOCTYPE blurb -->
    <xsl:text disable-output-escaping = "yes"><![CDATA[
<!DOCTYPE appendix PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" [
  <!ENTITY % BOOK_ENTITIES SYSTEM "Wayland.ent">
%BOOK_ENTITIES;
]>
]]></xsl:text>

  <appendix id="appe-Wayland-Protocol">
    <title>Wayland Protocol Specification</title>
    <xsl:apply-templates select="protocol/copyright" />

    <xsl:apply-templates select="protocol/interface" />
  </appendix>
</xsl:template>

<!-- Break text blocks separated by two new lines into paragraphs -->
<xsl:template name="break">
     <xsl:param name="text" />
     <xsl:param name="linebreak" select="'&#10;&#10;'" />
     <xsl:choose>
       <xsl:when test="contains($text,$linebreak)">
         <para>
           <xsl:value-of select="substring-before($text,$linebreak)" />
         </para>
         <xsl:call-template name="break">
           <xsl:with-param name="text" select="substring-after($text,$linebreak)" />
         </xsl:call-template>
       </xsl:when>
       <xsl:otherwise>
         <para><xsl:value-of select="$text" /></para>
       </xsl:otherwise>
     </xsl:choose>
</xsl:template>

<!-- Copyright blurb -->
<xsl:template match="copyright">
  <para>
    <literallayout>
      <xsl:value-of select="." disable-output-escaping="yes"/>
    </literallayout>
  </para>
</xsl:template>

<!-- Interface descriptions -->
<xsl:template match="interface" >
  <section id="protocol-spec-{@name}">
    <title>
      <xsl:value-of select="@name" />
      <!-- only show summary if it exists -->
      <xsl:if test="description/@summary">
	- <xsl:value-of select="description/@summary" />
      </xsl:if>
    </title>
    <xsl:call-template name="break">
      <xsl:with-param name="text" select="description" />
    </xsl:call-template>
    <xsl:if test="request">
      <section>
        <title>Requests provided by <xsl:value-of select="@name" /></title>
        <xsl:apply-templates select="request" />
      </section>
    </xsl:if>
    <xsl:if test="event">
      <section>
        <title>Events provided by <xsl:value-of select="@name" /></title>
        <xsl:apply-templates select="event" />
      </section>
    </xsl:if>
    <xsl:if test="enum">
      <section>
        <title>Enums provided by <xsl:value-of select="@name" /></title>
      <xsl:apply-templates select="enum" />
      </section>
    </xsl:if>
  </section>
</xsl:template>

<!-- table contents for enum values -->
<xsl:template match="entry">
  <varlistentry>
    <term><xsl:value-of select="@name"/></term>
    <listitem>
      <simpara>
        <xsl:value-of select="@value"/>
        <xsl:if test="@summary" >
          - <xsl:value-of select="@summary"/>
        </xsl:if>
      </simpara>
    </listitem>
  </varlistentry>
</xsl:template>

<!-- table contents for request/event arguments -->
<xsl:template match="arg">
  <varlistentry>
    <term><xsl:value-of select="@name"/></term>
    <listitem>
        <simpara>
          <xsl:value-of select="@type"/>
          <xsl:if test="@summary" >
            - <xsl:value-of select="@summary"/>
          </xsl:if>
        </simpara>
    </listitem>
  </varlistentry>
</xsl:template>

<!-- id arguments -->
<xsl:template match="arg[@type='object' and @interface]">
  <varlistentry>
    <term><xsl:value-of select="@name"/></term>
    <listitem>
        <simpara>
          <link linkend="protocol-spec-{@interface}">
            <xsl:value-of select="@interface"/>
          </link>
          <xsl:if test="@summary" >
            - <xsl:value-of select="@summary"/>
          </xsl:if>
        </simpara>
    </listitem>
  </varlistentry>
</xsl:template>

<!-- new_id arguments -->
<xsl:template match="arg[@type='new_id' and @interface]">
  <varlistentry>
    <term><xsl:value-of select="@name"/></term>
    <listitem>
        <simpara>
          id for the new
          <link linkend="protocol-spec-{@interface}">
            <xsl:value-of select="@interface"/>
          </link>
          <xsl:if test="@summary" >
            - <xsl:value-of select="@summary"/>
          </xsl:if>
        </simpara>
    </listitem>
  </varlistentry>
</xsl:template>

<!-- enum and bitfield arguments -->
<xsl:template match="arg[@enum]">
  <varlistentry>
    <term><xsl:value-of select="@name"/></term>
    <listitem>
        <simpara>
          <xsl:choose>
            <xsl:when test="contains(@enum, '.')">
              <link linkend="protocol-spec-{substring-before(@enum, '.')}-enum-{substring-after(@enum, '.')}">
                <xsl:value-of select="substring-before(@enum, '.')"/>
                <xsl:text>::</xsl:text>
                <xsl:value-of select="substring-after(@enum, '.')"/>
              </link>
            </xsl:when>
            <xsl:otherwise>
              <link linkend="protocol-spec-{../../@name}-enum-{@enum}">
                <xsl:value-of select="../../@name"/>
                <xsl:text>::</xsl:text>
                <xsl:value-of select="@enum"/>
              </link>
            </xsl:otherwise>
          </xsl:choose>
          (<xsl:value-of select="@type"/>)
          <xsl:if test="@summary" >
            - <xsl:value-of select="@summary"/>
          </xsl:if>
        </simpara>
    </listitem>
  </varlistentry>
</xsl:template>

<!-- Request/event list -->
<xsl:template match="request|event">
  <section id="protocol-spec-{../@name}-{name()}-{@name}">
    <title>
      <xsl:value-of select="../@name"/>::<xsl:value-of select="@name" />
      <xsl:if test="description/@summary">
        - <xsl:value-of select="description/@summary" />
      </xsl:if>
    </title>
    <para>
      <variablelist>
        <xsl:apply-templates select="arg"/>
      </variablelist>
    </para>
    <xsl:call-template name="break">
      <xsl:with-param name="text" select="description" />
    </xsl:call-template>
  </section>
</xsl:template>

<!-- Enumeration -->
<xsl:template match="enum">
  <section id="protocol-spec-{../@name}-enum-{@name}">
    <title>
      <xsl:value-of select="../@name"/>::<xsl:value-of select="@name" />
      <xsl:if test="@bitfield">
        - bitfield
      </xsl:if>
      <xsl:if test="description/@summary">
        - <xsl:value-of select="description/@summary" />
      </xsl:if>
    </title>
    <xsl:call-template name="break">
      <xsl:with-param name="text" select="description" />
    </xsl:call-template>
    <variablelist>
      <xsl:apply-templates select="entry"/>
    </variablelist>
  </section>
</xsl:template>

</xsl:stylesheet>

<!-- vim: set expandtab shiftwidth=2: -->
