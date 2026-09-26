# Examples

Things to upload to a unit to see a feature work, rather than reason about it.

| File | What it is |
| --- | --- |
| `commands.cfg` | a configuration that assigns commands to a button and to incoming MIDI - see COMMANDS.md |
| `pot-scaling.lua` | the script for the POT SCALING preset: preset functions, and two reports |

The preset itself is generated rather than stored:

```bash
python3 tools/compat/make_pot_preset.py > /tmp/pots.epr
```

To put all three on a unit, in this order - uploading a preset deletes the
slot's script, so the script has to follow it:

```bash
electraone --port "Electra CTRL" config upload examples/commands.cfg
electraone --port "Electra CTRL" preset set-slot -b 0 -s 4
electraone --port "Electra CTRL" preset upload /tmp/pots.epr
electraone --port "Electra CTRL" lua upload examples/pot-scaling.lua
electraone --port "Electra CTRL" preset switch -b 0 -s 4
```

`commands.cfg` gives the top right button the FUNCTIONS window instead of the
patch request, which the window offers anyway, and maps CC 110 to 117 on the
control port to a handful of commands. Everything else is the firmware's
default.
