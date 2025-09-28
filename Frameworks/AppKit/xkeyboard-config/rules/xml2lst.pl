#!/usr/bin/env perl

# converts the <rules>.xml file to the old format <rules>.lst file
#
# Usage:
#
# perl xml2lst.pl < filename.xml > filename.lst
#
# author Ivan Pascal

$doc = new_document( 0, '');
parse('', $doc);

($reg)   = node_by_name($doc, '/xkbConfigRegistry');
@models  = node_by_name($reg, 'modelList/model/configItem');
@layouts = node_by_name($reg, 'layoutList/layout/configItem');
@options = node_by_name($reg, 'optionList/group/configItem');

print "! model\n";
for $i (@models) {
   ($name) = node_by_name($i, 'name');
   ($descr) = node_by_name($i, 'description');
    printf("  %-15s %s\n", text_child($name), text_child($descr));
}

print "\n! layout\n";
for $i (@layouts) {
   ($name) = node_by_name($i, 'name');
   ($descr) = node_by_name($i, 'description');
    printf("  %-15s %s\n", text_child($name), text_child($descr));
}

print "\n! variant\n";
for $l (@layouts) {
   ($lname) = node_by_name($l, 'name');
    @variants = node_by_name($l, '../variantList/variant/configItem');
    for $v (@variants) {
      ($name) = node_by_name($v, 'name');
      ($descr) = node_by_name($v, 'description');
       printf("  %-15s %s: %s\n",
               text_child($name), text_child($lname), text_child($descr));
    }
}

print "\n! option\n";
for $g (@options) {
   ($name) = node_by_name($g, 'name');
   ($descr) = node_by_name($g, 'description');
    printf("  %-20s %s\n", text_child($name), text_child($descr));

    @opts = node_by_name($g, '../option/configItem');
    for $o (@opts) {
      ($name) = node_by_name($o, 'name');
      ($descr) = node_by_name($o, 'description');
       printf("  %-20s %s\n",
               text_child($name), text_child($descr));
    }
}

sub with_attribute {
    local ($nodelist, $attrexpr) = @_;
    local ($attr, $value) = split (/=/, $attrexpr);
    local ($node, $attrvalue);
    if (defined $value && $value ne '') {
        $value =~ s/"//g;
        foreach $node (@{$nodelist}) {
           $attrvalue = node_attribute($node, $attr);
           if (defined $attrvalue && $attrvalue eq $value) {
               return $node;
           }
        }
    } else {
        foreach $node (@{$nodelist}) {
           if (! defined node_attribute($node, $attr)) {
               return $node;
           }
        }
    }
    undef;
}

# Subroutines

sub parse {
   local $intag = 0;
   my (@node_stack, $parent);
   $parent = @_[1];
   local ($tag, $text);

   while (<>) {
      chomp;
      @str = split /([<>])/;
      shift @str if ($str[0] eq '' || $str[0] =~ /^[ \t]*$/);

      while (scalar @str) {
         $token = shift @str;
         if ($token eq '<') {
            $intag = 1;
            if (defined $text) {
               add_text_node($parent, $text);
               undef $text;
            }
         } elsif ($token eq '>') {
            $intag = 0;
            if ($tag =~ /^\/(.*)/) { # close tag
               $parent = pop @node_stack;
            } elsif ($tag =~ /^([^\/]*)\/$/) {
               empty_tag($parent, $1);
            } else {
               if (defined ($node = open_tag($parent, $tag))) {
                  push @node_stack, $parent;
                  $parent = $node;
               }
            }
            undef $tag;
         } else {
            if ($intag == 1) {
               if (defined $tag) {
                  $tag .= ' '. $token;
               } else {
                  $tag = $token;
               }
            } else {
               if (defined $text) {
                  $text .= "\n" . $token;
               } else {
                  $text = $token;
               }
            }
         }
      }
   }
}

sub new_document {
   $doc = new_node( 0, '', 'DOCUMENT');
   $doc->{CHILDREN} = [];
   return $doc;
}

sub new_node {
  local ($parent_node, $tag, $type) = @_;

  my %node;
  $node{PARENT} = $parent_node;
  $node{TYPE} = $type;

  if ($type eq 'COMMENT' || $type eq 'TEXT') {
     $node{TEXT} = $tag;
     $node{NAME} = $type;
     return \%node;
  }

  local ($tname, $attr) = split(' ', $tag, 2);
  $node{NAME} = $tname;

  if (defined $attr && $attr ne '') {
     my %attr_table;
     local @attr_list = split ( /"/, $attr);
     local ($name, $value);
     while (scalar @attr_list) {
        $name = shift @attr_list;
        $name =~ s/[ =]//g;
        next if ($name eq '');
        $value =  shift @attr_list;
        $attr_table{$name} =$value;
     }
     $node{ATTRIBUTES} = \%attr_table;
  }
  return \%node;
}

sub add_node {
  local ($parent_node, $node) = @_;
  push @{$parent_node->{CHILDREN}}, $node;

  local $tname = $node->{NAME};
  if (defined $parent_node->{$tname}) {
      push @{$parent_node->{$tname}}, $node
  } else {
      $parent_node->{$tname} = [ $node ];
  }
}

sub empty_tag {
   local ($parent_node, $tag) = @_;
   local $node = new_node($parent_node, $tag, 'EMPTY');
   add_node($parent_node, $node);
}

sub open_tag {
   local ($parent_node, $tag) = @_;
   local $node;

   if ($tag =~ /^\?.*/ || $tag =~ /^\!.*/) {
      $node = new_node($parent_node, $tag, 'COMMENT');
      add_node($parent_node, $node);
      undef; return;
   } else {
      $node = new_node($parent_node, $tag, 'NODE');
      $node->{CHILDREN} = [];
      add_node($parent_node, $node);
      return $node;
   }
}

sub add_text_node {
   local ($parent_node, $text) = @_;
   local $node = new_node($parent_node, $text, 'TEXT');
   add_node($parent_node, $node);
}

sub node_by_name {
   local ($node, $name) = @_;
   local ($tagname, $path) = split(/\//, $name, 2);

   my @nodelist;

   if ($tagname eq '') {
      while ($node->{PARENT} != 0) {
         $node = $node->{PARENT};
      }
      sublist_by_name($node, $path, \@nodelist);
   } else {
      sublist_by_name($node, $name, \@nodelist);
   }
   return @nodelist;
}

sub sublist_by_name {
   local ($node, $name, $res) = @_;
   local ($tagname, $path) = split(/\//, $name, 2);

   if (! defined $path) {
       push @{$res}, (@{$node->{$tagname}});
       return;
   }

   if ($tagname eq '..' && $node->{PARENT} != 0) {
      $node = $node->{PARENT};
      sublist_by_name($node, $path, $res);
   } else {
      local $n;
      for $n (@{$node->{$tagname}}) {
         sublist_by_name($n, $path, $res);
      }
   }
}

sub node_attribute {
    local $node = @_[0];
    if (defined $node->{ATTRIBUTES}) {
       return $node->{ATTRIBUTES}{@_[1]};
    }
    undef;
}

sub text_child {
    local ($node) = @_;
    local ($child) = node_by_name($node, 'TEXT');
    return $child->{TEXT};
}
