(
  (SET C (ABS '-2'))
  (PRINT C)
  (SET B '-6')
  (PRINT B)
  (SWAP B C)
  (PRINT C)
  (PRINT B)
  (RESET B)
  (IF (ATOM '(1 3)') ((PRINT "(1 3) IS ATOM"))((PRINT "(1 3) IS NOT ATOM")))
  (IF (ATOM 'C') ((PRINT "C IS ATOM"))((PRINT "C IS NOT ATOM")))
  (IF (ATOM NIL) ((PRINT "NIL IS ATOM"))((PRINT "NIL IS NOT ATOM")))
)