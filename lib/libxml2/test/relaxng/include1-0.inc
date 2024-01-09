<grammar xmlns="http://relaxng.org/ns/structure/1.0">
 <include href="include1-1.inc">
   <start>
     <notAllowed/>
   </start>
 </include>
 <start combine="choice">
   <element name="b">
     <empty/>
   </element>
 </start>
</grammar>
