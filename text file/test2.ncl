#this is test2 case
#
#
#
(
  (SET A (LENGTH NIL))
  (SET B '(-1)')
  (WHILE (LESS A '4') (
  (SET  A (PLUS A '1'))
  (PRINT A)
  (SET B (CONS A B))
  (PRINT (CDR B))
  ))
)