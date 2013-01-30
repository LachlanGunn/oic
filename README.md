Open Instrument Control
=======================

The goal of this project is to develop an embedded-side software stack for
instrument control.  The initial target will be the Arduino platform, however
the goal is to eventually provide support for at least 8/16/32-bit PIC, AVR,
and ARM.

Initial development is for PC, and not much effort has been spent on
efficiency.  Data structures are all dynamically-allocated linked lists,
unsuitable for platforms with constrained resources.