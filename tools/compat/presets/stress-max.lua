-- Formatter for the stress preset.
--
-- Every fader and every envelope attack runs this on each value change, so it
-- is what writes the value labels the measurement is about.

function formatValue (valueObject, value)
   return (string.format("%d%%", math.floor(value * 100 / 127)))
end
