% { dg-do assemble { target mmix-*-* } }
 LOC (#80 << 56) + #200
 TETRA 1
 LOC (#80 << 56) + #100 % { dg-error "LOC expression stepping backwards" "" }
 TETRA 2
