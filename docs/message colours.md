# Message Colours

This is a general rule of thumb, but exceptions can be made. Coloured backgrounds should be avoided except in the case of error messages (or debug messages),
and for coloured foregrounds, the following colours are used for these purposes.

`{k}` (non-bold black) should never be used, as it won't display at all unless the user has their background set to another colour.

The brighter colours should be reserved for more important, attention-getting purposes, while the darker colours are for less-important things happening.

---

`{r}` (dark red)

`{g}` (dark green)

`{y}` (dark yellow)
- Weather messages.

`{b}` (dark blue)
- Low-importance NPC actions, such as non-hostile mobs moving around, or any mobs opening doors.

`{m}` (dark magenta)

`{c}` (dark cyan)
- System and tutorial messages.

`{w}` (gray)
- General messages, low-importance. Usually used as a response to commands, rather than something that happens separately.

---

`{R}` (bright red)

`{G}` (bright green)

`{Y}` (bright yellow)
- In-game error messages (such as "you can't travel in that direction", not code error messages).

`{B}` (bright blue)
- Medium-importance NPC actions, such as hostile mobs moving around.

`{M}` (bright magenta)

`{C}` (bright cyan)

`{W}` (white)
