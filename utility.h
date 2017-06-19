/**
 * Print data followed by a newline to the serial interface
 * only when debugging is enabled
 * 
 * This is a no-op when debugging is disabled.
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
 * Print data to the serial interface only when debugging is enabled
 * 
 * This is a no-op when debugging is disabled.
 * \see Serial::print
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
