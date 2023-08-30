#this is a test case cover all the basic cases -- parse_pass && interp_pass  
(
  (SET A '(2 3)') 
  (SET C (ABS '-2') )
  (RESET A)  
  (SET A '4')
  (WHILE (GREATER A '2') (
  (SET B '(1 -2 -3)')
  (SET D (CAT A B))
  (SWAP D C)
  (SET B (MUL A B))
  (SET A (MOD A '1'))
  (SET C (DIV C '2'))
  (PRINT A)
  (IF (EQUAL A (LENGTH C)) ((PRINT "A EQUAL TO LENGTH C"))((PRINT "A NOT EQUAL TO LENGTH C")))
  ))
)