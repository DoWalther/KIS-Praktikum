/**
 * zeilenweise Ausgabe wenn debugging aktiviert * 
 * \see Serial::println
 */
template<typename T>
void debugprintln(
  T const&
#ifdef DEBUG
  t
#endif
)
{
#ifdef DEBUG
  Serial.println(t);
#endif
}

/**
 * zeilenweises Ausgabe wenn debugging aktiviert * 
 * \see Serial::println
 */
template<typename T>
void debugprint(
  T const&
#ifdef DEBUG
  t
#endif
)
{
#ifdef DEBUG
  Serial.print(t);
#endif
}
