/* This is generated by the overlay tool - DO NOT EDIT. */

  SHORT(ORIGIN(gui16))
  BYTE(ORIGIN(gui16)>>16)
  SHORT(end_gui16 - __gui16_lma)
  BYTE((end_gui16 - __gui16_lma)>>16)
  TRIM(gui16)

  SHORT(ORIGIN(gui8))
  BYTE(ORIGIN(gui8)>>16)
  SHORT(end_gui8 - __gui8_lma)
  BYTE((end_gui8 - __gui8_lma)>>16)
  TRIM(gui8)

  SHORT(ORIGIN(pal))
  BYTE(ORIGIN(pal)>>16)
  SHORT(end_pal - __pal_lma)
  BYTE((end_pal - __pal_lma)>>16)
  TRIM(pal)

  SHORT(ORIGIN(pianopal))
  BYTE(ORIGIN(pianopal)>>16)
  SHORT(end_pianopal - __pianopal_lma)
  BYTE((end_pianopal - __pianopal_lma)>>16)
  TRIM(pianopal)

  SHORT(ORIGIN(keys))
  BYTE(ORIGIN(keys)>>16)
  SHORT(end_keys - __keys_lma)
  BYTE((end_keys - __keys_lma)>>16)
  TRIM(keys)

