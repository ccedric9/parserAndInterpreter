#this is a test case cover all the basic cases -- parse_pass && interp_pass  
(
  (SET A '4') (SET C '(2 3)')
  (WHILE (GREATER A '2') (
  (SET B '(1 -2 -3)')
  (PRINT (LENGTH C))
  (PRINT (CDR B))
  (SET B (CONS A B))
  (SET A (MINUS A '1'))
  (PRINT A)
  (IF (EQUAL A (LENGTH C)) ((PRINT "A EQUAL TO LENGTH C"))((PRINT "A NOT EQUAL TO LENGTH C")))
  ))
)