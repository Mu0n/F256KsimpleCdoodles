/* This is generated by the overlay tool - DO NOT EDIT. */

  SHORT(ORIGIN(gui))
  BYTE(ORIGIN(gui)>>16)
  SHORT(end_gui - __gui_lma)
  BYTE((end_gui - __gui_lma)>>16)
  TRIM(gui)

  SHORT(ORIGIN(pal))
  BYTE(ORIGIN(pal)>>16)
  SHORT(end_pal - __pal_lma)
  BYTE((end_pal - __pal_lma)>>16)
  TRIM(pal)

